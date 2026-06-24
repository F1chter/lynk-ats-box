//https://github.com/mathieucarbou/MycilaJSY/blob/main/src/MycilaJSY.cpp
#define jsySerial Serial1
#define JSY_RX_PIN 25           //ESP RX -> JSY TX pin YELLOW(last on JSY)
#define JSY_TX_PIN 26           //ESP TX -> JSY RX pin GREEN(near VCC on JSY)
#define JSY_READ_INTERVAL 1000  //1s
#define JSY_READ_TIMEOUT 5000   //5s
#define JSY_REGISTERS_COUNT 10
#define JSY_RESPONSE_SIZE JSY_REGISTERS_COUNT * 2 + 5

//bool _jsyReadFailed = false;
bool _jsyReadInProgress = false;
uint32_t _jsyStartMillis = 0;
uint8_t _jsyResponseIndex = 0;
uint16_t _jsyExpectedLength = 0;

uint8_t jsyRequestFrame[8] = { 0x01, 0x03, 0x00, 0x48,                   //id, readRegisters, firstRegisterAddress
                               0x00, JSY_REGISTERS_COUNT, 0x45, 0xDB };  //10registers, CRC16
uint8_t jsyResponseFrameBuffer[JSY_RESPONSE_SIZE];

void _readJsy();
void _parseJsyResponse();

void jsyBegin() {
  //pinMode(BMS_TX_PIN, OUTPUT);
  pinMode(JSY_RX_PIN, INPUT_PULLUP); 
  jsySerial.begin(9600, SERIAL_8N1, JSY_RX_PIN, JSY_TX_PIN);
  uint8_t request[] = { 0x01, 0x03, 0x00, 0x48, 0x00, 0x0A, 0x45, 0xDB };
  //for (int i = 0; i < 8; i++) {
  //  jsySerial.write(request[i]);
  //}
  _readJsy();
}

void tickJsy(uint32_t timeout_ms = 1000L) {
  if (!_jsyReadInProgress && millis() - _jsyStartMillis < JSY_READ_INTERVAL) return;
  _readJsy();
  if (!_jsyReadInProgress) {
    _jsyStartMillis = millis();
    if (!boxFlags.jsyReadFailed) _parseJsyResponse();
  }
}

void _readJsy() {

  uint8_t request[] = { 0x01, 0x03, 0x00, 0x48, 0x00, 0x0A, 0x45, 0xDB };  // 1-slave id, 3-function, 0048 - first register address, 000A - 10 registers, 45DB - CRC16
  //expected response  01 03 14 5F C0 01 8F 03 D5 00 00 07 C0 03 E8 00 00 01 E6 6E 5E 13 86 F9 F1 - 01-  slave id, 03-function, 14 - 20byte, ... , F9F1 - CRC16

  if (!_jsyReadInProgress) {
    Serial.println("JSY start _readJsy");
    _jsyReadInProgress = true;
    _jsyStartMillis = millis();
    _jsyResponseIndex = 0;
    //read previous response
    while (jsySerial.available() && millis() - _jsyStartMillis < JSY_READ_TIMEOUT) Serial.print(jsySerial.read(), HEX);
    if (jsySerial.available()) {
      Serial.println("Too many unexpected data from JSY");
      boxFlags.jsyReadFailed = true;
      _jsyReadInProgress = false;
      _jsyStartMillis = millis();
      return;
    }
    Serial.print("Send to JSY");
    jsySerial.write(request, sizeof(request));
    _jsyExpectedLength = 0;
  }

  // Process incoming data
  while (jsySerial.available()) {
    jsyResponseFrameBuffer[_jsyResponseIndex] = jsySerial.read();

    if (_jsyResponseIndex == 0) {
      // 01 - slave id
      if (jsyResponseFrameBuffer[_jsyResponseIndex] != 0x01) {
        boxFlags.jsyReadFailed = true;
        _jsyReadInProgress = false;
        return;
      }
      Serial.print("<-id ");
    } else if (_jsyResponseIndex == 1) {
      if (jsyResponseFrameBuffer[_jsyResponseIndex] != 0x03) {
        boxFlags.jsyReadFailed = true;
        _jsyReadInProgress = false;
        return;
      }
      //03 - function
      Serial.print("<-function ");
    } else if (_jsyResponseIndex == 2) {
      Serial.print("<-length ");
      if (jsyResponseFrameBuffer[_jsyResponseIndex] != JSY_REGISTERS_COUNT * 2) {
        Serial.print(jsyResponseFrameBuffer[_jsyResponseIndex], HEX);
        Serial.print(JSY_REGISTERS_COUNT * 2, HEX);
        boxFlags.jsyReadFailed = true;
        _jsyReadInProgress = false;
        return;
      }
      _jsyExpectedLength = JSY_RESPONSE_SIZE;
    } else if (_jsyExpectedLength != 0 && _jsyResponseIndex == _jsyExpectedLength - 1) {
      //TODO crc check;
      Serial.print("<-CRC");
      boxFlags.jsyReadFailed = false;
      _jsyReadInProgress = false;
      Serial.println(" JSY END");
      return;
    }
    _jsyResponseIndex++;
    // Prevent buffer overflow
    if (_jsyResponseIndex >= JSY_RESPONSE_SIZE) {
      Serial.println("Buffer overflow");
      boxFlags.jsyReadFailed = true;
      _jsyReadInProgress = false;
      return;
    }
  }
  if (_jsyReadInProgress && millis() - _jsyStartMillis > JSY_READ_TIMEOUT) {
    boxFlags.jsyReadFailed = true;
    _jsyReadInProgress = false;
    Serial.println("JSY Timeout");
  } else delay(1);
}

void _parseJsyResponse() {
  uint16_t word = (jsyResponseFrameBuffer[3] << 8) | jsyResponseFrameBuffer[4];
  Serial.println("_parseJsyResponse");
  if (jsyData.voltage != word) {
    jsyData.voltage = word;
    boxFlags.outputInfoUpdated = true;
    boxFlags.outputInfoScreenNeedToRedraw = true;
  }
  Serial.print("Voltage: ");
  Serial.print(jsyData.voltage);

  word = (jsyResponseFrameBuffer[5] << 8) | jsyResponseFrameBuffer[6];
  if (jsyData.current != word) {
    jsyData.current = word;
    //boxFlags.outputInfoUpdated = true;
    boxFlags.outputInfoScreenNeedToRedraw = true;
  }
  Serial.print("V Current: ");
  Serial.print(jsyData.current);

  word = (jsyResponseFrameBuffer[7] << 8) | jsyResponseFrameBuffer[8];
  if (jsyData.power != word) {
    jsyData.power = word;
    boxFlags.outputInfoUpdated = true;
    boxFlags.outputInfoScreenNeedToRedraw = true;
  }
  Serial.print("A Power: ");
  Serial.print(jsyData.power);

  uint32_t dword = (jsyResponseFrameBuffer[9] << 24) | (jsyResponseFrameBuffer[10] << 16) | (jsyResponseFrameBuffer[11] << 8) | jsyResponseFrameBuffer[12];
  if (jsyData.energy != dword) {
    jsyData.energy = dword;
    //boxFlags.outputInfoUpdated = true;
    boxFlags.outputInfoScreenNeedToRedraw = true;
  }
  Serial.print("W Energy: ");
  Serial.print(jsyData.energy);

  word = (jsyResponseFrameBuffer[13] << 8) | jsyResponseFrameBuffer[14];
  if (jsyData.pf != word) {
    jsyData.pf = word;
    //boxFlags.outputInfoUpdated = true;
    boxFlags.outputInfoScreenNeedToRedraw = true;
  }
  Serial.print("Wh PowerFactor: ");
  Serial.print(jsyData.pf);

  jsyData.co2 = (jsyResponseFrameBuffer[15] << 24) | (jsyResponseFrameBuffer[16] << 16) | (jsyResponseFrameBuffer[17] << 8) | jsyResponseFrameBuffer[18];
  Serial.print(" CO2: ");
  Serial.print(jsyData.co2);

  jsyData.temp = (jsyResponseFrameBuffer[19] << 8) | jsyResponseFrameBuffer[20];
  Serial.print("kg Temp: ");
  Serial.print(jsyData.temp);

  word = (jsyResponseFrameBuffer[21] << 8) | jsyResponseFrameBuffer[22];
  if (jsyData.freq != word) {
    jsyData.freq = word;
    //boxFlags.outputInfoUpdated = true;
    boxFlags.outputInfoScreenNeedToRedraw = true;
  }
  Serial.print(" Frequency: ");
  Serial.print(jsyData.freq);

  Serial.println("Hz");

  jsyData.version++;
}
