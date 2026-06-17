/* ==================================================================== */
/* COMMON ELEMENT BUILDER */
/* ==================================================================== */
#define ITEMS_PER_PAGE 5  // +1 if xItemShift == 0

void _drawScroll(uint8_t itemsCount, uint8_t itemsPerPage, uint8_t shift) {
  uint16_t totalHeight = itemsPerPage * 8;
  uint16_t a = itemsPerPage * totalHeight;
  uint8_t scrollHeight = a / itemsCount;
  uint8_t oneElementHeight = totalHeight / itemsCount;
  uint8_t y0 = 13 + (shift * oneElementHeight);
  uint8_t y1 = y0 + scrollHeight;
  oled.fastLineV(126, y0, y1);
  //oled.fastLineV(127, y0, y1);
}

void drawWifiStatus() {
  if (boxFlags.isWifiConnected) {
    oled.drawBitmap2(120, 0, box_wifi, 7, 8);
  } else {
    oled.drawBitmap2(120, 0, box_nowifi, 7, 8);
  }
}

void drawSaveStatus() {
  oled.clear(111, 0, 119, 7);
  if (!boxFlags.isNeedToSaveConfig) return;
  oled.drawBitmap2(111, 0, box_save2, 8, 8);
}

void drawForceChangeMode() {
  oled.clear(96, 0, 110, 7);
  if (forceChangeMode == 0) return;
  oled.drawBitmap2(96, 0, box_hand, 8, 8);
  oled.setCursorXY(104, 0);
  if (forceChangeMode == 1) oled.print("G");
  else if (forceChangeMode == 2) oled.print("I");
  else if (forceChangeMode == 3) oled.print("+");
}

void buildStatusLine(const __FlashStringHelper* screenLabel) {
  oled.setCursorXY(0, 0);
  oled.print(screenLabel);
  drawWifiStatus();
  //statusBarLine
  oled.fastLineH(8, 1, 127);
}

void printItem(bool selected, const String& itemLabel, const __FlashStringHelper* prefix, bool invert = true, bool newLine = true) {
  if (selected && invert) oled.invertText(true);
  if (selected) oled.print(prefix);
  else oled.print(' ');
  oled.print(itemLabel);
  if (selected && invert) oled.invertText(false);
  if (newLine) oled.println();
}

//number should be less 1K, digits 0 - 3
void printMillis(uint16_t number, uint8_t digits = 3) {
  oled.print(".");
  if (digits == 0) return;
  for (uint8_t i = 0; i < 3 - digits; i++) number /= 10;
  oled.printNumberFmt(number, digits);
}

//number should be less 100M
void printInt6char(uint32_t number) {
  if (number < N10K) {
    if (number < 1000) oled.print("0");
    else {
      oled.print(number / 1000);
      number = number % 1000;
    }
    printMillis(number);
    oled.print("k");
  } else if (number < N1M) {
    oled.print(number / 1000);
    printMillis(number % 1000, number < N100K ? 2 : 1);
    oled.print("k");
  } else {  //if(number < N100M) {
    if (number < N10M) oled.print("0");
    oled.print(number / 1000);
    oled.print("k");
  }
}

//
//for 100M+ numbers, argument is 10 times lower than actual value, numberx01 - 10M - 2B
void printGigaInt6char(uint32_t numberx01) {
  if (numberx01 < N100M) {
    oled.print(numberx01 / N100K);  //99 988 888 -> 999.8
    numberx01 = numberx01 % N100K;
    numberx01 /= 100;
    printMillis(numberx01, 1);
    oled.print("G");
  } else {  //if(number < N10B) {
    if (numberx01 < N1B) oled.print("0");
    oled.print(numberx01 / N100K);
    oled.print("G");
  }
}

/* ==================================================================== */
/* HOME SCREEN BUILDER */
/* ==================================================================== */

//needToClear = true - if redraw over previous state, false - if was full clear before
void drawPanelInfo(bool needToClear = true);
void drawBatInfo(bool needToClear = true);
void drawBoxMode(bool needToClear = true);
void drawOutputInfo(bool needToClear = true);

void drawHomeScreen() {
  oled.clear();
  buildStatusLine(F(" Home:"));
  drawSaveStatus();
  drawForceChangeMode();
  oled.drawBitmap2(0, 9, box_solar_panel_sun, 16, 16);
  drawPanelInfo(false);
  oled.fastLineV(80, 9, 24);  //end solar info
  drawBatInfo(false);
  oled.fastLineV(71, 42, 63);  //end batt info
  drawBoxMode(false);
  //box
  oled.roundRect(76, 31, 113, 41, STROKE);
  //grid
  oled.drawBitmap2(81, 9, box_grid, 15, 16);
  //home
  oled.drawBitmap2(113, 10, box_home, 15, 16);
  //box to fridge
  oled.fastLineH(37, 116, 119);
  oled.fastLineV(120, 38, 42);
  oled.drawArrowHead(120, 45, 2);
  //fridge
  oled.drawBitmap2(114, 48, box_fridge, 14, 16);
  drawOutputInfo(false);
  oled.update();
}

void drawPanelInfo(bool needToClear) {
  if (needToClear) {
    oled.clear(16, 9, 79, 24);  //12h metering and total
    oled.clear(0, 25, 71, 32);  //arrow tail and solar power
    oled.clear(1, 33, 5, 44);   //arrow
  }

  oled.drawBitmap2(16, 10, box_12h, 8, 8);
  oled.setCursorXY(24, 10);
  uint32_t w12h = getSolarPanelMetering12h();
  if (w12h < N100M) printInt6char(w12h);
  else (printGigaInt6char(w12h / 10));
  oled.print("Wh");
  oled.drawBitmap2(16, 18, box_sum, 8, 8);
  oled.setCursorXY(24, 18);
  if (statInfo.sPanelMeteringTotal < N10M) printInt6char(statInfo.sPanelMeteringTotal * 10);
  else (printGigaInt6char(statInfo.sPanelMeteringTotal));
  oled.print("Wh");
  oled.setCursorXY(7, 26);
  oled.print(mainInfo.solarPanelPower);
  oled.print("W");

  //arrow from panel to bat
  oled.fastLineV(4, 29, 41);
  oled.drawArrowHead(4, 44, 2);
}

void drawBatInfo(bool needToClear) {
  if (needToClear) {
    oled.clear(1, 47, 13, 63);   //battery
    oled.clear(15, 40, 70, 63);  //batInfo
  }
  //bat
  oled.fastLineV(1, 49, 63);
  oled.fastLineV(13, 49, 63);
  oled.fastLineH(47, 3, 11);
  oled.fastLineH(49, 2, 12);
  oled.fastLineH(63, 2, 12);
  oled.dot(3, 48);
  oled.dot(11, 48);
  if(boxFlags.bmsReadFailed)
    oled.drawBitmap2(2, 50, box_batt_fail, 11, 13);
  //TODO else draw blocks
  //batt info
  oled.setCursorXY(15, 40);
  if (bmsData.current > 0) oled.print("←");
  else if (bmsData.current < 0) oled.print("→");
  else oled.print(" ");
  uint16_t c = bmsData.current < 0 ? -(bmsData.current) : bmsData.current;
  c = constrain(c, 0, 99999);
  oled.print(c / 100);
  printMillis((c % 100) * 10, 2);
  oled.print("A");
  oled.setCursorXY(15, 48);
  if (bmsData.soc == 100)
    oled.drawBitmap2(15, 48, box_100, 14, 8);
  else
    oled.printNumberFmt(bmsData.soc, 2);

  oled.setCursorXY(29, 48);
  oled.print("%");
  if(config.showTemperature != 0) {
    int8_t t = getBmsTemperature();
    if (t > 0) oled.print(" ");
    oled.print(constrain(t, -99, 99));
    oled.print("℃");
  }
  oled.setCursorXY(15, 56);

  uint16_t v = constrain(bmsData.totalVoltage, 0, 99999);
  oled.print(v / 100);
  printMillis((v % 100) * 10, 2);
  oled.print("v");
}

void _drawGridToBoxArrow();
void _clearGridToBoxArrow();
void _drawBoxToHomeArrow();
void _clearBoxToHomeArrow();
void _drawBatToInv();
void _drawInvToBoxArrow();
void _clearBatToBox();

void drawBoxMode(bool needToClear) {
  if (mainInfo.boxMode == UNKNOWN) {
    if (needToClear) oled.clear(81, 33, 110, 40);
    oled.setCursorXY(81, 33);
    oled.print("????");
  } else if (mainInfo.boxMode == TO_GRID) {
    if (needToClear) oled.clear(81, 33, 110, 40);
    //oled.setCursorXY(81, 33);
    //oled.print("GRID");
    _drawGridToBoxArrow();
    if (needToClear) _clearBatToBox();
  } else if (mainInfo.boxMode == GRID) {
    oled.setCursorXY(81, 33);
    //if (!needToClear)
    oled.print("GRID");
    _drawGridToBoxArrow();
  } else if (mainInfo.boxMode == INV_PREHEAT) {
    oled.setCursorXY(81, 33);
    if (!needToClear) oled.print("GRID");
    _drawBatToInv();
  } else if (mainInfo.boxMode == TO_INV) {
    if (needToClear) oled.clear(81, 33, 110, 40);
    oled.setCursorXY(81, 33);
    //oled.print("INV");
    if (!needToClear) _drawBatToInv();
    _drawInvToBoxArrow();
    if (needToClear) _clearGridToBoxArrow();
  } else if (mainInfo.boxMode == INV) {
    if (needToClear) oled.clear(81, 33, 110, 40);
    oled.setCursorXY(81, 33);
    oled.print("INV");
    if (!needToClear) {
      _drawBatToInv();
      _drawInvToBoxArrow();
    }
    if (needToClear) _clearBoxToHomeArrow();
  } else if (mainInfo.boxMode == INV_PLUS) {
    oled.setCursorXY(81, 33);
    oled.print("INV+");
    if (!needToClear) {
      _drawBatToInv();
      _drawInvToBoxArrow();
    }
    _drawBoxToHomeArrow();
  }
}

void _drawGridToBoxArrow() {
  oled.fastLineH(21, 97, 99);
  oled.fastLineV(100, 22, 25);
  oled.drawArrowHead(100, 28, 2);
}

void _clearGridToBoxArrow() {
  oled.clear(97, 21, 102, 28);
}

void _drawBoxToHomeArrow() {
  oled.fastLineH(34, 116, 119);
  oled.fastLineV(120, 30, 33);
  oled.drawArrowHead(120, 27, 0);
}

void _clearBoxToHomeArrow() {
  oled.clear(116, 27, 122, 34);
}

void _drawBatToInv() {
  //arrow from bat to box
  oled.fastLineV(10, 37, 44);
  oled.fastLineH(36, 11, 31);
  oled.drawBitmap2(34, 33, box_inv, 16, 7);
}

void _drawInvToBoxArrow() {
  oled.fastLineH(36, 52, 70);
  oled.drawArrowHead(73, 36, 1);
}

void _clearBatToBox() {
  oled.clear(10, 37, 10, 44);
  oled.clear(11, 34, 73, 39);
}

void drawOutputInfo(bool needToClear) {
  if (needToClear) {
    oled.clear(72, 48, 113, 63);
  }
  oled.setCursorXY(72, 48);
  oled.print(constrain(jsyData.power, 0, 99999));
  oled.print("W");
  oled.setCursorXY(72, 56);
  oled.print(constrain(jsyData.voltage / 100, 0, 999));
  oled.print(".");
  oled.print((jsyData.voltage % 100) / 10);
  oled.print("V");
}

#define ENCODER_TICKS_TO_CHANGE_MODE 3
int8_t encPosition = 0;

void homeHandleEncoderCommand() {
  int8_t p = encPosition;
  if (encIsRight()) encPosition++;
  if (encIsLeft()) encPosition--;
  if (encIsClick()) {
    encPosition = 0;
    screenData.goToScreen = 255;
    encResetStates();
  }
  if (p != encPosition) {
    Serial.print("position = ");
    Serial.println(encPosition);
  }
  if (encPosition >= ENCODER_TICKS_TO_CHANGE_MODE) {
    if (forceChangeMode == 2) forceChangeMode = 3;      //TO_INV_PLUS
    else if (forceChangeMode < 2) forceChangeMode = 2;  //TO_INV
    boxFlags.boxModeUpdated = true;
    encPosition = 0;
  } else if (encPosition <= -ENCODER_TICKS_TO_CHANGE_MODE) {
    if (forceChangeMode == 3) forceChangeMode = 2;       //TO_INV
    else if (forceChangeMode != 1) forceChangeMode = 1;  //TO_GRID
    boxFlags.boxModeUpdated = true;
    encPosition = 0;
  }
}


/* ==================================================================== */
/* MENU SCREEN BUILDER */
/* ==================================================================== */
uint8_t selectedMenuItem = 1;
uint8_t previousMenuItem = 1;
void drawMenuScreen(bool partialUpdate) {
  oled.clear();
  buildStatusLine(F(" Menu:"));
  drawSaveStatus();
  oled.setCursorXY(0, 9);
  printItem(selectedMenuItem == 1, F("Battery info"), F("→"));
  printItem(selectedMenuItem == 2, F("Output info"), F("→"));
  printItem(selectedMenuItem == 3, F("Statistics"), F("→"));
  printItem(selectedMenuItem == 4, F("Network"), F("→"));
  printItem(selectedMenuItem == 5, F("Log"), F("→"));
  printItem(selectedMenuItem == 6, F("Settings"), F("→"));
  oled.drawBitmap2(0, 57, box_back, 12, 7, selectedMenuItem == 7);
  if (partialUpdate) {
    uint8_t y = selectedMenuItem > previousMenuItem ? 9 + (previousMenuItem * 8) : 9 + (selectedMenuItem * 8);
    oled.update(0, 0, 127, y + 16);
  } else oled.update();
  boxFlags.menuScreenNeedToRedraw = false;
}

void menuHandleEncoderCommand() {
  previousMenuItem = selectedMenuItem;
  if (encIsRight() && selectedMenuItem < 7) {
    selectedMenuItem++;
    boxFlags.menuScreenNeedToRedraw = true;
  } else if (encIsLeft() && selectedMenuItem > 1) {
    selectedMenuItem--;
    boxFlags.menuScreenNeedToRedraw = true;
  } else if (encIsClick()) {
    screenData.goToScreen = (selectedMenuItem < 7) ? selectedMenuItem : 0;
    encResetStates();
  }
}

/* ==================================================================== */
/* BATT INFO SCREEB BUILDER */
/* ==================================================================== */
ScrollListScreen battInfoScreen(ITEMS_PER_PAGE + 1, 1);

uint8_t _cellInfoRows = 0;
uint8_t _battInfoVersion = 0;

void _recalculateBattInfoRowsAndMaxShift() {
  if (boxFlags.bmsReadFailed && bmsData.version == 0) return;
  _cellInfoRows = (bmsData.numCells + 1) / 2;
  battInfoScreen.setItemCount(8 + _cellInfoRows);
}

void _printCellInfo(uint8_t cellNo) {
  oled.print(cellNo + 1);
  oled.print(F(": "));
  oled.print(bmsData.cellVoltages[cellNo] / 1000);
  printMillis(bmsData.cellVoltages[cellNo] % 1000, 3);
}

void _drawBattInfoItem(uint8_t idx) {
  if (idx == 0) {
    oled.print(F("Status: "));
    Serial.print("============bmsReadFailed ");
    Serial.print(boxFlags.bmsReadFailed);
    if (boxFlags.bmsReadFailed) oled.println(F("Fail"));
    else {
      oled.print(F("OK #"));
      oled.println(bmsData.version);
    }
  } else if (idx == 1) {
    oled.print(bmsData.soc);
    oled.print(F("% - "));
    oled.print(constrain(bmsData.totalVoltage / 100, 0, 99));
    oled.print(".");
    oled.printNumberFmt((uint8_t)(bmsData.totalVoltage % 100), 2);
    oled.println(F("V"));
  } else if (idx >= 2 && idx < 2 + _cellInfoRows) {
    _printCellInfo(idx - 2);
    oled.print(" ");
    if ((idx - 2 + _cellInfoRows) < bmsData.numCells) _printCellInfo(idx - 2 + _cellInfoRows);
    oled.println();
  } else if (idx == (2 + _cellInfoRows)) {
    oled.print(F("Max-Min: "));
    uint16_t diff = bmsData.cellVoltages[bmsData.maxVoltageCellIndex] - bmsData.cellVoltages[bmsData.minVoltageCellIndex];
    oled.print(diff / 1000);
    printMillis(diff % 1000);
    oled.println();
  } else if (idx == (3 + _cellInfoRows)) {
    if (bmsData.current > 0) {
      oled.print(F("Charge: "));
      oled.print(constrain(bmsData.current / 100, 0, 999));
      oled.print(".");
      oled.printNumberFmt((uint8_t)(bmsData.current % 100), 2);
      oled.println(F("A"));
    } else if (bmsData.current < 0) {
      oled.print(F("Discharge: "));
      oled.print(constrain(bmsData.current / -100, 0, 999));
      oled.print(".");
      oled.printNumberFmt((uint8_t)(-bmsData.current % 100), 2);
      oled.println(F("A"));
    } else {
      oled.println(F("Zero current"));
    }
  } else if (idx == (4 + _cellInfoRows)) {
    oled.println(F("AAAA AAAA AAAA AAA"));
  } else if (idx == (5 + _cellInfoRows)) {
    oled.println(F("BBBB BBBB BBBB BBB"));
  } else if (idx == (6 + _cellInfoRows)) {
    oled.println(F("CCCC CCCC CCCC CCC"));
  } else if (idx == (7 + _cellInfoRows)) {
    oled.println(F("DDDD DDDD DDDD DDD"));
  }
}

bool isBattInfoNewVersionAvailable() {
  return _battInfoVersion != bmsData.version;
}

void drawBattInfoScreen() {
  oled.clear();
  _recalculateBattInfoRowsAndMaxShift();

  buildStatusLine(F(" Battery info:"));
  if (battInfoScreen.canScrollUp()) {
    oled.drawArrowHead(126, 10, 0, 2);
    oled.setCursorXY(0, 13);  //21 29 37 45 53 61
  } else oled.setCursorXY(0, 9);

  battInfoScreen.draw(_drawBattInfoItem);

  if (battInfoScreen.canScrollDown())
    oled.drawArrowHead(126, 63, 2, 2);
  else
    oled.drawBitmap2(0, 57, box_back, 12, 7, true);

  _drawScroll(8 + _cellInfoRows, ITEMS_PER_PAGE + 1, battInfoScreen.getShift());
  boxFlags.battInfoScreenNeedToRedraw = false;
  _battInfoVersion = bmsData.version;
  oled.update();
}

void battInfoHandleEncoderCommand() {
  if ((encIsLeft() && battInfoScreen.scrollUp())
      || (encIsRight() && battInfoScreen.scrollDown()))
    boxFlags.battInfoScreenNeedToRedraw = true;
  else if (encIsClick()) {
    screenData.goToScreen = 255;
    encResetStates();
    battInfoScreen.reset();
  }
}

/* ==================================================================== */
/* OUTPUT INFO SCREEN BUILDER */
/* ==================================================================== */

void drawOutputInfoScreen() {
  oled.clear();
  buildStatusLine(F(" Output info:"));
  oled.setCursorXY(0, 9);
  oled.print(F("Status: "));
  //1
  if (boxFlags.jsyReadFailed) oled.println(F("Fail"));
  else {
    oled.print(F("OK #"));
    oled.println(jsyData.version % 1000);
  }
  //2
  oled.print(constrain(jsyData.voltage / 100, 0, 999));
  oled.print(".");
  oled.printNumberFmt((uint8_t)(jsyData.voltage % 100), 2);
  oled.print("V ");
  oled.print(constrain(jsyData.current / 100, 0, 999));
  oled.print(".");
  oled.printNumberFmt((uint8_t)(jsyData.current % 100), 2);
  oled.println("A");
  //3
  oled.print(constrain(jsyData.power, 0, 99999));
  oled.print("W pf");
  oled.print(constrain(jsyData.pf / 1000, 0, 9));
  printMillis(jsyData.pf % 1000);
  oled.println();
  //4
  oled.print(constrain(jsyData.freq / 100, 0, 999));
  oled.print(".");
  oled.printNumberFmt((uint8_t)(jsyData.freq % 100), 2);
  oled.println("Hz");
  //5
  oled.print(jsyData.energy);
  oled.println("Wh");
  //6
  oled.print(jsyData.co2);
  oled.println("kg");

  oled.drawBitmap2(0, 57, box_back, 12, 7, true);
  boxFlags.outputInfoScreenNeedToRedraw = false;
  oled.update();
}

/* ==================================================================== */
/* STAT SCREEN BUILDER */
/* ==================================================================== */
#define STAT_ITEMS_COUNT 23
ScrollListScreen statScreen(ITEMS_PER_PAGE + 1, STAT_ITEMS_COUNT);

void _drawStatItem(uint8_t idx) {
  if (idx == 0) {
    oled.println("Solar Panel:");
  } else if (idx == 1) {
    oled.print("Update in: ");
    oled.print(60 - statInfo.sPanelLastMinCounter);
    oled.println("m");
  } else if (idx >= 2 && idx < 14) {
    oled.printNumberFmt((uint8_t)(idx - 1), 2, ' ');
    oled.print("h: ");
    oled.println("123456789Wh");
  } else if (idx == 14) {
    oled.print("Sum: ");
    oled.println("123456789Wh");
  } else if (idx == 15) {
    oled.print("Total: ");
    oled.println("123456KWh");
  } else if (idx == 16) {
    oled.println("-----------------");
  } else if (idx == 17) {
    oled.println("Output GRID:");
  } else if (idx == 18) {
    oled.println("123456KWh");
  } else if (idx == 19) {
    oled.println("12d34h56m78s");
  } else if (idx == 20) {
    oled.println("Output INV:");
  } else if (idx == 21) {
    oled.println("123456KWh");
  } else if (idx == 22) {
    oled.println("12d34h56m78s");
  }
}

void drawStatScreen() {
  oled.clear();
  buildStatusLine(F(" Statistics:"));
  if (statScreen.canScrollUp()) {
    oled.drawArrowHead(126, 10, 0, 2);
    oled.setCursorXY(0, 13);  //21 29 37 45 53 61
  } else oled.setCursorXY(0, 9);

  statScreen.draw(_drawStatItem);

  if (statScreen.canScrollDown())
    oled.drawArrowHead(126, 63, 2, 2);
  else
    oled.drawBitmap2(0, 57, box_back, 12, 7, true);

  _drawScroll(STAT_ITEMS_COUNT, ITEMS_PER_PAGE + 1, statScreen.getShift());
  boxFlags.statScreenNeedToRedraw = false;
  oled.update();
}

void statHandleEncoderCommand() {
  if ((encIsLeft() && statScreen.scrollUp())
      || (encIsRight() && statScreen.scrollDown()))
    boxFlags.statScreenNeedToRedraw = true;
  else if (encIsClick()) {
    screenData.goToScreen = 255;
    encResetStates();
    statScreen.reset();
  }
}

/* ==================================================================== */
/* NETWORK INFO SCREEN BUILDER */
/* ==================================================================== */

/**
typedef enum {
    WIFI_MODE_NULL = 0,  //Null mode
    WIFI_MODE_STA,       //Wi-Fi station mode 
    WIFI_MODE_AP,        //Wi-Fi soft-AP mode 
    WIFI_MODE_APSTA,     //Wi-Fi station + soft-AP mode 
    WIFI_MODE_NAN,       //Wi-Fi NAN mode 
    WIFI_MODE_MAX
} wifi_mode_t;

typedef enum {
  WL_NO_SHIELD = 255,  // for compatibility with WiFi Shield library
  WL_STOPPED = 254,
  WL_IDLE_STATUS = 0,
  WL_NO_SSID_AVAIL = 1,
  WL_SCAN_COMPLETED = 2,
  WL_CONNECTED = 3,
  WL_CONNECT_FAILED = 4,
  WL_CONNECTION_LOST = 5,
  WL_DISCONNECTED = 6
} wl_status_t;

*/

void drawNetworkScreen() {
  oled.clear();
  buildStatusLine(F(" Network:"));
  oled.setCursorXY(0, 9);
  //1
  oled.println(F("SSID: "));
  //2
  oled.println(WiFi.SSID());
  //3
  oled.print(F("Mode: "));
  oled.print(WiFi.getMode());
  oled.print(F(" Status: "));
  oled.println(WiFi.status());
  if(WiFi.status() == WL_CONNECTED) {
    //4
    oled.print(F("IP: "));
    oled.println(WiFi.localIP());
    //5
    oled.println(F("MAC:"));
    //6
    oled.println(WiFi.macAddress());
  } else {
    //4
    oled.print(F("Reconnect in: "));
    oled.print((RECONNECT_INTERVAL - constrain(millis() - lastReconnectMillis, 0, RECONNECT_INTERVAL)) / 1000);
    oled.println("s");
  }
  
  oled.drawBitmap2(0, 57, box_back, 12, 7, true);
  boxFlags.networkScreenNeedToRedraw = false;
  oled.update();
}

/* ==================================================================== */
/* LOG SCREEN BUILDER */
/* ==================================================================== */
uint32_t _logNow = 0;
//uint8_t logItemShift = 0;
ScrollListScreen logScreen(ITEMS_PER_PAGE + 1, LOG_SIZE);

void _printLogItem(uint8_t idx) {
  uint32_t ago = _logNow - modeChangeLogMillis[idx];
  uint8_t d = ago / 86400000;
  ago %= 86400000;
  oled.printNumberFmt(d, 2);
  //_printNumber2cFmt(d);
  oled.print("d");
  uint8_t h = ago / 3600000;
  ago %= 3600000;
  oled.printNumberFmt(h, 2);
  //_printNumber2cFmt(h);
  oled.print("h");
  uint8_t m = ago / 60000;
  ago %= 60000;
  oled.printNumberFmt(m, 2);
  //_printNumber2cFmt(m);
  oled.print("m");
  ago /= 1000;
  oled.printNumberFmt((uint8_t)ago, 2);
  //_printNumber2cFmt(ago);
  oled.print("s");
  oled.print(" ");
  oled.println(boxModeStrings4c[modeChangeLogMode[idx]]);
}

void drawLogScreen() {
  oled.clear();
  buildStatusLine(F(" Log"));
  if (logScreen.canScrollUp()) {
    oled.drawArrowHead(126, 10, 0, 2);
    oled.setCursorXY(0, 13);  //21 29 37 45 53 61
  } else oled.setCursorXY(0, 9);

  logScreen.draw(_printLogItem);

  if (logScreen.canScrollDown())
    oled.drawArrowHead(126, 63, 2, 2);
  else
    oled.drawBitmap2(0, 57, box_back, 12, 7, true);

  _drawScroll(LOG_SIZE, ITEMS_PER_PAGE + 1, logScreen.getShift());
  boxFlags.logScreenNeedToRedraw = false;
  oled.update();
}

void logHandleEncoderCommand() {
  if ((encIsLeft() && logScreen.scrollUp())
      || (encIsRight() && logScreen.scrollDown()))
    boxFlags.logScreenNeedToRedraw = true;
  else if (encIsClick()) {
    screenData.goToScreen = 255;
    encResetStates();
    logScreen.reset();
  }
}


/* ==================================================================== */
/* SETTINGS SCREEN BUILDER */
/* ==================================================================== */

uint8_t selectedSettingsItem = 0;
#define SETTINGS_ITEMS_COUNT 12
bool settingsEditMode = false;
uint8_t tempSettingsValue;


// --- Formatter function pointers ---

String fmtWattx10ZeroIsOff(uint8_t v) {
  return v ? String(v * 10) + "W" : String("Off");
}
String fmtWattx10(uint8_t v) {
  return String(((uint16_t)v) * 10) + "W";
}
String fmtPct(uint8_t v) {
  return String(v) + "%";
}
String fmtWatt(uint8_t v) {
  return String(v) + "W";
}
String fmtBool(uint8_t v) {
  return v ? F("Yes") : F("No");
}
String fmtTime(uint8_t v) {
  return String((((uint16_t)v) + 1) * 10) + "s";
}
String fmtBatTemp(uint8_t v) {
  static const char* const opts[] = { "No", "t1", "t2", "MOS", "Avg", "Min", "Max" };
  if (v >= 7) return String("?");
  return String(opts[v]);
}

// --- Dynamic bounds (return uint8_t) ---

uint8_t minToInvSoc() {
  return config.toGridSoc + 1;
}

uint8_t maxToInvSoc() {
  return 100;
}

uint8_t minToGridSoc() {
  return max(config.wakeUpSoc, config.toGridSocCritical) + 1;
}
uint8_t maxToGridSoc() {
  return config.toInvSoc - 1;
}

uint8_t minToGridSocCritical() {
  return config.goToSleepSoc + 1;
}
uint8_t maxToGridSocCritical() {
  return config.toGridSoc - 1;
}

uint8_t minGoToSleepSoc() {
  return 0;
}
uint8_t maxGoToSleepSoc() {
  return min(config.wakeUpSoc, config.toGridSocCritical) - 1;
}

uint8_t minWakeUpSoc() {
  return config.goToSleepSoc + 1;
}
uint8_t maxWakeUpSoc() {
  return config.toGridSoc - 1;
}

uint8_t minFixed0() {
  return 0;
}
uint8_t maxFixed100() {
  return 100;
}
uint8_t maxFixed255() {
  return 255;
}
uint8_t maxFixed6() {
  return 6;
}
uint8_t maxFixed1() {
  return 1;
}

struct SettingsItemDef {
  const __FlashStringHelper* label;
  uint8_t* configValue;
  uint8_t fastStep;
  uint8_t (*getMin)();
  uint8_t (*getMax)();
  String (*format)(uint8_t v);
};

const SettingsItemDef SETTINGS_DEFS[] = {
  // label              config field                           fast  getMin        getMax        formatter
  { F("ToInvSPanel"), &config.toInvSolarPanelPower, 5, minFixed0, maxFixed255, fmtWattx10ZeroIsOff },       // 0
  { F("To Inv   Soc"), &config.toInvSoc, 5, minToInvSoc, maxToInvSoc, fmtPct },                             // 1
  { F("To Grid  Soc"), &config.toGridSoc, 5, minToGridSoc, maxToGridSoc, fmtPct },                          // 2
  { F("Critical Soc"), &config.toGridSocCritical, 5, minToGridSocCritical, maxToGridSocCritical, fmtPct },  // 3
  { F("KeepInvIf >"), &config.outputThreshold, 5, minFixed0, maxFixed255, fmtWattx10 },                     // 4  (note: display is *10)
  { F("To Sleep Soc"), &config.goToSleepSoc, 5, minGoToSleepSoc, maxGoToSleepSoc, fmtPct },                 // 5
  { F("Wake  Up Soc"), &config.wakeUpSoc, 5, minWakeUpSoc, maxWakeUpSoc, fmtPct },                          // 6
  { F("Tg send hello"), &config.needToSendHelloAfterReconnect, 1, minFixed0, maxFixed1, fmtBool },          // 7
  { F("Show bat temp"), &config.showTemperature, 3, minFixed0, maxFixed6, fmtBatTemp },                     // 8
  { F("InverterIdle"), &config.invIdle, 5, minFixed0, maxFixed255, fmtWatt },                               // 9
  { F("InvEfficienc"), &config.invEfficiency, 5, minFixed0, maxFixed100, fmtPct },                          // 10
  { F("Force Time"), &config.ignoreMinorConditionsDuration, 5, minFixed0, maxFixed255, fmtTime },           // 11
};

String _getSettingsValue(uint8_t itemIdx, bool isSelectedEdit) {
  const SettingsItemDef& def = SETTINGS_DEFS[itemIdx];
  return def.format(isSelectedEdit ? tempSettingsValue : *def.configValue);
}

void _settingsItem(uint8_t itemIdx, const __FlashStringHelper* prefix) {
  printItem(selectedSettingsItem == itemIdx && !settingsEditMode, SETTINGS_DEFS[itemIdx].label, prefix, !settingsEditMode, false);
  if (selectedSettingsItem == itemIdx && settingsEditMode) printItem(true, _getSettingsValue(itemIdx, true), prefix, true);
  else printItem(false, _getSettingsValue(itemIdx, false), prefix, false);
}

void _drawSettingsItem(uint8_t itemIdx) {
  _settingsItem(itemIdx, F("→"));
}

ScrollListScreen settingsScreen(ITEMS_PER_PAGE + 1, SETTINGS_ITEMS_COUNT);

void drawSettingsScreen() {
  oled.clear();
  buildStatusLine(F(" Settings:"));
  drawSaveStatus();
  oled.setCursorXY(76, 0);
  oled.print(selectedSettingsItem < SETTINGS_ITEMS_COUNT ? selectedSettingsItem + 1 : selectedSettingsItem);
  oled.print("/");
  oled.print(SETTINGS_ITEMS_COUNT);

  if (settingsScreen.canScrollUp()) {
    oled.drawArrowHead(126, 10, 0, 2);
    oled.setCursorXY(0, 13);  //21 29 37 45 53 61
  } else oled.setCursorXY(0, 9);

  settingsScreen.draw(_drawSettingsItem);

  if (settingsScreen.canScrollDown())
    oled.drawArrowHead(126, 63, 2, 2);
  else
    oled.drawBitmap2(0, 57, box_back, 12, 7, selectedSettingsItem == SETTINGS_ITEMS_COUNT);

  _drawScroll(SETTINGS_ITEMS_COUNT, ITEMS_PER_PAGE + 1, settingsScreen.getShift());
  boxFlags.settingsScreenNeedToRedraw = false;
  oled.update();
}

void _increaseSettingsValue(bool isFast) {
  const SettingsItemDef& def = SETTINGS_DEFS[selectedSettingsItem];
  uint8_t maxV = def.getMax();
  if (tempSettingsValue >= maxV) return;
  uint8_t step = isFast ? def.fastStep : 1;
  // Check headroom before adding to avoid overflow
  uint8_t available = maxV - tempSettingsValue;
  tempSettingsValue += min(step, available);

  boxFlags.settingsScreenNeedToRedraw = true;
}

void _decreaseSettingsValue(bool isFast) {
  const SettingsItemDef& def = SETTINGS_DEFS[selectedSettingsItem];
  uint8_t minV = def.getMin();
  if (tempSettingsValue <= minV) return;
  uint8_t step = isFast ? def.fastStep : 1;
  // Check headroom before subtracting to avoid underflow
  uint8_t available = tempSettingsValue - minV;
  tempSettingsValue -= min(step, available);

  boxFlags.settingsScreenNeedToRedraw = true;
}

void _loadSettingsValue() {
  tempSettingsValue = *SETTINGS_DEFS[selectedSettingsItem].configValue;
  encResetStates();
  settingsEditMode = true;
  boxFlags.settingsScreenNeedToRedraw = true;
}

void _saveSettingsValue() {
  if (*SETTINGS_DEFS[selectedSettingsItem].configValue != tempSettingsValue) {
    *SETTINGS_DEFS[selectedSettingsItem].configValue = tempSettingsValue;
    boxFlags.isNeedToSaveConfig = true;
    lastConfigChangesMillis = millis();
  }
  encResetStates();
  settingsEditMode = false;
  boxFlags.settingsScreenNeedToRedraw = true;
}

#define FAST_SCROLL_STEPS 3

void settingsHandleEncoderCommand() {
  if (encIsFastL()) {
    if (!settingsEditMode && selectedSettingsItem > 0) {
      selectedSettingsItem -= min(selectedSettingsItem, (uint8_t)FAST_SCROLL_STEPS);
      settingsScreen.decreaseShiftToShow(selectedSettingsItem);
      boxFlags.settingsScreenNeedToRedraw = true;
    } else if (settingsEditMode) {
      _decreaseSettingsValue(true);
    }
  } else if (encIsLeft()) {
    if (!settingsEditMode && selectedSettingsItem > 0) {
      selectedSettingsItem--;
      settingsScreen.decreaseShiftToShow(selectedSettingsItem);
      boxFlags.settingsScreenNeedToRedraw = true;
    } else if (settingsEditMode) {
      _decreaseSettingsValue(false);
    }
  } else if (encIsFastR()) {
    if (!settingsEditMode && selectedSettingsItem < SETTINGS_ITEMS_COUNT) {
      uint8_t available = SETTINGS_ITEMS_COUNT - selectedSettingsItem;
      selectedSettingsItem += min(available, (uint8_t)FAST_SCROLL_STEPS);
      settingsScreen.increaseShiftToShow(selectedSettingsItem);
      boxFlags.settingsScreenNeedToRedraw = true;
    } else if (settingsEditMode) {
      _increaseSettingsValue(true);
    }
  } else if (encIsRight()) {
    if (!settingsEditMode && selectedSettingsItem < SETTINGS_ITEMS_COUNT) {
      selectedSettingsItem++;
      settingsScreen.increaseShiftToShow(selectedSettingsItem);
      boxFlags.settingsScreenNeedToRedraw = true;
    } else if (settingsEditMode) {
      _increaseSettingsValue(false);
    }
  } else if (encIsClick()) {
    if (selectedSettingsItem < SETTINGS_ITEMS_COUNT)
      if (!settingsEditMode) _loadSettingsValue();
      else _saveSettingsValue();
    else if (selectedSettingsItem == SETTINGS_ITEMS_COUNT) {
      screenData.goToScreen = 255;
      selectedSettingsItem = 0;
      encResetStates();
      settingsScreen.reset();
    }
  }
}