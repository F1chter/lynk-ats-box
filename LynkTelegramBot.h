#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>
#include <StreamString.h>
//#include "LynkJsonHelper.h"

#define HOSTNAME "ESP32-ATS-BOX"
#define TG_POLL_INTERVAL 10000    //10s
#define RECONNECT_INTERVAL 30000  //30s
#define MESSAGE_BUFFER_SIZE 32

struct JsonValue {
  uint32_t unsignedIntValue = 0;
  bool boolValue = false;  //isNegative for int value
} jsonValue;

bool needToSendHello = true;
//bool isWifiConnected = false;
//bool isWifiDisabled = false;

int32_t _updateId = 0;
HTTPClient* _http = nullptr;
uint8_t _otaState = 0;  //0 - no ota; 1 - fileId(from getUpdates); 2 - filePath(from getFile); 3 - file writed in ota segment -> write OK and restartESP
int32_t lastTgPollMillis = 0;
int32_t lastReconnectMillis = 0;
String filePath;
int _otaErrorCode = 0;
String _otaInitiatorId;

String (*_statusFunction)() = nullptr;

struct TgMessage {
  String userId;
  String chatId;
  String text;
  String fileId;
  String fileName;
  void reserve() {
    userId.reserve(16);
    chatId.reserve(16);
    text.reserve(MESSAGE_BUFFER_SIZE);
    fileId.reserve(71);
    fileName.reserve(32);
  }
  void clear() {
    userId.clear();
    chatId.clear();
    text.clear();
    fileId.clear();
    fileName.clear();
  }
} tgMessage;

//Methods declarations
void _disableWiFi();
void _enableWiFi();
uint8_t sendMessage(const String msg, const String chatId, bool withMenu = false);
uint8_t tickManual();


void telegramBegin() {
  _http = new HTTPClient;
  tgMessage.reserve();
  filePath.reserve(64);
  _otaInitiatorId.reserve(9);
  _enableWiFi();
  needToSendHello = true;
  lastReconnectMillis = now;
  lastTgPollMillis = now;
}

void tickTelegram() {
  bool wifi = WiFi.status() == WL_CONNECTED;
  if (wifi != boxFlags.isWifiConnected) {
    boxFlags.isWifiConnected = wifi;
    if (wifi) {
      //Serial.print("WIFI Connected, MAC:");
      //Serial.println(WiFi.macAddress());
      needToSendHello = needToSendHello || config.needToSendHelloAfterReconnect;
    } else {
      //Serial.println("WIFI Disconnected, disable Wifi...");
      _disableWiFi();
      lastReconnectMillis = now;
    }
  }
  if (wifi && needToSendHello) {
    needToSendHello = false;
    sendMessage("Controller Connected🔗(v1.0)\n🫳Force change to:", ADMIN_ID, true);
    //Serial.println("Send hello...");
  }
  if (wifi) {
    if (now - lastTgPollMillis < TG_POLL_INTERVAL) return;
    tickManual();
    lastTgPollMillis = now;
  } else if (now - lastReconnectMillis > RECONNECT_INTERVAL) {
    //Serial.println(boxFlags.isWifiDisabled ? "Try to reconnect, enable wifi..." : "Reconnect failed, disable wifi...");
    if (boxFlags.isWifiDisabled) _enableWiFi();
    else _disableWiFi();
    lastReconnectMillis = now;
  }
}

void handleUpdate() {
  /*
  Serial.print("New message from:");
  Serial.print(tgMessage.userId);
  Serial.print(" chat: ");
  Serial.print(tgMessage.chatId);
  Serial.print(" text: ");
  Serial.print(tgMessage.text);
  Serial.print(" fileName: ");
  Serial.print(tgMessage.fileName);
  Serial.print(" fileId: ");
  Serial.println(tgMessage.fileId);*/
  //uint8_t status = sendMessage(tgMessage.text, tgMessage.chatId);
  if (tgMessage.userId != ADMIN_ID) {
    String s = "new user:";
    s += tgMessage.userId;
    //s+=" status ";
    //s+=status;
    sendMessage(s, ADMIN_ID);
    return;
  }
  if (tgMessage.text == "status" && _statusFunction != nullptr) {
    sendMessage(_statusFunction(), tgMessage.chatId);
  } else if (tgMessage.text == "resetstat1234") {
    boxFlags.isNeedToResetJsy = true;
    boxFlags.isNeedToResetStatInNVS = true;
    sendMessage("Reset stat OK", tgMessage.chatId);
  }
  if (tgMessage.fileName.endsWith(F(".bin"))) {
    _otaState = 1;
  }
}

void attachStatusFunction(String (*handler)()) {
  _statusFunction = handler;
}

void _addInlineMenuHeader(String&);
void _addInlineMenuItem(String&, const String, const String);
void _addInlineMenuNewLine(String&);
void _addInlineMenuFooter(String&);

void _addMenu(String& req) {
  _addInlineMenuHeader(req);
  _addInlineMenuItem(req, "🏙 GRID ", "togrid");
  req += ',';
  _addInlineMenuItem(req, "❄ INV ", "toinv");
  req += ',';
  _addInlineMenuItem(req, "❄+🏠 INV+ ", "toplus");
  _addInlineMenuNewLine(req);
  _addInlineMenuItem(req, "Status", "status");
  _addInlineMenuFooter(req);
}

uint8_t _requestFilePath();
uint8_t _getUpdates();
int8_t _updateFirmwareOTA();

uint8_t tickManual() {
  if (_otaState > 0) {
    if (_otaState == 1) {
      if (_requestFilePath() == 1)
        _otaState = 2;
      else
        _otaState = 0;
      _otaInitiatorId = tgMessage.chatId;
      //return 1; //make one more _getUpdates to remove firmware from updates, firmware should not appear after reboot
    } else if (_otaState == 2) {
      sendMessage("Downloading firmware...", _otaInitiatorId);
      int8_t updateStatus = _updateFirmwareOTA();
      if (updateStatus == 2)
        _otaState = 3;
      else {
        String errorMsg;
        errorMsg += F("Error! Code: ");
        errorMsg += _otaErrorCode;
        sendMessage(errorMsg, _otaInitiatorId);
        _otaState = 0;
      }
      return 1;
    } else if (_otaState == 3) {
      sendMessage("OK, Rebooting...", _otaInitiatorId);
      ESP.restart();
    }
  }
  tgMessage.clear();
  uint8_t status = _getUpdates();
  if (status == 1) handleUpdate();
  //else if (status != 0) {
  //  Serial.print("Error: ");
  //  Serial.println(status);
  //}
  return status;
}

void _addToken(String& req) {
  req.reserve(150);
  req += F("https://api.telegram.org/bot");
  req += TG_BOT_TOKEN;
}

uint8_t _sendRequest(String&);
uint8_t _parseGetFileResponse(const String&);

uint8_t _requestFilePath() {
  if (tgMessage.fileId.length() == 0) return 3;
  String req;
  _addToken(req);
  req += F("/getFile?file_id=");
  req += tgMessage.fileId;
  uint8_t status = _sendRequest(req);
  if (status != 4 && status != 1) _http->end();
  if (status != 1) return status;

  int size = _http->getSize();
  bool ovfFlag = size > 25000;
  if (size) {
    StreamString sstring;
    if (!ovfFlag && sstring.reserve(size + 1)) {
      _http->writeToStream(&sstring);
      _http->end();
      return _parseGetFileResponse(sstring);
    } else status = 2;  //overflow
  } else status = 3;    //empty response
  _http->end();
  return status;
}

void _addChatId(String& req, const String& id) {
  req += F("&chat_id=");
  req += id;
}

void _addInlineMenuHeader(String& req) {
  req += F("&reply_markup={\"inline_keyboard\":[[");
}

void urlEncode(const String& s, String& dest);

void _addInlineMenuItem(String& req, const String itemName, const String itemCmd) {
  req += F("{\"text\":\"");
  urlEncode(itemName, req);
  if (itemCmd.startsWith(F("http"))) req += F("\",\"url\":\"");
  else req += F("\",\"callback_data\":\"");
  req += itemCmd;
  req += F("\"}");
}

void _addInlineMenuNewLine(String& req) {
  req += F("],[");
}

void _addInlineMenuFooter(String& req) {
  req += F("]]}");
}


uint8_t sendMessage(const String msg, const String chatId, bool withMenu) {
  String req;
  _addToken(req);
  req += F("/sendMessage?text=");
  urlEncode(msg, req);
  if (withMenu) _addMenu(req);
  //if (parseMode == FB_MARKDOWN) s += F("&parse_mode=MarkdownV2");
  //else if (parseMode == FB_HTML) s += F("&parse_mode=HTML");
  //if (!notif) req += F("&disable_notification=true");
  _addChatId(req, chatId);
  //Serial.print("send message: ");
  //Serial.println(req);
  uint8_t status = _sendRequest(req);
  if (status != 4) _http->end();
  return status;
}



uint8_t _sendRequest(String& req) {
  if (!_http->begin(req)) return 4;
  int answ = _http->GET();
  if (answ == -1 && _http) {  // ESP32 Workaround retry
    //Serial.println("get returns -1, retry connect...");
    _http->end();
    delete _http;
    _http = new HTTPClient;
    if (!_http->begin(req)) return 4;
    answ = _http->GET();
  }
  uint8_t status = 1;
  if (answ == HTTP_CODE_OK && _http->getSize()) {
    //parseRequest(_http->getString());
  } else {
    status = 3;
    //Serial.print("HTTP Client return:");
    //Serial.println(answ);
  }
  //_http->end();
  return status;
}


uint8_t _parseMessage(const String&);
uint8_t _getUpdates() {
  //if (!*_callback) return 7;
  String req;
  //req.reserve(120);
  _addToken(req);
  req += F("/getUpdates?limit=1");
  if (_updateId != 0) {
    req += F("&offset=");
    req += _updateId;
  }
  // req += F("&allowed_updates=[\"update_id\",\"message\",\"edited_message\",\"channel_post\",\"edited_channel_post\",\"callback_query\"]");
  uint8_t status = _sendRequest(req);
  if (status != 4 && status != 1) _http->end();
  if (status != 1) return status;

  int size = _http->getSize();
  bool ovfFlag = size > 25000;
  if (size) {
    StreamString sstring;
    if (!ovfFlag && sstring.reserve(size + 1)) {
      _http->writeToStream(&sstring);
      _http->end();
      return _parseMessage(sstring);
    } else status = 2;  //overflow
  } else status = 3;    //empty response
  _http->end();
  return status;
}

bool isNextJsonString(const String& str, int16_t startPos = 0);
bool isNextJsonBool(const String& str, int16_t startPos = 0);
bool isNextJsonInteger(const String& str, int16_t startPos = 0);
bool isNextJsonArray(const String& str, int16_t startPos = 0);
bool isNextJsonNode(const String& str, int16_t startPos = 0);
uint16_t skipJsonValue(const String& str, int16_t startPos = 0, char alsoSkip = ' ');
uint16_t skipSymbolsJson(const String& str, int16_t startPos = 0, char alsoSkip = ' ');
uint16_t endJsonNodePos(const String& str, int16_t startPos = 0);
uint16_t endJsonArrayPos(const String& str, int16_t startPos = 0);
uint16_t endJsonIntegerPos(const String& str, int16_t startPos = 0);
uint16_t parseJsonInteger(const String& str, int16_t startPos = 0);
uint16_t parseJsonBool(const String& str, String& out, int16_t startPos = 0);
uint16_t parseJsonString(const String& str, String& out, int16_t startPos = 0);
uint16_t endJsonStringPos(const String& str, int16_t startPos = 0);
uint8_t _parseOrdinaryMessage(const String&, int16_t);
uint8_t _parseCallbackQuery(const String&, int16_t);

uint8_t _parseMessage(const String& str) {
  //Serial.print("raw message: ");
  //Serial.println(str);
  if (!str.startsWith(F("{\"ok\":true"))) return 3;  // error
  // update_id The update's unique identifier. Update identifiers start from a certain positive number and increase sequentially. If there are no new updates for at least a week, then identifier of the next update will be chosen randomly instead of sequentially.
  int16_t startPos = str.indexOf(F("{\"update_id\":"), 0);
  if (startPos < 0) {
    _updateId = 0;  //reset _updateId
    return 0;       //no update_id
  }
  startPos += 13;
  //update_id
  if (!isNextJsonInteger(str, startPos)) return 3;  ////no update_id value
  startPos = parseJsonInteger(str, startPos);
  _updateId = jsonValue.unsignedIntValue + 1;  //will used in next read as offset
  //Serial.print("new offset: ");
  //Serial.println(_updateId);
  //message type
  startPos = skipSymbolsJson(str, startPos, ',');
  if (!isNextJsonString(str, startPos)) return 3;  //no message type
  String messageType;
  startPos = parseJsonString(str, messageType, startPos);
  startPos = skipSymbolsJson(str, startPos, ':');
  if (messageType == "message") {
    return _parseOrdinaryMessage(str, startPos);
  } else if (messageType == "callback_query") {
    _parseCallbackQuery(str, startPos);
  } else {
    Serial.print("Unhandled message with type: ");
    Serial.println(messageType);
  }
  return 1;
}

uint8_t _parseGetFileResponse(const String& str) {
  //Serial.print("raw message: ");
  //Serial.println(str);
  if (!str.startsWith(F("{\"ok\":true,"))) return 3;  // error
  int16_t startPos = str.indexOf(F("\"result\":"), 0);
  if (startPos < 0) return 3;  //no result
  startPos += 9;
  if (!isNextJsonNode(str, startPos)) return 3;  //no result value
  int16_t endPos = endJsonNodePos(str, startPos) - 1;
  startPos = skipSymbolsJson(str, startPos, '{');
  while (startPos < endPos) {
    if (!isNextJsonString(str, startPos)) {
      //Serial.print("can't parse next node: ");
      //Serial.println(str.substring(startPos));
      return 3;
    }
    String field;
    startPos = parseJsonString(str, field, startPos);
    startPos = skipSymbolsJson(str, startPos, ':');
    if (field == "file_path") {
      if (!isNextJsonString(str, startPos)) {
        //Serial.print("can't parse field value ");
        //Serial.println(str.substring(startPos));
        return 3;
      }
      String p;
      startPos = parseJsonString(str, p, startPos);
      //Serial.print("File path: https://api.telegram.org/file/bot*******/");
      //Serial.println(p);
      if (p.length() != 0) {
        String fullPath;
        fullPath = F("https://api.telegram.org/file/bot");
        fullPath += TG_BOT_TOKEN;
        fullPath += '/';
        fullPath += p;
        filePath = fullPath;

        return 1;
      } else return 3;
    } else {
      //Serial.print("Unhandled field: ");
      //Serial.println(field);
      startPos = skipJsonValue(str, startPos);
    }
    startPos = skipSymbolsJson(str, startPos, ',');
  }
  return 1;
}


uint8_t _parseFromPart(const String&, int16_t, int16_t);
uint8_t _parseChatPart(const String&, int16_t, int16_t);
uint8_t _parseDocumentPart(const String&, int16_t, int16_t);
uint8_t _parseOrdinaryMessage(const String& str, int16_t startPos) {

  if (!isNextJsonNode(str, startPos)) return 3;  //no message value
  int16_t endPos = endJsonNodePos(str, startPos) - 1;
  startPos = skipSymbolsJson(str, startPos, '{');
  //Serial.print("ordinary message: ");
  //Serial.println(str.substring(startPos, endPos));
  while (startPos < endPos) {
    if (!isNextJsonString(str, startPos)) {
      //Serial.print("can't parse next node: ");
      //Serial.println(str.substring(startPos));
      return 3;
    }
    String field;
    startPos = parseJsonString(str, field, startPos);
    startPos = skipSymbolsJson(str, startPos, ':');
    if (field == "from") {
      int16_t endUserPos = endJsonNodePos(str, startPos);
      //Serial.print("from: ");
      //Serial.println(str.substring(startPos, endUserPos));
      _parseFromPart(str, startPos, endUserPos);
      startPos = endUserPos;
    } else if (field == "chat") {
      int16_t endChatPos = endJsonNodePos(str, startPos);
      //Serial.print("chat: ");
      //Serial.println(str.substring(startPos, endChatPos));
      _parseChatPart(str, startPos, endChatPos);
      startPos = endChatPos;
    } else if (field == "document") {
      int16_t endDocumentPos = endJsonNodePos(str, startPos);
      //Serial.print("document: ");
      //Serial.println(str.substring(startPos, endDocumentPos));
      _parseDocumentPart(str, startPos, endDocumentPos);
      startPos = endDocumentPos;
    } else if (field == "text") {
      if (!isNextJsonString(str, startPos)) {
        //Serial.print("can't parse field value ");
        //Serial.println(str.substring(startPos));
        return 3;
      }
      startPos = parseJsonString(str, tgMessage.text, startPos);
    } else {
      //Serial.print("Unhandled field: ");
      //Serial.println(field);
      startPos = skipJsonValue(str, startPos);
    }
    startPos = skipSymbolsJson(str, startPos, ',');
  }

  return 1;
}

uint8_t extrachChatInfoFromOrdinaryMessage(const String& str, int16_t startPos, int16_t endPos) {

  if (!isNextJsonNode(str, startPos)) return 3;  //no message value
  endPos--;
  startPos = skipSymbolsJson(str, startPos, '{');
  //Serial.print("ordinary message: ");
  //Serial.println(str.substring(startPos, endPos));
  while (startPos < endPos) {
    if (!isNextJsonString(str, startPos)) {
      //Serial.print("can't parse next node: ");
      //Serial.println(str.substring(startPos));
      return 3;
    }
    String field;
    startPos = parseJsonString(str, field, startPos);
    startPos = skipSymbolsJson(str, startPos, ':');
    if (field == "chat") {
      int16_t endChatPos = endJsonNodePos(str, startPos);
      //Serial.print("chat: ");
      //Serial.println(str.substring(startPos, endChatPos));
      _parseChatPart(str, startPos, endChatPos);
      return 1;
      startPos = endChatPos;
    } else {
      //Serial.print("Unhandled field: ");
      //Serial.println(field);
      startPos = skipJsonValue(str, startPos);
    }
    startPos = skipSymbolsJson(str, startPos, ',');
  }

  return 1;
}

uint8_t _parseCallbackQuery(const String& str, int16_t startPos) {
  if (!isNextJsonNode(str, startPos)) return 3;  //no callback value
  int16_t endPos = endJsonNodePos(str, startPos) - 1;
  startPos = skipSymbolsJson(str, startPos, '{');
  Serial.print("callbackQuery message: ");
  Serial.println(str.substring(startPos, endPos));
  while (startPos < endPos) {
    if (!isNextJsonString(str, startPos)) {
      //Serial.print("can't parse next node: ");
      //Serial.println(str.substring(startPos));
      return 3;
    }
    String field;
    startPos = parseJsonString(str, field, startPos);
    startPos = skipSymbolsJson(str, startPos, ':');
    if (field == "from") {
      int16_t endUserPos = endJsonNodePos(str, startPos);
      //Serial.print("from: ");
      //Serial.println(str.substring(startPos, endUserPos));
      _parseFromPart(str, startPos, endUserPos);
      startPos = endUserPos;
    } else if (field == "message") {
      int16_t endMessagePos = endJsonNodePos(str, startPos);
      //Serial.print("chat: ");
      //Serial.println(str.substring(startPos, endChatPos));
      extrachChatInfoFromOrdinaryMessage(str, startPos, endMessagePos);
      startPos = endMessagePos;
    } else if (field == "data") {
      if (!isNextJsonString(str, startPos)) {
        //Serial.print("can't parse field value ");
        //Serial.println(str.substring(startPos));
        return 3;
      }
      startPos = parseJsonString(str, tgMessage.text, startPos);
    } else {
      //Serial.print("Unhandled field: ");
      //Serial.println(field);
      startPos = skipJsonValue(str, startPos);
    }
    startPos = skipSymbolsJson(str, startPos, ',');
  }
  return 1;
}


uint8_t _parseFromPart(const String& str, int16_t startPos, int16_t endPos) {
  startPos = skipSymbolsJson(str, startPos, '{');
  endPos--;
  while (startPos < endPos) {
    if (!isNextJsonString(str, startPos)) {
      //Serial.print("can't parse next node: ");
      //Serial.println(str.substring(startPos, endPos));
      return 3;
    }
    String field;
    startPos = parseJsonString(str, field, startPos);
    startPos = skipSymbolsJson(str, startPos, ':');
    if (field == "id") {
      if (!isNextJsonInteger(str, startPos)) {
        //Serial.print("can't parse id: ");
        //Serial.println(str.substring(startPos, endPos));
        return 3;
      }
      uint16_t endIdPos = endJsonIntegerPos(str, startPos);
      tgMessage.userId = str.substring(startPos, endIdPos);
      return 1;  //id found, not else needed
    } else {
      //Serial.print("Unhandled field: ");
      //Serial.println(field);
      startPos = skipJsonValue(str, startPos);
    }
    startPos = skipSymbolsJson(str, startPos, ',');
  }
  return 1;
}

uint8_t _parseChatPart(const String& str, int16_t startPos, int16_t endPos) {
  startPos = skipSymbolsJson(str, startPos, '{');
  endPos--;
  while (startPos < endPos) {
    if (!isNextJsonString(str, startPos)) {
      //Serial.print("can't parse next node: ");
      //Serial.println(str.substring(startPos, endPos));
      return 3;
    }
    String field;
    startPos = parseJsonString(str, field, startPos);
    startPos = skipSymbolsJson(str, startPos, ':');
    if (field == "id") {
      if (!isNextJsonInteger(str, startPos)) {
        //Serial.print("can't  parse id: ");
        //Serial.println(str.substring(startPos, endPos));
        return 3;
      }
      uint16_t endIdPos = endJsonIntegerPos(str, startPos);
      tgMessage.chatId = str.substring(startPos, endIdPos);
      return 1;  //id found, not else needed
    } else {
      //Serial.print("Unhandled field: ");
      //Serial.println(field);
      startPos = skipJsonValue(str, startPos);
    }
    startPos = skipSymbolsJson(str, startPos, ',');
  }

  return 1;
}

uint8_t _parseDocumentPart(const String& str, int16_t startPos, int16_t endPos) {
  startPos = skipSymbolsJson(str, startPos, '{');
  endPos--;
  while (startPos < endPos) {
    if (!isNextJsonString(str, startPos)) {
      //Serial.print("can't parse next node: ");
      //Serial.println(str.substring(startPos, endPos));
      return 3;
    }
    String field;
    startPos = parseJsonString(str, field, startPos);
    startPos = skipSymbolsJson(str, startPos, ':');
    if (field == "file_name") {
      if (!isNextJsonString(str, startPos)) {
        //Serial.print("can't parse file_name value: ");
        //Serial.println(str.substring(startPos, endPos));
        return 3;
      }
      startPos = parseJsonString(str, tgMessage.fileName, startPos);
    } else if (field == "file_id") {
      if (!isNextJsonString(str, startPos)) {
        //Serial.print("can't parse file_id value: ");
        //Serial.println(str.substring(startPos, endPos));
        return 3;
      }
      startPos = parseJsonString(str, tgMessage.fileId, startPos);
    } else {
      //Serial.print("Unhandled field: ");
      //Serial.println(field);
      startPos = skipJsonValue(str, startPos);
    }
    startPos = skipSymbolsJson(str, startPos, ',');
  }
  return 1;
}

void _sendErrorUpdateOtaCode(int errorCode) {
  //Serial.print("UPDATE ERROR: ");
  //Serial.println(errorCode);
  _otaErrorCode = errorCode;
}

int8_t _updateFirmwareOTA() {
  if (filePath.length() == 0) return -1;
  WiFiClientSecure client;
  client.setInsecure();
  httpUpdate.rebootOnUpdate(false);
  httpUpdate.onError(_sendErrorUpdateOtaCode);
  return httpUpdate.update(client, filePath);
}

void urlEncode(const String& s, String& dest) {
  dest.reserve(s.length() + 32);
  char c;
  for (uint16_t i = 0; i < s.length(); i++) {
    c = s[i];
    if (c == ' ') dest += '+';
    else if (c <= 38 || c == '+') {
      dest += '%';
      dest += (char)((c >> 4) + (((c >> 4) > 9) ? 87 : 48));
      dest += (char)((c & 0xF) + (((c & 0xF) > 9) ? 87 : 48));
    } else dest += c;
  }
}

void _disableWiFi() {
  boxFlags.isWifiDisabled = true;
  //adc_power_off();
  WiFi.disconnect(true);  // Disconnect from the network
  WiFi.mode(WIFI_OFF);    // Switch WiFi off
}

void _enableWiFi() {
  boxFlags.isWifiDisabled = false;
  //adc_power_on();
  WiFi.disconnect(false);  // Reconnect the network
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  WiFi.begin();
  //WiFi.begin(WIFI_SSID, WIFI_PASS);
}

/* RESPONSE JSON PARSE HELPER FUNCTIONS*/

//start from "
bool isNextJsonString(const String& str, int16_t startPos) {
  if (startPos < 0 || startPos >= str.length()) return false;
  //Serial.println((int)str[startPos]);
  return str[startPos] == '"';
}

//start with t or f
bool isNextJsonBool(const String& str, int16_t startPos) {
  if (startPos < 0 || startPos >= str.length()) return false;
  return str[startPos] == 't' || str[startPos] == 'f';
}

//start with - or digit
bool isNextJsonInteger(const String& str, int16_t startPos) {
  if (startPos < 0 || startPos >= str.length()) return false;
  return str[startPos] == '-' || isDigit(str[startPos]);
}

bool isNextJsonArray(const String& str, int16_t startPos) {
  if (startPos < 0 || startPos >= str.length()) return false;
  return str[startPos] == '[';
}

bool isNextJsonNode(const String& str, int16_t startPos) {
  if (startPos < 0 || startPos >= str.length()) return false;
  return str[startPos] == '{';
}

//return endPos - position of close "
uint16_t endJsonStringPos(const String& str, int16_t startPos) {
  if (str[startPos] != '"') return startPos;
  bool escaped = false;
  startPos++;
  while (startPos < str.length()) {
    if (!escaped && str[startPos] == '\\') escaped = true;
    else if (!escaped && str[startPos] == '"') return startPos;
    else if (escaped) escaped = false;
    startPos++;
  }
  Serial.println("Close \" not found");
  return startPos;
}

uint16_t parseJsonString(const String& str, String& out, int16_t startPos) {
  int16_t endPos = endJsonStringPos(str, startPos);
  if (startPos + 1 < endPos) out = str.substring(startPos + 1, endPos);
  return endPos + 1;
}

//return endPos - position of last letter + 1
uint16_t parseJsonBool(const String& str, String& out, int16_t startPos) {
  if (str[startPos] == 't' && str.substring(startPos, startPos + 4) == "true") {
    jsonValue.boolValue = true;
    return (uint16_t)(startPos + 4);
  } else if (str[startPos] == 'f' && str.substring(startPos, startPos + 5) == "false") {
    jsonValue.boolValue = false;
    return (uint16_t)(startPos + 5);
  }
  //Serial.print("Can't parse bool str[");
  //Serial.print(startPos);
  //Serial.print("]=");
  //Serial.println(str[startPos]);
  return (uint16_t)startPos;
}

//return endPos - position of last digit + 1
uint16_t parseJsonInteger(const String& str, int16_t startPos) {
  if (str[startPos] == '-') {
    jsonValue.boolValue = true;
    startPos++;
  } else jsonValue.boolValue = false;
  jsonValue.unsignedIntValue = 0;
  while (startPos < str.length() && isDigit(str[startPos])) {
    jsonValue.unsignedIntValue = jsonValue.unsignedIntValue * 10 + (str[startPos] - '0');
    startPos++;
  }
  return startPos;
}

uint16_t endJsonIntegerPos(const String& str, int16_t startPos) {
  if (str[startPos] == '-')
    startPos++;
  while (startPos < str.length() && isDigit(str[startPos]))
    startPos++;
  return startPos;
}

//return endPos - position of this array end position + 1
uint16_t endJsonArrayPos(const String& str, int16_t startPos) {
  bool insideString = false;
  bool escaped = false;
  if (str[startPos] != '[') return startPos;
  uint16_t counter = 1;
  startPos++;
  while (startPos < str.length()) {
    if (str[startPos] == '[' && !insideString) counter++;
    else if (str[startPos] == ']' && !insideString) counter--;
    else if (!escaped && str[startPos] == '\\') escaped = true;
    else if (!escaped && str[startPos] == '"') insideString = !insideString;
    else if (escaped) escaped = false;
    startPos++;
    if (counter == 0) return startPos;
  }
}

//return endPos - position of this node end position + 1
uint16_t endJsonNodePos(const String& str, int16_t startPos) {
  bool insideString = false;
  bool escaped = false;
  if (str[startPos] != '{') return startPos;
  uint16_t counter = 1;
  startPos++;
  while (startPos < str.length()) {
    if (str[startPos] == '{' && !insideString) counter++;
    else if (str[startPos] == '}' && !insideString) counter--;
    else if (!escaped && str[startPos] == '\\') escaped = true;
    else if (!escaped && str[startPos] == '"') insideString = !insideString;
    else if (escaped) escaped = false;
    startPos++;
    if (counter == 0) return startPos;
  }
}

//return new startPos from valuable symbol
uint16_t skipSymbolsJson(const String& str, int16_t startPos, char alsoSkip) {
  while (startPos < str.length() && (str[startPos] < '!' || str[startPos] == alsoSkip)) startPos++;
  return startPos;
}

//return new startPos after json value
uint16_t skipJsonValue(const String& str, int16_t startPos, char alsoSkip) {
  if (isNextJsonString(str, startPos)) {
    return (uint16_t)(endJsonStringPos(str, startPos) + 1);
  } else if (isNextJsonBool(str, startPos)) {
    if (str[startPos] == 't' && str.substring(startPos, startPos + 4) == "true") {
      return (uint16_t)(startPos + 5);
    } else if (str[startPos] == 'f' && str.substring(startPos, startPos + 5) == "false") {
      return (uint16_t)(startPos + 6);
    }
    //Serial.print("Can't parse bool str[");
    //Serial.print(startPos);
    //Serial.print("]=");
    //Serial.println(str[startPos]);
    return (uint16_t)startPos;
  } else if (isNextJsonInteger(str, startPos)) {
    if (str[startPos] == '-') startPos++;
    while (startPos < str.length() && isDigit(str[startPos])) startPos++;
    return startPos + 1;
  } else if (isNextJsonArray(str, startPos)) {
    return endJsonArrayPos(str, startPos);
  } else if (isNextJsonNode(str, startPos)) {
    return endJsonNodePos(str, startPos);
  }
  //Serial.print("Unknown field value: ");
  //Serial.println(str.substring(startPos));
  return startPos;
}
