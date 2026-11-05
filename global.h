/* =========== ENUMS & CONSTANTS ===========*/

//const char* hostname = "esp32-node-temperature";
#define BMS_MAX_CELLS 8
#define N10K 10000
#define N100K 100000
#define N1M 1000000
#define N10M 10000000
#define N100M 100000000
#define N1B 1000000000
#define N10B 10000000000
//const uint32_t ten_pow[] PROGMEM = {1, 10, 100, 1000, N10K, N100K, N1M, N10M, N100M, N1B};

static int quick_pow10(int n) {
  static int pow10[10] = { 1, 10, 100, 1000, N10K, N100K, N1M, N10M, N100M, N1B };
  return pow10[n];
}

enum BoxMode {
  UNKNOWN,
  TO_GRID,
  GRID,
  INV_PREHEAT,
  TO_INV,
  INV,
  INV_PLUS
};

const char* boxModeStrings4c[] = { "????", "2GRI", "GRID", "INVH", "2INV", "INV ", "INV+" };

/*=========== Flash config ===========*/
uint8_t configStructVersion = 1;  // v1 - version of config struct, not values
struct ConfigStruct {
  uint8_t toInvSolarPanelPower = 10;          //0-255 x 10 watts
  uint8_t toInvSoc = 90;                      //% >toGridSoc
  uint8_t toGridSoc = 20;                     //% >wakeUpSoc && >toGridSocCritical && <toInvSoc
  uint8_t toGridSocCritical = 10;             //% <toGridSoc && >goToSleepSoc
  uint8_t outputThreshold = 10;               //0-255 x 10 watts
  uint8_t goToSleepSoc = 5;                   //% <wakeUp && <toGridSocCritical
  uint8_t wakeUpSoc = 15;                     //% >goToSleepSoc && <toGridSoc
  uint8_t needToSendHelloAfterReconnect = 1;  //send Hello to Telegram Bot after WIFI reconnect
  uint8_t showTemperature = 1;                //"No", "t1", "t2", "MOS", "Avg", "Min", "Max"
  uint8_t invIdle = 5;                        //w 0-255 Inverter Idle Consumption, for panel power calculation
  uint8_t invEfficiency = 90;                 //% 1 - 100 Inverter Energy conversion efficiency, for panel power calculation
  uint8_t ignoreConditionsDuration = 2;       //(2+1)*10=30s 1-256x10 sec, force change mode duration, all rules is ignored during this period
} config;

/* =========== VARIABLES ===========*/
struct BoxFlagsStruct {
  //false
  bool isWifiConnected : 1;
  bool isWifiDisabled : 1;
  bool wifiStatusUpdated : 1;
  bool solarPanelInfoUpdated : 1;

  bool boxModeUpdated : 1;
  bool battInfoUpdated : 1;
  bool outputInfoUpdated : 1;
  bool homeScreenNeedToRedraw : 1;

  bool menuScreenNeedToRedraw : 1;
  bool battInfoScreenNeedToRedraw : 1;
  bool outputInfoScreenNeedToRedraw : 1;
  bool statScreenNeedToRedraw : 1;

  bool settingsScreenNeedToRedraw : 1;
  bool logScreenNeedToRedraw : 1;
  bool isNeedToSaveConfig : 1;
  bool networkScreenNeedToRedraw : 1;

  bool isNeedToResetJsy : 1;
  bool isNeedToResetStatInNVS : 1;
  //true
  bool jsyReadFailed : 1;
  bool bmsReadFailed : 1;
} boxFlags = { false, false, false, false,
               false, false, false, false,
               false, false, false, false,
               false, false, false, false,
               false, false, true, true };

uint16_t solarPanelPower = 0;

inline void setSolarPower(int16_t value) {
  if (solarPanelPower != value) {
    solarPanelPower = value;
    boxFlags.solarPanelInfoUpdated = true;
  }
}

BoxMode boxMode = UNKNOWN;

struct BMSDataStruct {
  uint16_t cellVoltages[BMS_MAX_CELLS] = { 0 };  // * 0.001 V
  uint8_t minVoltageCellIndex = 0;               // 0 - BMS_MAX_CELLS
  uint8_t maxVoltageCellIndex = 0;               // 0 - BMS_MAX_CELLS
  uint8_t numCells = 0;                          // 0 - BMS_MAX_CELLS
  uint16_t totalVoltage = 0;                     // * 0.01 V
  int16_t current = 0;                           //*0.01 A
  uint16_t rawCurrent = 0;                       //Unconverted value
  uint8_t soc = 0;
  int8_t mosTemp = 0;
  int8_t temp1 = 0;
  int8_t temp2 = 0;
  uint16_t alarmStatus = 0;
  uint8_t statusInfo = 0;  //1-bit CHARGE MOS, 2-bit DISCHARGE MOS, 3=bit BALANCE
  uint16_t cycles = 0;
  uint8_t version = 0;
  uint8_t protocolVersion = 0;
  uint8_t readCount = 0;
  uint8_t failCountOld = 0;
  uint8_t failCountNew = 0;
} bmsData;

uint8_t _end = 0;
uint16_t _crcWord = 0;
uint16_t _calcCrc = 0;

struct JSYDataStruct {
  uint16_t voltage = 0;    // *0.01 V
  uint16_t current = 0;    // *0.01 A
  uint16_t power = 0;      // W
  uint32_t energyRaw = 0;  // /3200 kWh
  // >>5 == /32
  uint32_t energyDeka = 0;  // *10 Wh
  uint16_t pf = 0;          // *0.001
  uint32_t co2 = 0;         // *0.01 kg
  uint16_t temp = 0;        // reserved
  uint16_t freq = 0;        // *0.01 Hz
  uint16_t maxPower = 0;
  uint16_t avgPower = 0;
  uint8_t version = 0;
} jsyData;

struct ScreenDataStruct {
  uint8_t screen = 0;
  uint8_t goToScreen = 0;
} screenData;

struct StatInfoStruct {
  uint8_t sPanelLastSecCounter = 0;        //0 - 59s
  uint8_t sPanelLastMinCounter = 0;        //0 - 59m
  uint32_t sPanelCounterValue = 0;         //Ws Ws/60=Wh  0 - 4,294,967,295
  uint16_t sPanelMetering12h[12] = { 0 };  //Wh 0 - 65,535  65kw
  uint8_t sPanelMetering12hIdx = 0;        // 0 - 11, e.g. 8 - last hour, 7 previous hour, etc
  uint32_t sPanelMeteringTotal = 0;        //Wh*10 0 - 4,294,967,295
  uint32_t gridModeTime = 0;               //s time on INV_PREHEAT, TO_GRID,GRID modes
  uint32_t invModeTime = 0;                //s time on TO_INV, INV,INV+ modes
  uint32_t prevOutputMetering = 0;         // /3200 KWh
  uint32_t gridOutputMetering = 0;         // *0.01 KWH INV_PREHEAT, TO_INV, GRID modes, P.S. INV_PREHEAT and TO_INV cause delta mostly accumulated on previuos mode
  uint32_t invOutputMetering = 0;          // *0.01 KWH TO_GRID INV INV+ modes, P.S. TO_GRID cause delta mostly accumulated on previuos mode
  uint32_t warnTime = 0;                   //s time on INV_PREHEAT, TO_GRID, GRID modes and outputvoltage < 190V
  uint32_t failTime = 0;                   //s time on TO_INV, INV, INV+ modes  and outputvoltage < 190V
} statInfo;


uint8_t forceChangeMode = 0;  //0-NO_FORCE, 1-TO_GRID, 2-TO_INV, 3-TO_INV_PLUS
uint8_t lastToInvSoc = 0;     //store soc of changing to inv mode

/* =========== LOG VARIABLES ===========*/
#define LOG_SIZE 8
uint32_t modeChangeLogMillis[LOG_SIZE] = { 0 };
BoxMode modeChangeLogMode[LOG_SIZE] = { UNKNOWN };
uint8_t modeChangeLogBmsStatus[LOG_SIZE] = { 0 };
uint8_t modeChangeLogBmsSoc[LOG_SIZE] = { 0 };
uint8_t lastChangeIndex = LOG_SIZE - 1;

/* =========== MILLIS VARIABLES ===========*/
uint32_t now;  //call millis() one per loop, Every call millis(), the Arduino has to disable interrupts, copy a 4-byte volatile variable from memory, and re-enable interrupts.
uint32_t lastStatMillis;
uint32_t lastModeChangeMillis;
uint32_t lastConfigChangesMillis;
/* =========== UTIL METHODS ===========*/

uint32_t getSolarPanelMetering12h() {
  uint32_t result = 0;
  for (int i = 0; i < 12; i++) {
    result += statInfo.sPanelMetering12h[i];
  }
  return result;
}

int8_t getBmsTemperature() {
  switch (config.showTemperature) {
    case 1:
      return bmsData.temp1;
    case 2:
      return bmsData.temp2;
    case 3:
      return bmsData.mosTemp;
    case 4:
      return (int16_t(bmsData.temp1) + int16_t(bmsData.temp2) + int16_t(bmsData.mosTemp)) / 3;
    case 5:
      return (bmsData.temp1 < bmsData.temp2 && bmsData.temp1 < bmsData.mosTemp) ? bmsData.temp1 :
                                                                                //bmsData.t1 >= bmsData.t2 || bmsData.t1>=bmsData.mosTemp
               ((bmsData.temp2 < bmsData.mosTemp) ? bmsData.temp2 : bmsData.mosTemp);
    case 6:
      return (bmsData.temp1 > bmsData.temp2 && bmsData.temp1 > bmsData.mosTemp) ? bmsData.temp1 : ((bmsData.temp2 > bmsData.mosTemp) ? bmsData.temp2 : bmsData.mosTemp);
    default:
      return 0;
  }
}

inline void logModeChange() {
  if (lastChangeIndex < LOG_SIZE - 1) lastChangeIndex++;
  else lastChangeIndex = 0;
  modeChangeLogMillis[lastChangeIndex] = now;
  modeChangeLogMode[lastChangeIndex] = boxMode;
  modeChangeLogBmsStatus[lastChangeIndex] = bmsData.statusInfo;
  modeChangeLogBmsSoc[lastChangeIndex] = bmsData.soc;
}

void _appendCellInfo(String& result, uint8_t i) {
  result += (i + 1);
  result += ": ";
  result += bmsData.cellVoltages[i] / 1000;
  result += ".";
  if (bmsData.cellVoltages[i] % 1000 < 10) result += "00";
  else if (bmsData.cellVoltages[i] % 1000 < 100) result += "0";
  result += bmsData.cellVoltages[i] % 1000;
}

String getCurrentBoxStatus() {
  String result;
  result.reserve(256);
  result += "Mode: ";
  result += boxModeStrings4c[(uint8_t)boxMode];
  result += "\n\rStatus: ";                                  //8
  result += boxFlags.bmsReadFailed ? "Fail\n\r" : "OK\n\r";  //4
  //if (boxFlags.bmsReadFailed) return result;

  result += bmsData.soc;
  result += "% ";  //5
  result += constrain(bmsData.totalVoltage / 100, 0, 99);
  result += ".";
  if ((bmsData.totalVoltage % 100) < 10) result += "0";
  result += bmsData.totalVoltage % 100;
  result += "v ";  //7
  result += "\n\r";

  uint8_t rows = (bmsData.numCells + 1) / 2;
  //Serial.print("rows");
  //Serial.println(rows);
  for (uint8_t i = 0; i < rows; i++) {
    _appendCellInfo(result, i);
    result += " ";
    if ((i + rows) < bmsData.numCells) _appendCellInfo(result, i + rows);
    result += "\n\r";
  }

  result += "Max-Min:";
  uint16_t diff = bmsData.cellVoltages[bmsData.maxVoltageCellIndex] - bmsData.cellVoltages[bmsData.minVoltageCellIndex];
  result += diff / 1000;
  result += ".";
  if (diff % 1000 < 10) result += "00";
  else if (diff % 1000 < 100) result += "0";
  result += diff % 1000;
  result += "\n\rRaw current:";
  result += bmsData.rawCurrent;
  result += "\n\r";

  if (bmsData.current > 0) {
    result += "Charge: ";
    result += constrain(bmsData.current / 100, 0, 999);
    result += ".";
    if ((bmsData.current % 100) < 10) result += "0";
    result += (bmsData.current % 100);
    result += "A\n\r";
  } else if (bmsData.current < 0) {
    result += "Discharge: ";
    result += constrain(bmsData.current / -100, 0, 999);
    result += ".";
    if (((-bmsData.current % 100)) < 10) result += "0";
    result += ((-bmsData.current) % 100);
    result += "A\n\r";
  } else {
    result += "Zero current\n\r";
  }
  if (bitRead(bmsData.alarmStatus, 0)) result += "WARNING: Low capacity alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 1)) result += "WARNING: MOS overtemperature alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 2)) result += "WARNING: Charge overvoltage alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 3)) result += "WARNING: Discharge overvoltage alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 4)) result += "WARNING: Temp1 overtemperature alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 5)) result += "WARNING: Charge overcurrent alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 6)) result += "WARNING: Discharge overcurrent alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 7)) result += "WARNING: Differential pressure alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 8)) result += "WARNING: Temp2 overtemperature alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 9)) result += "WARNING: Battery low temperature alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 10)) result += "WARNING: Unit overvoltage alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 11)) result += "WARNING: Unitundervoltage alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 12)) result += "WARNING: 309_A protection\n\r";
  if (bitRead(bmsData.alarmStatus, 13)) result += "WARNING: 309_B protection\n\r";
  if (bitRead(bmsData.alarmStatus, 14)) result += "WARNING: Unknown1(reserved) alarm\n\r";
  if (bitRead(bmsData.alarmStatus, 15)) result += "WARNING: Unknown2(reserved) alarm\n\r";
  result += "lastToInvSoc";
  result += lastToInvSoc;
  result += "\n\rend:";
  result += _end;
  result += "\n\rcrc: ";
  result += _crcWord;
  result += " == ";
  result += _calcCrc;
  result += "\n\rfailRate:";
  result += bmsData.failCountNew;
  result += "(";
  result += bmsData.failCountOld;
  result += ")/";
  result += bmsData.readCount;
  result += "\n\rl=";
  result += result.length();
  //Serial.print("=========== result.length() ==");
  //Serial.println(result.length());
  return result;
}

String getLogInfo() {
  String result;
  result.reserve(256);
  for (uint8_t i = lastChangeIndex + LOG_SIZE; i > lastChangeIndex; i--) {
    result += (now - modeChangeLogMillis[i % LOG_SIZE]) / 1000;
    result += "s ago to ";
    result += boxModeStrings4c[modeChangeLogMode[i % LOG_SIZE]];
    result += " soc=";
    result += modeChangeLogBmsSoc[i % LOG_SIZE];
    result += " bmsStatus=";
    result += modeChangeLogBmsStatus[i % LOG_SIZE];
    result += "\n\r";
  }
  return result;
}
