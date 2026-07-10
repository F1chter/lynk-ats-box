/* =========== IMPORTS ===========*/
#include "global.h"
//#include <LittleFS.h>
//#include "LynkFile.h"
#include "LynkNVS.h"
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
#define SSR_GRID_PIN 13              //YELLOW
#define SSR_INV_PIN 14               //BLUE
#define SSR_REL_PIN 27               //GREEN
#define SAVE_AFTER_LAST_DELAY 30000  //30s

/* =========== VARIABLES ===========*/
uint32_t lastPanelCounterMillis;


uint8_t taskExecutionIndex = 0;  //avoid execution heavy tasks inside one loop

uint32_t lastModeChangeMillis;
uint32_t lastEmulateMeterMillis;

//LynkFile configFile(&LittleFS, "/config.cfg", 1, &config, sizeof(config));
/* =========== SETUP ===========*/

void setup() {
  pinMode(INV_PIN, OUTPUT);
  digitalWrite(INV_PIN, LOW);
  pinMode(SSR_GRID_PIN, OUTPUT);
  digitalWrite(SSR_GRID_PIN, LOW);
  pinMode(SSR_INV_PIN, OUTPUT);
  digitalWrite(SSR_INV_PIN, LOW);
  pinMode(SSR_REL_PIN, OUTPUT);
  digitalWrite(SSR_REL_PIN, LOW);
  ledBegin();
  simpleBlink(1000);
  buzzerBegin();
  encBegin();

  //Serial.begin(115200);

  setupScreen();
  //simpleMelody();
  //delay(3000);
  //Serial.println("=========== SETUP STARTED===========");

  //Serial.println("=========== DEFAULT VALUES==========");
  //printConfig();  // print default values
  //LittleFS.begin(true);
  //FileStatus fileStatus = configFile.init();
  //Serial.println("=========== VALUES FROM FLASH==========");
  //printConfig();  // print values from flash
  nvsBegin();

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
  if (boxMode == INV_PREHEAT || boxMode == TO_INV || boxMode == INV || boxMode == INV_PLUS) battPower += config.invIdle;
  if (boxMode == TO_INV || boxMode == INV || boxMode == INV_PLUS) {
    int32_t outputPower = (int32_t)jsyData.power;
    outputPower *= 100;
    outputPower /= config.invEfficiency;
    if (outputPower + battPower > 0) {
      solarPanelPower = outputPower + battPower;
      boxFlags.solarPanelInfoUpdated = true;
    } else if (solarPanelPower != 0) {
      solarPanelPower = 0;
      boxFlags.solarPanelInfoUpdated = true;
    }

  } else if (battPower > 0) {
    solarPanelPower = battPower;
    boxFlags.solarPanelInfoUpdated = true;
  } else if (solarPanelPower != 0) {
    solarPanelPower = 0;
    boxFlags.solarPanelInfoUpdated = true;
  }
}

void tickPanelMeter() {
  if (millis() - lastPanelCounterMillis < 1000L) return;
  statInfo.sPanelCounterValue += solarPanelPower;
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
  if (boxMode == TO_GRID && millis() - lastModeChangeMillis > 5000L) {
    boxMode = GRID;
    boxFlags.boxModeUpdated = true;
    digitalWrite(SSR_GRID_PIN, LOW);
    digitalWrite(INV_PIN, LOW);
  } else if (forceChangeMode != 1 && boxMode == GRID && millis() - lastModeChangeMillis > 10000L) {
    if (forceChangeMode == 2 || forceChangeMode == 3 || bmsData.soc >= config.toInvSoc || (bmsData.soc > config.toGridSoc && solarPanelPower >= (((uint16_t)config.toInvSolarPanelPower) * 10))) {
      debugSendToAdmin(INV_PREHEAT);
      boxMode = INV_PREHEAT;
      boxFlags.boxModeUpdated = true;
      digitalWrite(INV_PIN, HIGH);
    }
  } else if (boxMode == INV_PREHEAT && millis() - lastModeChangeMillis > 5000L) {
    boxMode = TO_INV;
    boxFlags.boxModeUpdated = true;
    digitalWrite(SSR_INV_PIN, HIGH);
  } else if (boxMode == TO_INV && millis() - lastModeChangeMillis > 5000L) {
    boxMode = INV;
    boxFlags.boxModeUpdated = true;
    digitalWrite(SSR_INV_PIN, LOW);
  } else if (forceChangeMode != 2 && boxMode == INV && millis() - lastModeChangeMillis > 10000L) {
    if (forceChangeMode == 3) {
      boxMode = INV_PLUS;
      boxFlags.boxModeUpdated = true;
      digitalWrite(SSR_REL_PIN, HIGH);
    } else if (forceChangeMode == 1 || bmsData.soc <= config.toGridSocCritical || (bmsData.soc <= config.toGridSoc && jsyData.power < (config.outputThreshold * 10))) {
      debugSendToAdmin(TO_GRID);
      boxMode = TO_GRID;
      boxFlags.boxModeUpdated = true;
      digitalWrite(SSR_GRID_PIN, HIGH);
    }
  } else if (forceChangeMode != 3 && boxMode == INV_PLUS) {
    if (forceChangeMode == 1 || forceChangeMode == 2 || bmsData.soc <= config.toGridSoc) {
      debugSendToAdmin(INV);
      boxMode = INV;
      boxFlags.boxModeUpdated = true;
      digitalWrite(SSR_REL_PIN, LOW);
    }
  } else if (boxMode == UNKNOWN) {
    if (forceChangeMode == 2 || forceChangeMode == 3 || bmsData.soc >= config.toInvSoc || (bmsData.soc > config.toGridSoc && solarPanelPower >= (((uint16_t)config.toInvSolarPanelPower) * 10))) {
      boxMode = INV_PREHEAT;
      boxFlags.boxModeUpdated = true;
      digitalWrite(INV_PIN, HIGH);
    }
  }

  if (boxFlags.boxModeUpdated) {
    lastModeChangeMillis = millis();
    if (lastChangeIndex < LOG_SIZE - 1) lastChangeIndex++;
    else lastChangeIndex = 0;
    modeChangeLogMillis[lastChangeIndex] = lastModeChangeMillis;
    modeChangeLogMode[lastChangeIndex] = boxMode;
    boxFlags.logScreenNeedToRedraw = true;
  } else if (forceChangeMode != 0 && (millis() - lastModeChangeMillis) > (((uint32_t)config.ignoreConditionsDuration) + 1) * 10000) {
    forceChangeMode = 0;
    boxFlags.boxModeUpdated = true;
  }
}

void debugSendToAdmin(uint8_t newMode) {
  String s = "DEBUG from ";
  s += (uint8_t)boxMode;
  s += " to ";
  s += newMode;
  s += " soc = ";
  s += bmsData.soc;
  s += " crit = ";
  s += config.toGridSocCritical;
  s += " toGridSoc = ";
  s += config.toGridSoc;
  s += " toInvSoc = ";
  s += config.toInvSoc;
  sendMessage(s, ADMIN_ID);
}

void tickSaveConfig() {
  if (!boxFlags.isNeedToSaveConfig) return;
  if (millis() - lastConfigChangesMillis < SAVE_AFTER_LAST_DELAY) return;
  saveConfigNVS();
  //configFile.commit();
  setFlagToRedrawCurrentScreen();
  boxFlags.isNeedToSaveConfig = false;
}

/*
void emulateChargeDischarge() {
  if (boxMode == UNKNOWN || boxMode == TO_GRID || boxMode == GRID || boxMode == INV_PREHEAT) {
    if (bmsData.soc < 100) {
      bmsData.soc++;
      boxFlags.battInfoUpdated = true;
    }
  } else if (bmsData.soc > 0) {
    bmsData.soc -= boxMode == INV_PLUS ? 2 : 1;
    boxFlags.battInfoUpdated = true;
  }
}
*/