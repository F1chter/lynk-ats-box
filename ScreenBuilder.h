/* CONSTANTS */
#define ITEMS_PER_PAGE 6
#define SCROLL_X 126
//status bar icons
#define WIFI_X 120
#define SAVE_X 111
#define FORCE_X 96
#define STATUS_BAR_ENDY 8
#define SOLAR_INFO_END_X 79         //16px solar icon width + 8px metering icon width + 7px*8symbols - 1
#define SOLAR_INFO_ENDLINE_ENDY 24  //9 +2*8(px per line) - 1
#define BAT_X 1
#define BAT_W 12
#define BAT_Y 47
#define BAT_H 16
#define BAT_INFO_Y 40  //64 - 3*8(px per line)
#define BAT_INFO_X 15
#define BAT_INFO_ENDX 63           //15 + 7*7(px per symbol) - 1
#define BATTERY_INFO_ENDLINE_X 66  //14 + 7*7(px per symbol) + 3
#define BATTERY_INFO_ENDLINE_Y 40  //64 - 3*8
#define BOX_X 76
#define BOX_Y 31
#define BOX_ENDX 113
#define BOX_ENDY 41
#define HOME_X 113
#define FRIDGE_X 114
#define FRIDGE_Y 48


/* ==================================================================== */
/* COMMON ELEMENT BUILDER */
/* ==================================================================== */

void _drawScroll(uint8_t itemsCount, uint8_t shift) {
  uint8_t y0 = 13 + (shift * (ITEMS_PER_PAGE * 8 / itemsCount));         //y after statusbar + (shift * scaled one element height)
  uint8_t y1 = y0 + (ITEMS_PER_PAGE * ITEMS_PER_PAGE * 8 / itemsCount);  // ITEMS_PER_PAGE * 8 = visible area height;  ITEMS_PER_PAGE/itemsCount = scale
  oled.fastLineV(SCROLL_X, y0, y1);
  //oled.fastLineV(127, y0, y1);
}

void drawWifiStatus() {
  if (boxFlags.isWifiConnected)
    oled.drawBitmap2(WIFI_X, 0, box_wifi, 7, 8);
  else oled.drawBitmap2(WIFI_X, 0, box_nowifi, 7, 8);
}

void drawSaveStatus() {
  oled.clear(SAVE_X, 0, SAVE_X + 7, 7);
  if (!boxFlags.isNeedToSaveConfig) return;
  oled.drawBitmap2(SAVE_X, 0, box_save2, 8, 8);
}

void drawForceChangeMode() {
  oled.clear(FORCE_X, 0, FORCE_X + 14, 7);
  if (forceChangeMode == 0) return;
  oled.drawBitmap2(FORCE_X, 0, box_hand, 8, 8);
  oled.setCursorXY(FORCE_X + 8, 0);
  if (forceChangeMode == 1) oled.print("G");
  else if (forceChangeMode == 2) oled.print("I");
  else if (forceChangeMode == 3) oled.print("+");
}

void buildStatusLine(const __FlashStringHelper* screenLabel) {
  oled.setCursorXY(0, 0);
  oled.print(screenLabel);
  drawWifiStatus();
  oled.fastLineH(STATUS_BAR_ENDY, 1, 127);  //status bar bottom line
}

void printItem(bool selected, const String& itemLabel, const __FlashStringHelper* prefix, bool invert = true, bool newLine = true) {
  if (selected && invert) oled.invertText(true);
  if (selected) oled.print(prefix);
  else oled.print(' ');
  oled.print(itemLabel);
  if (selected && invert) oled.invertText(false);
  if (newLine) oled.println();
}

//number should be less 1K, digits 1 - 3
void printMillis(uint16_t number, uint8_t digits = 3) {
  oled.print(".");
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

void printDayHourMinSec(uint32_t seconds, uint8_t daySymbolsCount = 5) {
  uint16_t d = seconds / 86400;
  seconds %= 86400;
  oled.printNumberFmt(d, daySymbolsCount, ' ');
  oled.print("d");
  uint8_t h = seconds / 3600;
  seconds %= 3600;
  oled.printNumberFmt(h, 2, '0');
  oled.print("h");
  uint8_t m = seconds / 60;
  seconds %= 60;
  oled.printNumberFmt(m, 2);
  oled.print("m");
  oled.printNumberFmt((uint8_t)seconds, 2);
  oled.print("s");
}

/* ==================================================================== */
/* HOME SCREEN BUILDER */
/* ==================================================================== */

//needToClear = true - if redraw over previous state, false - if was full clear before
void drawSolarPanelInfo(bool needToClear = true);
void _drawBat();
void drawBatInfo(bool);
void drawBoxMode(bool needToClear = true);
void drawOutputInfo(bool needToClear = true);

void drawHomeScreen() {
  oled.clear();
  buildStatusLine(F(" Home:"));
  drawSaveStatus();
  drawForceChangeMode();
  oled.drawBitmap2(0, STATUS_BAR_ENDY + 1, box_solar_panel_sun, 16, 16);
  drawSolarPanelInfo(false);
  oled.fastLineV(SOLAR_INFO_END_X + 1, STATUS_BAR_ENDY + 1, SOLAR_INFO_ENDLINE_ENDY);  //end solar info
  _drawBat();
  drawBatInfo(false);
  oled.fastLineV(BATTERY_INFO_ENDLINE_X, BATTERY_INFO_ENDLINE_Y, 63);  //end batt info
  drawBoxMode(false);
  //box
  oled.roundRect(BOX_X, BOX_Y, BOX_ENDX, BOX_ENDY, STROKE);
  //grid
  oled.drawBitmap2(SOLAR_INFO_END_X + 2, STATUS_BAR_ENDY + 1, box_grid, 15, 16);
  //home
  oled.drawBitmap2(HOME_X, STATUS_BAR_ENDY + 2, box_home, 15, 16);
  //box to fridge
  oled.fastLineH(BOX_Y + 6, BOX_ENDX + 3, FRIDGE_X + 5);
  oled.fastLineV(FRIDGE_X + 6, BOX_Y + 7, FRIDGE_Y - 6);
  oled.drawArrowHead(FRIDGE_X + 6, FRIDGE_Y - 3, 2);
  //fridge
  oled.drawBitmap2(FRIDGE_X, FRIDGE_Y, box_fridge, 14, 16);
  drawOutputInfo(false);
  oled.update();
}
#define SOLAR_INFO_X 16
void drawSolarPanelInfo(bool needToClear) {
  if (needToClear) {

    oled.clear(SOLAR_INFO_X, STATUS_BAR_ENDY + 1, SOLAR_INFO_END_X, STATUS_BAR_ENDY + 16);            //12h metering and total
    oled.clear(SOLAR_INFO_X - 9, STATUS_BAR_ENDY + 17, SOLAR_INFO_END_X - 16, STATUS_BAR_ENDY + 24);  //arrow tail and solar power
    oled.clear(1, 33, 5, 44);                                                                         //arrow
  }

  oled.drawBitmap2(SOLAR_INFO_X, STATUS_BAR_ENDY + 2, box_12h, 7, 8);
  oled.setCursorXY(SOLAR_INFO_X + 7, 10);
  uint32_t w12h = getSolarPanelMetering12h();
  if (w12h < N100M) printInt6char(w12h);
  else (printGigaInt6char(w12h / 10));
  oled.print("Wh");
  oled.drawBitmap2(SOLAR_INFO_X, STATUS_BAR_ENDY + 10, box_sum, 6, 8);
  oled.setCursorXY(SOLAR_INFO_X + 7, STATUS_BAR_ENDY + 10);
  if (statInfo.sPanelMeteringTotal < N10M) printInt6char(statInfo.sPanelMeteringTotal * 10);
  else (printGigaInt6char(statInfo.sPanelMeteringTotal));
  oled.print("Wh");
  oled.setCursorXY(SOLAR_INFO_X - 9, STATUS_BAR_ENDY + 18);
  oled.print(constrain(solarPanelPower, 0, 999999));
  oled.print("W");

  //arrow from panel to bat
  oled.fastLineV(4, STATUS_BAR_ENDY + 21, BAT_Y - 6);
  oled.drawArrowHead(4, BAT_Y - 3, 2);
}


inline void _drawBat() {
  oled.fastLineV(BAT_X, BAT_Y + 2, BAT_Y + BAT_H);
  oled.fastLineV(BAT_X + BAT_W, BAT_Y + 2, BAT_Y + BAT_H);
  oled.fastLineH(BAT_Y, BAT_X + 2, BAT_X + BAT_W - 2);
  oled.fastLineH(BAT_Y + 2, BAT_X + 1, BAT_X + BAT_W - 1);
  oled.fastLineH(BAT_Y + BAT_H, BAT_X + 1, BAT_X + BAT_W - 1);
  oled.dot(BAT_X + 2, BAT_Y + 1);
  oled.dot(BAT_X + BAT_W - 2, BAT_Y + 1);
}

void _drawSocBlocks() {
  if (bmsData.soc == 0) return;
  oled.rect2(BAT_X + 2, BAT_Y + 12, BAT_X + 10, BAT_Y + 14, false);
  if(bmsData.soc >= 20) oled.fastLineH(BAT_Y + 13, BAT_X + 3, BAT_X + 9); //WA, need to fix fill rect
  if (bmsData.soc < 40) return;
  oled.rect2(BAT_X + 2, BAT_Y + 8, BAT_X + 10, BAT_Y + 10, false);
  if(bmsData.soc >= 60) oled.fastLineH(BAT_Y + 9, BAT_X + 3, BAT_X + 9); //WA, need to fix fill rect
  if (bmsData.soc < 80) return;
  oled.rect2(BAT_X + 2, BAT_Y + 4, BAT_X + 10, BAT_Y + 6, false);
  if(bmsData.soc >= 100) oled.fastLineH(BAT_Y + 5, BAT_X + 3, BAT_X + 9); //WA, need to fix fill rect
}

void drawBatInfo(bool needToClear) {
  if (needToClear) {
    oled.clear(BAT_X + 1, BAT_Y + 3, BAT_X + 11, BAT_Y + 15);  //battery inner
    oled.clear(BAT_INFO_X, BAT_INFO_Y, BAT_INFO_ENDX, 63);     //batInfo
  }

  if (boxFlags.bmsReadFailed) oled.drawBitmap2(BAT_X + 1, BAT_Y + 3, box_batt_fail, 11, 13);
  else _drawSocBlocks();
  //batt info
  oled.setCursorXY(BAT_INFO_X, BAT_INFO_Y);
  if (bmsData.current > 0) oled.print("←");
  else if (bmsData.current < 0) oled.print("→");
  else oled.print(" ");
  uint16_t c = bmsData.current < 0 ? -(bmsData.current) : bmsData.current;
  c = constrain(c, 0, 99999);
  oled.print(c / 100);
  printMillis((c % 100) * 10, c < N10K ? 2 : 1);
  oled.print("A");

  oled.setCursorXY(BAT_INFO_X, BAT_INFO_Y + 8);
  if (bmsData.soc == 100)
    oled.drawBitmap2(BAT_INFO_X, BAT_INFO_Y + 8, box_100, 14, 8);
  else
    oled.printNumberFmt(bmsData.soc, 2);

  oled.setCursorXY(BAT_INFO_X + 14, BAT_INFO_Y + 8);
  oled.print("%");
  if (config.showTemperature != 0) {
    int8_t t = getBmsTemperature();
    if (t > 0) oled.print(" ");
    oled.print(constrain(t, -99, 99));
    oled.print("℃");
  }

  oled.setCursorXY(BAT_INFO_X, BAT_INFO_Y + 16);
  uint16_t v = constrain(bmsData.totalVoltage, 0, 99999);
  oled.print(v / 100);
  printMillis((v % 100) * 10, 2);
  oled.print("V");
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

void _clearBoxMode() {
  oled.clear(81, 33, 110, 40);
}

void drawBoxMode(bool needToClear) {
  if (boxMode == UNKNOWN) {
    if (needToClear) _clearBoxMode();
    oled.setCursorXY(81, 33);
    oled.print("????");
  } else if (boxMode == TO_GRID) {
    if (needToClear) _clearBoxMode();
    //oled.setCursorXY(81, 33);
    //oled.print("GRID");
    _drawGridToBoxArrow();
    if (needToClear) _clearBatToBox();
  } else if (boxMode == GRID) {
    oled.setCursorXY(81, 33);
    //if (!needToClear)
    oled.print("GRID");
    _drawGridToBoxArrow();
  } else if (boxMode == INV_PREHEAT) {
    oled.setCursorXY(81, 33);
    if (!needToClear) oled.print("GRID");
    _drawBatToInv();
  } else if (boxMode == TO_INV) {
    if (needToClear) _clearBoxMode();
    oled.setCursorXY(81, 33);
    //oled.print("INV");
    if (!needToClear) _drawBatToInv();
    _drawInvToBoxArrow();
    if (needToClear) _clearGridToBoxArrow();
  } else if (boxMode == INV) {
    if (needToClear) _clearBoxMode();
    oled.setCursorXY(81, 33);
    oled.print("INV");
    if (!needToClear) {
      _drawBatToInv();
      _drawInvToBoxArrow();
    }
    if (needToClear) _clearBoxToHomeArrow();
  } else if (boxMode == INV_PLUS) {
    oled.setCursorXY(81, 33);
    oled.print("INV+");
    if (!needToClear) {
      _drawBatToInv();
      _drawInvToBoxArrow();
    }
    _drawBoxToHomeArrow();
  }
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
    //Serial.print("position = ");
    //Serial.println(encPosition);
  }
  if (encPosition >= ENCODER_TICKS_TO_CHANGE_MODE) {
    if (forceChangeMode == 0 && boxMode == INV) forceChangeMode = 3;  //TO_INV_PLUS
    else if (forceChangeMode == 2) forceChangeMode = 3;               //TO_INV_PLUS
    else if (forceChangeMode < 2) forceChangeMode = 2;                //TO_INV
    boxFlags.boxModeUpdated = true;
    encPosition = 0;
  } else if (encPosition <= -ENCODER_TICKS_TO_CHANGE_MODE) {
    if (forceChangeMode == 0 && boxMode == INV_PLUS) forceChangeMode = 2;  //TO_INV
    else if (forceChangeMode == 3) forceChangeMode = 2;                    //TO_INV
    else if (forceChangeMode != 1) forceChangeMode = 1;                    //TO_GRID
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
    encPosition = 0;
  }
}

/* ==================================================================== */
/* BATT INFO SCREEB BUILDER */
/* ==================================================================== */
ScrollListScreen battInfoScreen(ITEMS_PER_PAGE, 1);

uint8_t _cellInfoRows = 0;
uint8_t _battInfoVersion = 0;

void _recalculateBattInfoRowsAndMaxShift() {
  if (boxFlags.bmsReadFailed && bmsData.version == 0) return;
  _cellInfoRows = (bmsData.numCells + 1) / 2;
  battInfoScreen.setItemCount(9 + _cellInfoRows);
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
    //Serial.print("============bmsReadFailed ");
    //Serial.print(boxFlags.bmsReadFailed);
    if (boxFlags.bmsReadFailed) oled.println(F("Fail"));
    else {
      oled.print(F("OK #"));
      oled.println(bmsData.version);
    }
  } else if (idx == 1) {
    oled.print(F("CHRG "));
    if (bitRead(bmsData.statusInfo, 0)) oled.print("ON |");
    else oled.print("OFF|");
    oled.print(F("DCRG "));
    if (bitRead(bmsData.statusInfo, 0)) oled.println("ON ");
    else oled.println("OFF ");
    //if (bitRead(lowByte, 2)) Serial.println("Balance ON");
    //else Serial.println("Balance OFF");
  } else if (idx == 2) {
    oled.print(F("Balance "));
    if (bitRead(bmsData.statusInfo, 2)) {
      oled.println("active 1A");
      //TODO display actual balance current
    } else oled.println("inactive");
  } else if (idx == 3) {
    oled.print(bmsData.soc);
    oled.print(F("% - "));
    oled.print(constrain(bmsData.totalVoltage / 100, 0, 99));
    oled.print(".");
    oled.printNumberFmt((uint8_t)(bmsData.totalVoltage % 100), 2);
    oled.println(F("V"));
  } else if (idx == 4) {
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
  } else if (idx >= 5 && idx < (5 + _cellInfoRows)) {
    _printCellInfo(idx - 5);
    oled.print(" ");
    if ((idx - 5 + _cellInfoRows) < bmsData.numCells) _printCellInfo(idx - 5 + _cellInfoRows);
    oled.println();
  } else if (idx == (5 + _cellInfoRows)) {
    oled.print(F("Max-Min: "));
    uint16_t diff = bmsData.cellVoltages[bmsData.maxVoltageCellIndex] - bmsData.cellVoltages[bmsData.minVoltageCellIndex];
    oled.print(diff / 1000);
    printMillis(diff % 1000);
    oled.println();
  } else if (idx == (6 + _cellInfoRows)) {
    oled.print(constrain(bmsData.temp1, -99, 127));    //3
    oled.print(F("℃| "));                              //3
    oled.print(constrain(bmsData.temp2, -99, 127));    //3
    oled.print(F("℃| M "));                            //5
    oled.print(constrain(bmsData.mosTemp, -99, 127));  //3
    oled.println(F("℃"));                              //1
  } else if (idx == (7 + _cellInfoRows)) {
    oled.print(F("Raw current: "));
    oled.println(bmsData.rawCurrent);
  } else if (idx == (8 + _cellInfoRows)) {
    oled.print(F("Raw alarm: "));
    oled.println(bmsData.alarmStatus);
  } else if (idx == (9 + _cellInfoRows)) {
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

  _drawScroll(9 + _cellInfoRows, battInfoScreen.getShift());
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
  oled.print(jsyData.energyDeka / 100);
  oled.print(".");
  oled.printNumberFmt((uint8_t)(jsyData.energyDeka % 100), 2);
  oled.println("KWh");
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
#define STAT_ITEMS_COUNT 27
ScrollListScreen statScreen(ITEMS_PER_PAGE, STAT_ITEMS_COUNT);

void _drawStatItem(uint8_t idx) {
  if (idx == 0) {
    oled.println("Solar Panel:");
  } else if (idx == 1) {
    oled.print("Update in: ");  //11
    oled.print(59 - statInfo.sPanelLastMinCounter);
    oled.print("m ");  //4
    oled.print(59 - statInfo.sPanelLastSecCounter);
    oled.println("s");  //3
  } else if (idx >= 2 && idx < 14) {
    oled.printNumberFmt((uint8_t)(idx - 1), 2, ' ');
    oled.print("h: ");
    uint16_t value = statInfo.sPanelMetering12h[(14 + statInfo.sPanelMetering12hIdx - idx) % 12];  //(startIndex + count + lastMeteringIdx - itemIdx) % count
    oled.print(value / 1000);
    oled.print(".");
    oled.printNumberFmt((uint16_t)(value % 1000), 3, '0');
    oled.println("KWh");
  } else if (idx == 14) {
    oled.print("Sum: ");
    uint32_t value = getSolarPanelMetering12h();
    oled.print(value / 1000);
    oled.print(".");
    oled.printNumberFmt((uint16_t)(value % 1000), 3, '0');
    oled.println("KWh");
  } else if (idx == 15) {
    oled.print("Total: ");  //7
    if (statInfo.sPanelMeteringTotal < N10M) {
      oled.print(statInfo.sPanelMeteringTotal / 100);  //5
      oled.print(".");
      oled.printNumberFmt((uint8_t)(statInfo.sPanelMeteringTotal % 100), 2, '0');  //3
    } else oled.print(statInfo.sPanelMeteringTotal / 100);                         //8
    oled.println("KWh");
  } else if (idx == 16) {
    oled.println("------------------");
  } else if (idx == 17) {
    oled.println("Output GRID:");
  } else if (idx == 18) {
    oled.print(statInfo.gridOutputMetering / 100);  //8
    oled.print(".");
    oled.printNumberFmt((uint8_t)(statInfo.gridOutputMetering % 100), 2, '0');  //3
    oled.println("KWh");
  } else if (idx == 19) {
    printDayHourMinSec(statInfo.gridModeTime);
    oled.println();
  } else if (idx == 20) {
    oled.println("Output INV:");
  } else if (idx == 21) {
    oled.print(statInfo.invOutputMetering / 100);  //8
    oled.print(".");
    oled.printNumberFmt((uint8_t)(statInfo.invOutputMetering % 100), 2, '0');  //3
    oled.println("KWh");
  } else if (idx == 22) {
    printDayHourMinSec(statInfo.invModeTime);  //15
    oled.println();
  } else if (idx == 23) {
    oled.println("------------------");
  } else if (idx == 24) {
    oled.println("Time with <190v output:");
  } else if (idx == 25) {
    oled.print("INV:");
    printDayHourMinSec(statInfo.failTime, 4);  //15
    oled.println();
  } else if (idx == 26) {
    oled.print("GRI:");
    printDayHourMinSec(statInfo.warnTime, 4);  //15
    oled.println();
  }
}

/*
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
  uint32_t gridOutputMetering = 0;         // *0.01 KWH
  uint32_t invOutputMetering = 0;          // *0.01 KWH

} statInfo;*/

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

  _drawScroll(STAT_ITEMS_COUNT, statScreen.getShift());
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
  if (WiFi.status() == WL_CONNECTED) {
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
    oled.print((RECONNECT_INTERVAL - constrain(now - lastReconnectMillis, 0, RECONNECT_INTERVAL)) / 1000);
    oled.println("s");
  }

  oled.drawBitmap2(0, 57, box_back, 12, 7, true);
  boxFlags.networkScreenNeedToRedraw = false;
  oled.update();
}

/* ==================================================================== */
/* LOG SCREEN BUILDER */
/* ==================================================================== */

//uint8_t logItemShift = 0;
ScrollListScreen logScreen(ITEMS_PER_PAGE, LOG_SIZE + 1);

void _printLogItem(uint8_t idx) {
  if (idx == 0) {
    oled.println("Mode Change Log:");
    return;
  }
  idx -= 1;
  idx = (lastChangeIndex - idx + LOG_SIZE) % LOG_SIZE;
  uint32_t ago = now - modeChangeLogMillis[idx];
  printDayHourMinSec(ago / 1000, 2);
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

  _drawScroll(LOG_SIZE + 1, logScreen.getShift());
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
  { F("Force Time"), &config.ignoreConditionsDuration, 5, minFixed0, maxFixed255, fmtTime },                // 11
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

ScrollListScreen settingsScreen(ITEMS_PER_PAGE, SETTINGS_ITEMS_COUNT);

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

  _drawScroll(SETTINGS_ITEMS_COUNT, settingsScreen.getShift());
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
    lastConfigChangesMillis = now;
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