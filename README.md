# lynk-ats-box
Controller based on ESP32C6 for Automatic Transfer Switch


## Setup:
1. Install libs:
*  [FastBot2](https://github.com/GyverLibs/FastBot2/)

2. Create secrets.h and define next variables:
```
const String WIFI_SSID = "MY_WIFI";
const String WIFI_PASS = "MY_PASSWORD";
const String BOT_TOKEN = "1234567890:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
const String CHAT_ID = "123456789";
```



/*


void drawSettingsScreen() {

  oled.clear();
  buildStatusLine(F(" Settings:"));
  oled.setCursorXY(76, 0);
  oled.print(selectedSettingsItem < SETTINGS_ITEMS_COUNT ? selectedSettingsItem + 1 : selectedSettingsItem);
  oled.print("/");
  oled.print(SETTINGS_ITEMS_COUNT);
  if (settingsItemShift != 0) oled.drawArrowHead(64, 11, 0);
  oled.setCursorXY(0, settingsItemShift == 0 ? 9 : 17);
  _settingsItem(0, F("ToInvSPanel"), F("→"));
  _settingsItem(1, F("To Inv   Soc"), F("→"));
  _settingsItem(2, F("To Grid  Soc"), F("→"));
  _settingsItem(3, F("Critical Soc"), F("→"));
  _settingsItem(4, F("KeepInvIf >"), F("→"));
  _settingsItem(5, F("To Sleep Soc"), F("→"));
  _settingsItem(6, F("Wake  Up Soc"), F("→"));
  _settingsItem(7, F("Tg send hello"), F("→"));
  _settingsItem(8, F("Show bat temp"), F("→"));
  _settingsItem(9, F("InverterIdle"), F("→"));
  _settingsItem(10, F("InvEfficienc"), F("→"));
  _settingsItem(11, F("Force Time"), F("→"));

  if (settingsItemShift + ITEMS_PER_PAGE + 1 == SETTINGS_ITEMS_COUNT) oled.drawBitmap2(0, 57, box_back, 12, 7, selectedSettingsItem == BACK_ITEM_INDEX);
  else oled.drawArrowHead(64, 62, 2);
  oled.update();
  settingsScreenNeedToRedraw = false;
}
*/
/*
bool _safeBelow(uint8_t value, uint8_t thanValue, uint8_t thanValueMinus) {
  if (thanValue > thanValueMinus) return value < (thanValue - thanValueMinus);
  else return false;
}

void _increaseSettingsValue(bool isFast) {
  if (selectedSettingsItem == 0 && isFast && tempSettingsValue < 251) tempSettingsValue += 5;
  else if (selectedSettingsItem == 0 && tempSettingsValue < 255) tempSettingsValue++;
  else if (selectedSettingsItem == 1 && isFast && tempSettingsValue < 96) tempSettingsValue += 5;
  else if (selectedSettingsItem == 1 && tempSettingsValue < 100) tempSettingsValue++;
  else if (selectedSettingsItem == 2 && isFast && _safeBelow(tempSettingsValue, config.toInvSoc, 5)) tempSettingsValue += 5;
  else if (selectedSettingsItem == 2 && tempSettingsValue < (config.toInvSoc - 1)) tempSettingsValue++;
  else if (selectedSettingsItem == 3 && isFast && _safeBelow(tempSettingsValue, config.toGridSoc, 5)) tempSettingsValue += 5;
  else if (selectedSettingsItem == 3 && tempSettingsValue < (config.toGridSoc - 1)) tempSettingsValue++;
  else if (selectedSettingsItem == 4 && isFast && tempSettingsValue < 251) tempSettingsValue += 5;
  else if (selectedSettingsItem == 4 && tempSettingsValue < 255) tempSettingsValue++;
  else if (selectedSettingsItem == 5 && isFast && _safeBelow(tempSettingsValue, config.wakeUpSoc, 5) && _less(tempSettingsValue, config.toGridSocCritical, 5)) tempSettingsValue += 5;
  else if (selectedSettingsItem == 5 && tempSettingsValue < (config.wakeUpSoc - 1) && tempSettingsValue < (config.toGridSocCritical - 1)) tempSettingsValue++;
  else if (selectedSettingsItem == 6 && isFast && _safeBelow(tempSettingsValue, config.toGridSoc, 5)) tempSettingsValue += 5;
  else if (selectedSettingsItem == 6 && tempSettingsValue < (config.toGridSoc - 1)) tempSettingsValue++;
  else if (selectedSettingsItem == 7 && tempSettingsValue < 1) tempSettingsValue++;
  else if (selectedSettingsItem == 8 && isFast && tempSettingsValue < 4) tempSettingsValue += 3;
  else if (selectedSettingsItem == 8 && tempSettingsValue < 6) tempSettingsValue++;
  else if (selectedSettingsItem == 9 && isFast && tempSettingsValue < 251) tempSettingsValue += 5;
  else if (selectedSettingsItem == 9 && tempSettingsValue < 255) tempSettingsValue++;
  else if (selectedSettingsItem == 10 && isFast && tempSettingsValue < 96) tempSettingsValue += 5;
  else if (selectedSettingsItem == 10 && tempSettingsValue < 100) tempSettingsValue++;
  else if (selectedSettingsItem == 11 && isFast && tempSettingsValue < 251) tempSettingsValue += 5;
  else if (selectedSettingsItem == 11 && tempSettingsValue < 255) tempSettingsValue++;
  else return;
  settingsScreenNeedToRedraw = true;
}

void _decreaseSettingsValue(bool isFast) {
  if (selectedSettingsItem == 0 && isFast && tempSettingsValue > 4) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 0 && tempSettingsValue > 0) tempSettingsValue--;
  else if (selectedSettingsItem == 1 && isFast && tempSettingsValue > (config.toGridSoc + 5)) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 1 && tempSettingsValue > (config.toGridSoc + 1)) tempSettingsValue--;
  else if (selectedSettingsItem == 2 && isFast && tempSettingsValue > (config.wakeUpSoc + 5) && tempSettingsValue > (config.toGridSocCritical + 5)) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 2 && tempSettingsValue > (config.wakeUpSoc + 1) && tempSettingsValue > (config.toGridSocCritical + 1)) tempSettingsValue--;
  else if (selectedSettingsItem == 3 && isFast && tempSettingsValue > (config.goToSleepSoc + 5)) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 3 && tempSettingsValue > (config.goToSleepSoc + 1)) tempSettingsValue--;
  else if (selectedSettingsItem == 4 && isFast && tempSettingsValue > 4) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 4 && tempSettingsValue > 0) tempSettingsValue--;
  else if (selectedSettingsItem == 5 && isFast && tempSettingsValue > 4) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 5 && tempSettingsValue > 0) tempSettingsValue--;
  else if (selectedSettingsItem == 6 && isFast && tempSettingsValue > (config.goToSleepSoc + 5)) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 6 && tempSettingsValue > (config.goToSleepSoc + 1)) tempSettingsValue--;
  else if (selectedSettingsItem == 7 && tempSettingsValue > 0) tempSettingsValue--;
  else if (selectedSettingsItem == 8 && isFast && tempSettingsValue > 2) tempSettingsValue -= 3;
  else if (selectedSettingsItem == 8 && tempSettingsValue > 0) tempSettingsValue--;
  else if (selectedSettingsItem == 9 && isFast && tempSettingsValue > 4) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 9 && tempSettingsValue > 0) tempSettingsValue--;
  else if (selectedSettingsItem == 10 && isFast && tempSettingsValue > 4) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 10 && tempSettingsValue > 0) tempSettingsValue--;
  else if (selectedSettingsItem == 11 && isFast && tempSettingsValue > 4) tempSettingsValue -= 5;
  else if (selectedSettingsItem == 11 && tempSettingsValue > 0) tempSettingsValue--;
  else return;

  settingsScreenNeedToRedraw = true;
}


void _loadSettingsValue() {
  if (selectedSettingsItem == 0) tempSettingsValue = config.toInvPanelPower;
  else if (selectedSettingsItem == 1) tempSettingsValue = config.toInvSoc;
  else if (selectedSettingsItem == 2) tempSettingsValue = config.toGridSoc;
  else if (selectedSettingsItem == 3) tempSettingsValue = config.toGridSocCritical;
  else if (selectedSettingsItem == 4) tempSettingsValue = config.outputThreshold;
  else if (selectedSettingsItem == 5) tempSettingsValue = config.goToSleepSoc;
  else if (selectedSettingsItem == 6) tempSettingsValue = config.wakeUpSoc;
  else if (selectedSettingsItem == 7) tempSettingsValue = config.needToSendHelloAfterReconnect;
  else if (selectedSettingsItem == 8) tempSettingsValue = config.showTemperature;
  else if (selectedSettingsItem == 9) tempSettingsValue = config.invIdle;
  else if (selectedSettingsItem == 10) tempSettingsValue = config.invEfficiency;
  else if (selectedSettingsItem == 11) tempSettingsValue = config.ignoreMinorConditionsDuration;
  encResetStates();
  settingsEditMode = true;
  settingsScreenNeedToRedraw = true;
}

void _saveSettingsValue() {
  if (selectedSettingsItem == 0) config.toInvPanelPower = tempSettingsValue;
  else if (selectedSettingsItem == 1) config.toInvSoc = tempSettingsValue;
  else if (selectedSettingsItem == 2) config.toGridSoc = tempSettingsValue;
  else if (selectedSettingsItem == 3) config.toGridSocCritical = tempSettingsValue;
  else if (selectedSettingsItem == 4) config.outputThreshold = tempSettingsValue;
  else if (selectedSettingsItem == 5) config.goToSleepSoc = tempSettingsValue;
  else if (selectedSettingsItem == 6) config.wakeUpSoc = tempSettingsValue;
  else if (selectedSettingsItem == 7) config.needToSendHelloAfterReconnect = tempSettingsValue;
  else if (selectedSettingsItem == 8) config.showTemperature = tempSettingsValue;
  else if (selectedSettingsItem == 9) config.invIdle = tempSettingsValue;
  else if (selectedSettingsItem == 10) config.invEfficiency = tempSettingsValue;
  else if (selectedSettingsItem == 11) config.ignoreMinorConditionsDuration = tempSettingsValue;
  encResetStates();
  settingsEditMode = false;
  settingsScreenNeedToRedraw = true;
}

*/




i have esp32 with oled display, and want to display log that store in variables, like ring buffer

#define LOG_SIZE 8

uint32_t modeChangeLogMillis[LOG_SIZE] = {0};

BoxMode modeChangeLogMode[LOG_SIZE] = {UNKNOWN};

uint8_t lastChangeIndex = LOG_SIZE - 1;

as example for output info is such code 