#define I2C_SDA 21
#define I2C_SCL 22
#define DISPLAY_UPDATE_INTERVAL 500

#define SCREEN_HOME 0
#define SCREEN_BATT_INFO 1
#define SCREEN_OUTPUT_INFO 2
#define SCREEN_STAT 3
#define SCREEN_NETWORK 4
#define SCREEN_LOG 5
#define SCREEN_SETTINGS 6
#define SCREEN_MENU 255

#include "LynkOLED.h"
#include "BoxIcons.h"
#include "ScrollListScreen.h"

LynkOLED<SSD1306_128x64> oled;

#include "ScreenBuilder.h"

uint32_t lastDisplayUpdateMillis = 0;


void setupScreen() {
  oled.begin(I2C_SDA, I2C_SCL);
  delay(1000);
  lastDisplayUpdateMillis = millis();
}

void _updateHomeScreen() {
  if (boxFlags.homeScreenNeedToRedraw) {
    drawHomeScreen();
    return;
  }

  if (boxFlags.wifiStatusUpdated) drawWifiStatus();
  if (boxFlags.solarPanelInfoUpdated) drawPanelInfo();
  if (boxFlags.battInfoUpdated) drawBatInfo();

  if (boxFlags.boxModeUpdated) {
    drawForceChangeMode();
    drawBoxMode();
  }

  if (boxFlags.outputInfoUpdated) drawOutputInfo();

  oled.update();
}

bool _currentScreenNeedsRedraw() {
  if (boxFlags.wifiStatusUpdated) return true;

  if (screenData.screen == SCREEN_HOME)
    return boxFlags.homeScreenNeedToRedraw
           || boxFlags.solarPanelInfoUpdated
           || boxFlags.battInfoUpdated
           || boxFlags.boxModeUpdated
           || boxFlags.outputInfoUpdated;
  else if (screenData.screen == SCREEN_MENU) return boxFlags.menuScreenNeedToRedraw;
  else if (screenData.screen == SCREEN_BATT_INFO)
    return boxFlags.battInfoScreenNeedToRedraw || isBattInfoNewVersionAvailable();
  else if (screenData.screen == SCREEN_OUTPUT_INFO) return boxFlags.outputInfoScreenNeedToRedraw;
  else if (screenData.screen == SCREEN_STAT) return boxFlags.statScreenNeedToRedraw;
  else if (screenData.screen == SCREEN_NETWORK) return boxFlags.networkScreenNeedToRedraw;
  else if (screenData.screen == SCREEN_LOG) return boxFlags.logScreenNeedToRedraw;
  else if (screenData.screen == SCREEN_SETTINGS) return boxFlags.settingsScreenNeedToRedraw;

  return false;
}

void _drawCurrentScreen(bool partialUpdate = true) {
  if (screenData.screen == SCREEN_HOME) {
    if (partialUpdate) _updateHomeScreen();
    else drawHomeScreen();
  } else if (screenData.screen == SCREEN_MENU) drawMenuScreen(partialUpdate);
  else if (screenData.screen == SCREEN_BATT_INFO) drawBattInfoScreen();
  else if (screenData.screen == SCREEN_OUTPUT_INFO) drawOutputInfoScreen();
  else if (screenData.screen == SCREEN_STAT) drawStatScreen();
  else if (screenData.screen == SCREEN_NETWORK) drawNetworkScreen();
  else if (screenData.screen == SCREEN_LOG) drawLogScreen();
  else if (screenData.screen == SCREEN_SETTINGS) drawSettingsScreen();
}

void tickScreen() {
  if (screenData.screen != screenData.goToScreen) {
    Serial.print(F("Go to "));
    Serial.println(screenData.goToScreen);
    screenData.screen = screenData.goToScreen;
    _drawCurrentScreen(false);
    return;
  }

  if (!_currentScreenNeedsRedraw()) return;

  _drawCurrentScreen(true);
}

void simpleHandleEncoderCommand() {
  if (encIsClick()) {
    screenData.goToScreen = 255;
    encResetStates();
  }
}

void handleEncoderCommand() {
  if (screenData.screen == SCREEN_HOME) homeHandleEncoderCommand();
  else if (screenData.screen == SCREEN_MENU) menuHandleEncoderCommand();
  else if (screenData.screen == SCREEN_BATT_INFO) battInfoHandleEncoderCommand();
  else if (screenData.screen == SCREEN_OUTPUT_INFO) simpleHandleEncoderCommand();
  else if (screenData.screen == SCREEN_STAT) statHandleEncoderCommand();
  else if (screenData.screen == SCREEN_NETWORK) simpleHandleEncoderCommand();
  else if (screenData.screen == SCREEN_LOG) logHandleEncoderCommand();
  else if (screenData.screen == SCREEN_SETTINGS) settingsHandleEncoderCommand();
}

void setFlagToRedrawCurrentScreen() {
  if (screenData.screen == SCREEN_HOME) boxFlags.homeScreenNeedToRedraw = true;
  else if (screenData.screen == SCREEN_MENU) boxFlags.menuScreenNeedToRedraw = true;
  else if (screenData.screen == SCREEN_BATT_INFO) boxFlags.battInfoScreenNeedToRedraw = true;
  else if (screenData.screen == SCREEN_OUTPUT_INFO) boxFlags.outputInfoScreenNeedToRedraw  = true;
  else if (screenData.screen == SCREEN_STAT) boxFlags.statScreenNeedToRedraw  = true;
  else if (screenData.screen == SCREEN_NETWORK) boxFlags.networkScreenNeedToRedraw = true;
  else if (screenData.screen == SCREEN_LOG) boxFlags.logScreenNeedToRedraw = true;
  else if (screenData.screen == SCREEN_SETTINGS) boxFlags.settingsScreenNeedToRedraw = true;
}