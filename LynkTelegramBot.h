#include <FastBot2.h>
//#include "driver/adc.h"

#define RECONNECT_INTERVAL 30000

FastBot2 bot;
bool needToSentHello = true;
String (*_statusFunction)() = nullptr;
uint32_t lastReconnectMillis = millis();

void updateh(fb::Update& u);


void telegramBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  lastReconnectMillis = millis();
  bot.attachUpdate(updateh);
  bot.setToken(BOT_TOKEN);
}

void enableWiFi();
void disableWiFi();

void tickTelegram() {
  bool wifi = WiFi.status() == WL_CONNECTED;
  if (wifi != boxFlags.isWifiConnected) {
    boxFlags.isWifiConnected = wifi;
    boxFlags.wifiStatusUpdated = true;
    needToSentHello = needToSentHello || config.needToSendHelloAfterReconnect;
    if (wifi) {
      Serial.print("WIFI Connected, MAC:");
      Serial.println(WiFi.macAddress());
    } else {
      Serial.println("WIFI Disconnected");
      disableWiFi();
      lastReconnectMillis = millis();
    }
  }
  if (wifi && needToSentHello) {
    needToSentHello = false;
    fb::Message msg("Controller Started(waiting OTA...)", CHAT_ID);
    fb::InlineMenu menu("Left ; Click ; Right \n Status", "left;click;right;status");
    msg.setInlineMenu(menu);
    bot.sendMessage(msg);
    Serial.println("Send hello...");
  }
  if (wifi) bot.tick();
  else if (millis() - lastReconnectMillis > RECONNECT_INTERVAL) {
    Serial.println(boxFlags.isWifiDisabled ? "WIFI Enable" : "WIFI Disable");
    if(boxFlags.isWifiDisabled) enableWiFi();
    else disableWiFi();
    lastReconnectMillis = millis();
  }
}


void updateh(fb::Update& u) {
  if (u.isQuery()) {
    Serial.print("QUERY = ");
    Serial.print(u.query().id());
    Serial.print(" ");
    Serial.println(u.query().data());
    switch (u.query().data().hash()) {
      case "left"_h:
          if(forceChangeMode == 3) forceChangeMode = 2; //TO_INV
          else if(forceChangeMode != 1) forceChangeMode = 1; //TO_GRID
        break;
      case "click"_h:
        //encCommand = ENC_CLICK;
        break;
      case "right"_h:
        if(forceChangeMode == 2) forceChangeMode = 3; //TO_INV_PLUS
        else if(forceChangeMode < 2) forceChangeMode = 2; //TO_INV
        break;
      case "status"_h:
        if(_statusFunction != nullptr) {
            bot.sendMessage(fb::Message(_statusFunction(), CHAT_ID));
            Serial.print("STATUS QUERY from"); Serial.println(u.query().from().id());
        }
        break;
    }
  } else if (!u.isMessage()) return;

  if (u.message().hasDocument()) {
    if (u.message().document().name().endsWith(".bin")) {  // .bin == ОТА
      bot.sendMessage(fb::Message("OTA begin", u.message().chat().id()));
      bot.updateFlash(u.message().document(), u.message().chat().id());
    } else {
      fb::Fetcher fetch = bot.downloadFile(u.message().document().id());
      if (fetch) {
        fetch.writeTo(Serial);
      }
    }
    return;
  }
#ifdef DEBUG_ENABLED
  Serial.println("NEW MESSAGE id/username/text");
  Serial.println(u.message().chat().id());
  Serial.println(u.message().from().username());
  Serial.println(u.message().text());
#endif
  if (u.message().chat().id() != CHAT_ID) return;

  if (_statusFunction != nullptr && u.message().text() == "/status") {
    bot.sendMessage(fb::Message(_statusFunction(), u.message().chat().id()));
    return;
  }
  bot.sendMessage(fb::Message(u.message().text(), u.message().chat().id()));
}

void attachStatusFunction(String (*handler)()) {
  _statusFunction = handler;
}

void sendToChat(String msg) {
  bot.sendMessage(fb::Message(msg, CHAT_ID));
}

void disableWiFi(){
    boxFlags.isWifiDisabled = true;
    //adc_power_off();
    WiFi.disconnect(true);  // Disconnect from the network
    WiFi.mode(WIFI_OFF);    // Switch WiFi off
}

void enableWiFi(){
    boxFlags.isWifiDisabled = false;
    //adc_power_on();
    WiFi.disconnect(false);  // Reconnect the network
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
 
}