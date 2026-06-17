/* =========== IMPORTS ===========*/
#include "global.h"
#include <LittleFS.h>
#include "LynkFile.h"
#include "LynkJkBms.h"
#include "LynkLED.h"
#include "LynkBuzzer.h"
#include "secrets.h"
#include "LynkTelegramBot.h"
#include "LynkEncoder.h"
#include "ScreenManager.h"
#include "LynkJsy1050.h"

/* =========== ENUMS & CONSTANTS ===========*/
#define INV_PIN 15
#define SAVE_AFTER_LAST_DELAY 30000  //30s

/* =========== VARIABLES ===========*/
uint32_t lastPanelCounterMillis;


uint8_t taskExecutionIndex = 0;  //avoid execution heavy tasks inside one loop

uint32_t lastModeChangeMillis;
uint32_t lastEmulateMeterMillis;

LynkFile configFile(&LittleFS, "/config.cfg", 1, &config, sizeof(config));
/* =========== SETUP ===========*/

void setup() {
  pinMode(INV_PIN, OUTPUT);
  digitalWrite(INV_PIN, LOW);
  ledBegin();
  simpleBlink(1000);
  buzzerBegin();
  encBegin();

  Serial.begin(115200);

  setupScreen();
  //simpleMelody();
  delay(3000);
  Serial.println("=========== SETUP STARTED===========");

  Serial.println("=========== DEFAULT VALUES==========");
  printConfig();  // print default values
  LittleFS.begin(true);
  FileStatus fileStatus = configFile.init();
  Serial.println("=========== VALUES FROM FLASH==========");
  printConfig();  // print values from flash


  bmsBegin();
  jsyBegin();

  drawHomeScreen();
  telegramBegin();
  attachStatusFunction(bmsSimpleStatus);
  lastPanelCounterMillis = millis();
  lastModeChangeMillis = millis();
  lastEmulateMeterMillis = millis();
  lastConfigChangesMillis = millis();
}
/* =========== LOOP ===========*/
void loop() {

  boxFlags.wifiStatusUpdated = false;
  boxFlags.solarPanelInfoUpdated = false;
  boxFlags.boxModeUpdated = false;
  boxFlags.battInfoUpdated = false;
  boxFlags.outputInfoUpdated = false;
  tickEnc();
  if (taskExecutionIndex > 3) taskExecutionIndex = 0;
  if (taskExecutionIndex == 3) {
    tickBms();
    taskExecutionIndex++;
  }
  if (taskExecutionIndex == 2) {
    //emulateMeter();
    tickJsy();
    taskExecutionIndex++;
  }
  calculatePanelPower();
  tickPanelMeter();
  if (taskExecutionIndex == 1) {
    tickTelegram();
    taskExecutionIndex++;
  }

  if (taskExecutionIndex == 0) {
    tickSaveConfig();
    taskExecutionIndex++;
  } 
  tickEnc();
  handleEncoderCommand();
  tickBoxMode();
  tickScreen();



  delay(1);
}


void calculatePanelPower() {
  if (!boxFlags.battInfoUpdated && !boxFlags.outputInfoUpdated) return;
  int32_t battPower = ((int32_t)bmsData.current) * ((int32_t)bmsData.totalVoltage);
  battPower /= 10000;
  if (mainInfo.boxMode == INV_PREHEAT || mainInfo.boxMode == TO_INV || mainInfo.boxMode == INV || mainInfo.boxMode == INV_PLUS) battPower += config.invIdle;
  if (mainInfo.boxMode == TO_INV || mainInfo.boxMode == INV || mainInfo.boxMode == INV_PLUS) {
    int32_t outputPower = (int32_t)jsyData.power;
    outputPower *= 100;
    outputPower /= config.invEfficiency;
    if (outputPower + battPower > 0) {
      mainInfo.solarPanelPower = outputPower + battPower;
      boxFlags.solarPanelInfoUpdated = true;
    } else if (mainInfo.solarPanelPower != 0) {
      mainInfo.solarPanelPower = 0;
      boxFlags.solarPanelInfoUpdated = true;
    }

  } else if (battPower > 0) {
    mainInfo.solarPanelPower = battPower;
    boxFlags.solarPanelInfoUpdated = true;
  } else if (mainInfo.solarPanelPower != 0) {
    mainInfo.solarPanelPower = 0;
    boxFlags.solarPanelInfoUpdated = true;
  }
}

void tickPanelMeter() {
  if (millis() - lastPanelCounterMillis < 1000L) return;
  statInfo.sPanelCounterValue += mainInfo.solarPanelPower;
  if (statInfo.sPanelLastSecCounter < 60) {
    statInfo.sPanelLastSecCounter++;
  } else {
    statInfo.sPanelMetering12h[statInfo.sPanelMetering12hIdx] += (statInfo.sPanelCounterValue / 3600);  //Ws to Wh
    statInfo.sPanelCounterValue = 0;
    statInfo.sPanelLastSecCounter = 0;
    statInfo.sPanelLastMinCounter++;
    boxFlags.solarPanelInfoUpdated = true;
  }
  if (statInfo.sPanelLastMinCounter >= 60) {
    statInfo.sPanelMeteringTotal += statInfo.sPanelMetering12h[statInfo.sPanelMetering12hIdx] / 10;
    if (statInfo.sPanelMetering12hIdx < 11) statInfo.sPanelMetering12hIdx++;
    else statInfo.sPanelMetering12hIdx = 0;
    statInfo.sPanelMetering12h[statInfo.sPanelMetering12hIdx] = 0;
  }
  lastPanelCounterMillis = millis();
}


/*
bool emulateMeterGrow = true;
void emulateMeter() {
  if (millis() - lastEmulateMeterMillis < 10000L) return;
  Serial.println("=========== emulateMeter STARTED===========");
  if (emulateMeterGrow) {
    mainInfo.outputPower += 10;
    if (mainInfo.outputPower > 500) emulateMeterGrow = false;
  } else {
    mainInfo.outputPower -= 10;
    if (mainInfo.outputPower < 10) emulateMeterGrow = true;
  }
  //for testing
  mainInfo.outputMeteringTotal *= 9;
  if (mainInfo.outputMeteringTotal > N1B) mainInfo.outputMeteringTotal = 1;
  boxFlags.outputInfoUpdated = true;

  //for testing
  mainInfo.panelMeteringTotal *= 9;
  if (mainInfo.panelMeteringTotal > N1B) mainInfo.panelMeteringTotal = 1;
  boxFlags.solarPanelInfoUpdated = true;

  lastEmulateMeterMillis = millis();
}
*/


//0-NO_FORCE, 1-TO_GRID, 2-TO_INV, 3-TO_INV_PLUS
void tickBoxMode() {
  if (mainInfo.boxMode == TO_GRID && millis() - lastModeChangeMillis > 5000L) {
    mainInfo.boxMode = GRID;
    boxFlags.boxModeUpdated = true;
    //disable to inv ssr
    digitalWrite(INV_PIN, LOW);
  } else if (forceChangeMode != 1 && mainInfo.boxMode == GRID && millis() - lastModeChangeMillis > 10000L) {
    if (forceChangeMode == 2 || forceChangeMode == 3 || bmsData.soc >= config.toInvSoc || (bmsData.soc > config.toGridSoc && mainInfo.solarPanelPower >= (((uint16_t)config.toInvSolarPanelPower) * 10))) {
      mainInfo.boxMode = INV_PREHEAT;
      boxFlags.boxModeUpdated = true;
      digitalWrite(INV_PIN, HIGH);
    }
  } else if (mainInfo.boxMode == INV_PREHEAT && millis() - lastModeChangeMillis > 5000L) {
    mainInfo.boxMode = TO_INV;
    boxFlags.boxModeUpdated = true;
    //enable to inv ssr
  } else if (mainInfo.boxMode == TO_INV && millis() - lastModeChangeMillis > 5000L) {
    mainInfo.boxMode = INV;
    boxFlags.boxModeUpdated = true;
    //disable to inv ssr
  } else if (forceChangeMode !=2 && mainInfo.boxMode == INV && millis() - lastModeChangeMillis > 10000L) {
    if (forceChangeMode == 3) {
      mainInfo.boxMode = INV_PLUS;
      boxFlags.boxModeUpdated = true;
      //enable homeRelaySSR
    } else if ((forceChangeMode == 1) || (bmsData.soc <= config.toGridSocCritical) || (bmsData.soc <= config.toGridSoc && jsyData.power < (config.outputThreshold * 10))) {
      mainInfo.boxMode = TO_GRID;
      boxFlags.boxModeUpdated = true;
      //enable to grid ssr
    }
  } else if (forceChangeMode != 3 && mainInfo.boxMode == INV_PLUS && millis() - lastModeChangeMillis > 5000L) {
    if (forceChangeMode == 1 || forceChangeMode == 2 || bmsData.soc <= config.toGridSoc) {
      mainInfo.boxMode = INV;
      boxFlags.boxModeUpdated = true;
      //disable homeRelaySSR
    }
  } else if (mainInfo.boxMode == UNKNOWN && millis() - lastModeChangeMillis > 5000L) {
    if ((forceChangeMode == 2 || forceChangeMode == 3 && bmsData.soc > config.toGridSoc) || bmsData.soc >= config.toInvSoc || (bmsData.soc > config.toGridSoc && mainInfo.solarPanelPower >= (((uint16_t)config.toInvSolarPanelPower) * 10))) {
      mainInfo.boxMode = INV_PREHEAT;
      boxFlags.boxModeUpdated = true;
      digitalWrite(INV_PIN, HIGH);
    }
  }

  if (boxFlags.boxModeUpdated) {
    lastModeChangeMillis = millis();
    if (lastChangeIndex < LOG_SIZE - 1) lastChangeIndex++;
    else lastChangeIndex = 0;
    modeChangeLogMillis[lastChangeIndex] = lastModeChangeMillis;
    modeChangeLogMode[lastChangeIndex] = mainInfo.boxMode;
    boxFlags.logScreenNeedToRedraw = true;
  } else if (forceChangeMode != 0 && (millis() - lastModeChangeMillis) > (((uint32_t)config.ignoreMinorConditionsDuration) + 1) * 10000) {
    forceChangeMode = 0;
    boxFlags.boxModeUpdated = true;
  }
}

void printConfig() {
  /*
  struct ConfigStruct {
  uint8_t toInvPanelPower = 10;               //0-255 x 10 watts
  uint8_t toInvSoc = 90;                      //% >toGridSoc
  uint8_t toGridSoc = 20;                     //% >wakeUpSoc && >toGridSocCritical && <toInvSoc
  uint8_t toGridSocCritical = 10;             //% <toGridSoc && >goToSleepSoc
  uint8_t outputThreshold = 10;               //0-255 x 10 watts
  uint8_t goToSleepSoc = 5;                   //% <wakeUp && <toGridSocCritical
  uint8_t wakeUpSoc = 15;                     //% >goToSleepSoc && <toGridSoc
  uint8_t needToSendHelloAfterReconnect = 1;  //send Hello to Telegram Bot after WIFI reconnect
  uint8_t showTemperature = 1;                //0 - no, 1 - avg, 2 - min, 3- max
  uint8_t invIdle = 5;                        //w 0-255 Inverter Idle Consumption, for panel power calculation
  uint8_t invEfficiency = 90;                 //% 1 - 100 Inverter Energy conversion efficiency, for panel power calculation
  uint8_t ignoreMinorConditionsDuration = 2;  //(2+1)*10=30s 1-256x10 sec, allow force to_grid if soc>to_inv and force to inv to_grid>soc>critical
} config;*/
}

void tickSaveConfig() {
  if (!boxFlags.isNeedToSaveConfig) return;
  if (millis() - lastConfigChangesMillis < SAVE_AFTER_LAST_DELAY) return;
  configFile.commit();
  setFlagToRedrawCurrentScreen();
  boxFlags.isNeedToSaveConfig = false;
}

/*
void emulateChargeDischarge() {
  if (mainInfo.boxMode == UNKNOWN || mainInfo.boxMode == TO_GRID || mainInfo.boxMode == GRID || mainInfo.boxMode == INV_PREHEAT) {
    if (bmsData.soc < 100) {
      bmsData.soc++;
      boxFlags.battInfoUpdated = true;
    }
  } else if (bmsData.soc > 0) {
    bmsData.soc -= mainInfo.boxMode == INV_PLUS ? 2 : 1;
    boxFlags.battInfoUpdated = true;
  }
}
*/