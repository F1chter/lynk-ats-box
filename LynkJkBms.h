#define READ_FROM_BMS_INTERVAL 1000
#define RESPONSE_BUFFER_SIZE 350
#define bmsSerial Serial2  //Serial0 esp32c3 Serial2 esp32
#define BMS_RX_PIN 16      //ESP RX -> BMS TX pin GREEN(near VCC on BMS)
#define BMS_TX_PIN 17      //ESP TX -> BMS RX pin YELLOW(near GND on BMS)

// see JK Communication protocol.pdf http://www.jk-bms.com/Upload/2022-05-19/1621104621.pdf
uint8_t bmsRequestFrame[21] = { 0x4E, 0x57 /*4E 57 = StartOfFrame*/, 0x00, 0x13 /*0x13 | 19 = LengthOfFrame*/, 0x00, 0x00,
                                0x00, 0x00 /*BMS ID, highest byte is default 00*/, 0x06 /*Function 1=Activate, 2=Write, 3=ReadIdentifier, 6=ReadAllData*/,
                                0x03 /*Frame source 0=BMS, 1=Bluetooth, 2=GPRS, 3=PC*/, 0x00 /*TransportType 0=Request, 1=Response, 2=BMSActiveUpload*/,
                                0x00 /*0=ReadAllData or commandToken*/, 0x00, 0x00, 0x00, 0x00 /*RecordNumber High byte is random code, low 3 bytes is record number*/, 0x68 /*0x68 = EndIdentifier*/,
                                0x00, 0x00, 0x01, 0x29 /*Checksum, high 2 bytes for checksum not yet enabled -> 0, low 2 Byte for checksum*/ };

//Size of reply is 291 bytes for 16 cells. sizeof(JKReplyStruct) is 221.
uint8_t bmsResponseFrameBuffer[RESPONSE_BUFFER_SIZE];  // The raw big endian data as received from JK BMS.

//bool _bmsReadFailed = false;
bool _bmsReadInProgress = false;
uint16_t _bmsResponseIndex = 0;
uint16_t _bmsExpectedLength = 0;
uint32_t lastReadFromBmsMillis = 0;


void bmsRead(uint32_t timeout_ms = 10000L);
void parseResponse();
void bmsPrintRawResponse();
String bmsSimpleStatus();

void bmsBegin() {
  //pinMode(BMS_TX_PIN, OUTPUT);
  //pinMode(BMS_RX_PIN, INPUT_PULLUP);
  bmsSerial.begin(115200, SERIAL_8N1, BMS_RX_PIN, BMS_TX_PIN);
  bmsRead();
  bmsPrintRawResponse();
  if (!_bmsReadInProgress && !boxFlags.bmsReadFailed) {
    parseResponse();
    Serial.println(bmsSimpleStatus());
  }
}

void tickBms() {
  if (!_bmsReadInProgress && millis() - lastReadFromBmsMillis < READ_FROM_BMS_INTERVAL) return;
  bmsRead();
  if (!_bmsReadInProgress) {
    lastReadFromBmsMillis = millis();
    if (!boxFlags.bmsReadFailed) parseResponse();
  }
}

void trackMainBatInfoChanged() {
  int16_t battCurrent = 0;  // *0.01 A
  uint8_t soc = 0;
  uint8_t temp = 0;
  uint16_t battV = 0;
}


void bmsRead(uint32_t timeout_ms) {
  if (!_bmsReadInProgress) {
    _bmsReadInProgress = true;
    lastReadFromBmsMillis = millis();
    _bmsResponseIndex = 0;
    //readFailed = false;
    while (bmsSerial.available() && millis() - lastReadFromBmsMillis < timeout_ms) Serial.write(bmsSerial.read());
    if (bmsSerial.available()) {
      Serial.println("Too many unexpected data from bms");
      boxFlags.bmsReadFailed = true;
      _bmsReadInProgress = false;
      //lastReadFromBmsMillis = millis();
      return;
    }
    bmsSerial.write(bmsRequestFrame, sizeof(bmsRequestFrame));
    _bmsExpectedLength = 0;
  }

  // Process incoming data
  while (bmsSerial.available() && millis() - lastReadFromBmsMillis < timeout_ms) {
    bmsResponseFrameBuffer[_bmsResponseIndex] = bmsSerial.read();

    if (_bmsResponseIndex == 1) {
      if (bmsResponseFrameBuffer[0] != 0x4E
          || bmsResponseFrameBuffer[1] != 0x57) {
        boxFlags.bmsReadFailed = true;
        _bmsReadInProgress = false;
        return;
      }
    } else if (_bmsResponseIndex == 3) {
      _bmsExpectedLength = (bmsResponseFrameBuffer[2] << 8) | bmsResponseFrameBuffer[3];
      //TODO validate nonzero
    } else if (_bmsExpectedLength != 0 && _bmsResponseIndex == _bmsExpectedLength - 3) {
      if (bmsResponseFrameBuffer[_bmsResponseIndex] != 0x68) {
        boxFlags.bmsReadFailed = true;
        _bmsReadInProgress = false;
        return;
      }
    } else if (_bmsExpectedLength != 0 && _bmsResponseIndex == _bmsExpectedLength + 1) {
      //TODO crc check;
      boxFlags.bmsReadFailed = false;
      _bmsReadInProgress = false;
      lastReadFromBmsMillis = millis();
      return;
    }
    _bmsResponseIndex++;
    // Prevent buffer overflow
    if (_bmsResponseIndex >= RESPONSE_BUFFER_SIZE) {
      Serial.println("Buffer overflow");
      boxFlags.bmsReadFailed = true;
      _bmsReadInProgress = false;
      return;
    }
  }

  if (_bmsReadInProgress && millis() - lastReadFromBmsMillis > timeout_ms) {
    Serial.println("BMS Timeout");
    boxFlags.bmsReadFailed = true;
    _bmsReadInProgress = false;
  } else delay(1);
}

int8_t _bmsConvertTemperature(uint8_t highByte, uint8_t lowByte) {
  uint16_t temp = (highByte << 8) | lowByte;
  if (temp > 100) {
    return -((int8_t)constrain((temp - 100), 1, 128));
  }
  return constrain(temp, 0, 127);
}

#define CURRENT_SPIKE_CANDIDATE_DELTA 500  //5A
int16_t _previousCurrent = 0;
bool _isCurrentSpike(int16_t curr) {
  uint16_t delta = 0;
  if (curr > bmsData.current) delta = curr - bmsData.current;
  else delta = bmsData.current - curr;
  if (delta < CURRENT_SPIKE_CANDIDATE_DELTA) {
    _previousCurrent = curr;
    return false;
  }

  if (curr > _previousCurrent) delta = curr - _previousCurrent;
  else delta = _previousCurrent - curr;

  if (delta < CURRENT_SPIKE_CANDIDATE_DELTA) {
    _previousCurrent = curr;
    return false;
  }

  _previousCurrent = curr;
  return true;
}



int16_t _bmsConvertCurrent(uint8_t version, uint16_t rawCurrent) {
  if (version == 0) {
    if (rawCurrent > 10000) {
      return -constrain((rawCurrent - 10000), 1, 32768);  // Discharge (negative)
    } else if (rawCurrent < 10000 && rawCurrent > 0) {
      return constrain((10000 - rawCurrent), 0, 32767);  // Charge(positive)
    } else {
      return 0;
    }
  } else {
    if ((rawCurrent & 0x8000) == 0) {
      return -constrain(rawCurrent, 1, 32768);  // Discharge (negative)
    } else {
      return constrain((rawCurrent & 0x07FF), 0, 32767);  // Charge(positive)
    }
  }
}

void _parseBatteryWarningMessage(uint8_t highByte, uint8_t lowByte) {
  if (bitRead(lowByte, 0)) Serial.println("WARNING: Low capacity alarm");
  if (bitRead(lowByte, 1)) Serial.println("WARNING: MOS overtemperature alarm");
  if (bitRead(lowByte, 2)) Serial.println("WARNING: Charge overvoltage alarm");
  if (bitRead(lowByte, 3)) Serial.println("WARNING: Discharge overvoltage alarm");
  if (bitRead(lowByte, 4)) Serial.println("WARNING: Temp1 overtemperature alarm");
  if (bitRead(lowByte, 5)) Serial.println("WARNING: Charge overcurrent alarm");
  if (bitRead(lowByte, 6)) Serial.println("WARNING: Discharge overcurrent alarm");
  if (bitRead(lowByte, 7)) Serial.println("WARNING: Differential pressure alarm");
  if (bitRead(highByte, 0)) Serial.println("WARNING: Temp2 overtemperature alarm");
  if (bitRead(highByte, 1)) Serial.println("WARNING: Battery low temperature alarm");
  if (bitRead(highByte, 2)) Serial.println("WARNING: Unit overvoltage alarm");
  if (bitRead(highByte, 3)) Serial.println("WARNING: Unitundervoltage alarm");
  if (bitRead(highByte, 4)) Serial.println("WARNING: 309_A protection");
  if (bitRead(highByte, 5)) Serial.println("WARNING: 309_B protection");
  if (bitRead(highByte, 6)) Serial.println("WARNING: Unknown1(reserved) alarm");
  if (bitRead(highByte, 7)) Serial.println("WARNING: Unknown2(reserved) alarm");
}

void _parseBatteryStatusInfo(uint8_t highByte, uint8_t lowByte) {
  if (bitRead(lowByte, 0)) Serial.println("Charging MOS ON");
  else Serial.println("Charging MOS OFF");
  if (bitRead(lowByte, 1)) Serial.println("Discharge MOS ON");
  else Serial.println("Discharge MOS OFF");
  if (bitRead(lowByte, 2)) Serial.println("Balance ON");
  else Serial.println("Balance OFF");
  if (bitRead(lowByte, 3)) Serial.println("Battery is down normal");
  else Serial.println("Battery is down dropped");
  //4-15 reserved
}

void parseResponse() {
  if (boxFlags.bmsReadFailed) return;
  uint8_t version = 0;
  uint16_t rawCurrent = 0;
  for (uint16_t i = 11; i < _bmsResponseIndex - 9;) {
    uint8_t id = bmsResponseFrameBuffer[i];

    if (id == 0x79) {  // Cells voltage
      uint8_t cellsInfoBytes = bmsResponseFrameBuffer[i + 1];
      for (uint16_t j = 0; j < cellsInfoBytes; j += 3) {
        uint8_t cellNo = constrain(bmsResponseFrameBuffer[i + 2 + j], 1, BMS_MAX_CELLS);
        uint16_t voltage = (bmsResponseFrameBuffer[i + 3 + j] << 8) | bmsResponseFrameBuffer[i + 4 + j];  //Big endian
        if (bmsData.cellVoltages[cellNo - 1] != voltage) {
          boxFlags.battInfoScreenNeedToRedraw = true;
          bmsData.cellVoltages[cellNo - 1] = voltage;  //*0.001f V
        }
        //Serial.print("C");
        //Serial.print(cellNo);
        //Serial.print("=");
        //Serial.print(bmsData.cellVoltages[cellNo - 1]);
        //Serial.print("*0.001V ");
      }
      //Serial.println("");
      i += cellsInfoBytes + 2;
    } else if (id == 0x80) {                                                                            //Read MOS temperature
      int8_t t = _bmsConvertTemperature(bmsResponseFrameBuffer[i + 1], bmsResponseFrameBuffer[i + 2]);  //Big endian
      if (bmsData.mosTemp != t) {
        bmsData.mosTemp = t;
        boxFlags.battInfoScreenNeedToRedraw = true;
        if (config.showTemperature) boxFlags.battInfoUpdated = true;
      }
      i += 3;
    } else if (id == 0x81) {                                                                            //Read Temp1
      int8_t t = _bmsConvertTemperature(bmsResponseFrameBuffer[i + 1], bmsResponseFrameBuffer[i + 2]);  //Big endian
      if (bmsData.temp1 != t) {
        bmsData.temp1 = t;
        boxFlags.battInfoScreenNeedToRedraw = true;
        if (config.showTemperature) boxFlags.battInfoUpdated = true;
      }
      i += 3;
    } else if (id == 0x82) {                                                                            //Read Temp2
      int8_t t = _bmsConvertTemperature(bmsResponseFrameBuffer[i + 1], bmsResponseFrameBuffer[i + 2]);  //Big endian
      if (bmsData.temp2 != t) {
        bmsData.temp2 = t;
        boxFlags.battInfoScreenNeedToRedraw = true;
        if (config.showTemperature) boxFlags.battInfoUpdated = true;
      }
      i += 3;
    } else if (id == 0x83) {                                                              //Total battery voltage
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      if (bmsData.totalVoltage != v) {
        bmsData.totalVoltage = v;
        //Serial.print("Total Voltage"); Serial.print(v); Serial.println("*0.01V");
        boxFlags.battInfoScreenNeedToRedraw = true;
        boxFlags.battInfoUpdated = true;
      }
      //bmsData.totalVoltage = v / 100.0f;
      i += 3;
    } else if (id == 0x84) {                                                              //The current data
      rawCurrent = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      i += 3;
    } else if (id == 0x85) {  //SOC
      if (bmsData.soc != bmsResponseFrameBuffer[i + 1]) {
        bmsData.soc = bmsResponseFrameBuffer[i + 1];
        boxFlags.battInfoScreenNeedToRedraw = true;
        boxFlags.battInfoUpdated = true;
      }
      i += 2;
    } else if (id == 0x86) {  //Temperature sensor count - skip, always 2
      i += 2;
    } else if (id == 0x87) {                                                              //Cycle times of battery use
      uint16_t c = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      if (bmsData.cycles != c) {
        bmsData.cycles = c;
        boxFlags.battInfoScreenNeedToRedraw = true;
      }
      i += 3;
    } else if (id == 0x89) {  //Total capacity of battery cycle - TBD
      i += 5;
    } else if (id == 0x8a) {  //Total number of battery strings
      //bmsData.numCells = bmsResponseFrameBuffer[i + 2];
      i += 3;
    } else if (id == 0x8b) {                                                              //Battery Warning Message
      uint16_t s = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      if (bmsData.alarmStatus != s) {
        bmsData.alarmStatus = s;
        boxFlags.battInfoScreenNeedToRedraw = true;
      }
      //_parseBatteryWarningMessage(bmsResponseFrameBuffer[i + 1], bmsResponseFrameBuffer[i + 2]);
      i += 3;
    } else if (id == 0x8c) {  //Battery status information
      if (bmsData.statusInfo != bmsResponseFrameBuffer[i + 2]) {
        bmsData.statusInfo = bmsResponseFrameBuffer[i + 2];
        boxFlags.battInfoScreenNeedToRedraw = true;
      }
      //_parseBatteryStatusInfo(bmsResponseFrameBuffer[i + 1], bmsResponseFrameBuffer[i + 2]);
      i += 3;
    } else if (id == 0x8e) {                                                              //Total voltage overvoltage protection
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Total voltage overvoltage protection: ");
      //Serial.println(v / 100.0f);
      i += 3;
    } else if (id == 0x8f) {                                                              //Total voltage undervoltage protection
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Total voltage undervoltage protection: ");
      //Serial.println(v / 100.0f);
      i += 3;
    } else if (id == 0x90) {                                                              //Cell overvoltage protection voltage
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Cell overvoltage protection voltage: ");
      //Serial.println(v / 1000.0f);
      i += 3;
    } else if (id == 0x91) {                                                              //Cell overvoltage recovery voltage
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Cell overvoltage recovery voltage: ");
      //Serial.println(v / 1000.0f);
      i += 3;
    } else if (id == 0x92) {                                                              //Cell overvoltage protection delay
      uint16_t s = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Cell overvoltage protection delay: ");
      //Serial.println(s);
      i += 3;
    } else if (id == 0x93) {                                                              //Unit undervoltage protection voltage
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Unit undervoltage protection voltage: ");
      //Serial.println(v / 1000.0f);
      i += 3;
    } else if (id == 0x94) {                                                              //Cell undervoltage recovery voltage
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Cell undervoltage recovery voltage: ");
      //Serial.println(v / 1000.0f);
      i += 3;
    } else if (id == 0x95) {                                                              //Cell undervoltage protection delay
      uint16_t s = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Cell undervoltage protection delay: ");
      //Serial.println(s);
      i += 3;
    } else if (id == 0x96) {                                                              //Core differential pressure protection value
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Core differential pressure protection value: ");
      //Serial.println(v / 1000.0f);
      i += 3;
    } else if (id == 0x97) {                                                              //Discharge overcurrent protection value
      uint16_t c = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Discharge overcurrent protection value: ");
      //Serial.println(c);
      i += 3;
    } else if (id == 0x98) {                                                              //Overcurrent delay of discharge
      uint16_t s = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Overcurrent delay of discharge: ");
      //Serial.println(s);
      i += 3;
    } else if (id == 0x99) {                                                              //Charge overcurrent protection value
      uint16_t c = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Charge overcurrent protection value: ");
      //Serial.println(c);
      i += 3;
    } else if (id == 0x9a) {                                                              //Charge overcurrent delay
      uint16_t s = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Charge overcurrent delay: ");
      //Serial.println(s);
      i += 3;
    } else if (id == 0x9b) {                                                              //Balance starting voltage
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Balance starting voltage: ");
      //Serial.println(v / 1000.0f);
      i += 3;
    } else if (id == 0x9c) {                                                              //Balance the opening differential pressure
      uint16_t v = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Balance the opening differential pressure: ");
      //Serial.println(v / 1000.0f);
      i += 3;
    } else if (id == 0x9d) {  //Active balance switch
      //Serial.print("Active balance switch: ");
      //Serial.println(bmsResponseFrameBuffer[i + 1] ? "ON" : "OFF");
      i += 2;
    } else if (id == 0x9e) {  //MOS temperature protection value
      //Serial.print("MOS temperature protection value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0x9f) {  //MOS temperature recovery value
      //Serial.print("MOS temperature recovery value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa0) {  //T1 temperature protection value
      //Serial.print("T1 temperature protection value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa1) {  //T1 temperature recovery value
      //Serial.print("T1 temperature recovery value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa2) {  //Temperature difference protection value of battery
      //Serial.print("Temperature difference protection value of battery: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa3) {  //Battery charging high temperature protection value
      //Serial.print("Battery charging high temperature protection value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa4) {  //Battery discharge high temperature protection value
      //Serial.print("Battery discharge high temperature protection value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa5) {  //Battery charging low temperature protection value
      //Serial.print("Battery charging low temperature protection value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa6) {  //Battery charging low temperature protection recovery value
      //Serial.print("Battery charging low temperature protection recovery value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa7) {  //Battery discharge low temperature protection value
      //Serial.print("Battery discharge low temperature protection value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa8) {  //Battery discharge low temperature protection recovery value
      //Serial.print("Battery discharge low temperature protection recovery value: ");
      //Serial.println((int16_t)(bmsResponseFrameBuffer[i + 1]<<8 | bmsResponseFrameBuffer[i + 2]));
      i += 3;
    } else if (id == 0xa9) {  //Battery string count
      bmsData.numCells = constrain(bmsResponseFrameBuffer[i + 1], 0, BMS_MAX_CELLS);
      //Serial.println(String("Battery string count:") + bmsData.numCells);
      i += 2;
    } else if (id == 0xaa) {  //Battery capacity setting
      i += 5;
    } else if (id == 0xab) {  //Charge MOS switch
      //Serial.print("Charge MOS switch: ");
      //Serial.println(bmsResponseFrameBuffer[i + 1] ? "ON" : "OFF");
      i += 2;
    } else if (id == 0xac) {  //Discharge MOS switch
      //Serial.print("Discharge MOS switch: ");
      //Serial.println(bmsResponseFrameBuffer[i + 1] ? "ON" : "OFF");
      i += 2;
    } else if (id == 0xad) {  //Current calibration
      i += 3;
    } else if (id == 0xae) {  //Guard plate address
      i += 2;
    } else if (id == 0xaf) {  //The battery type
      i += 2;
    } else if (id == 0xb0) {                                                              //Dormancy waiting time
      uint16_t s = (bmsResponseFrameBuffer[i + 1] << 8) | bmsResponseFrameBuffer[i + 2];  //Big endian
      //Serial.print("Dormancy waiting time: ");
      //Serial.println(s);
      i += 3;
    } else if (id == 0xb1) {  //Low capacity alarm value
      //Serial.print("Low capacity alarm value: ");
      //Serial.println(bmsResponseFrameBuffer[i + 1]);
      i += 2;
    } else if (id == 0xb2) {  //Change parameter password
      i += 11;
    } else if (id == 0xb3) {  //Special charger switch
      //Serial.print("Special charger switch: ");
      //Serial.println(bmsResponseFrameBuffer[i + 1] ? "ON" : "OFF");
      i += 2;
    } else if (id == 0xb4) {  //The device ID code
      i += 9;
    } else if (id == 0xb5) {  //Manufacture date
      i += 5;
    } else if (id == 0xb6) {  //System working time
      i += 5;
    } else if (id == 0xb7) {  //Software version number
      i += 16;
    } else if (id == 0xb8) {  //Whether to start current calibration
      i += 2;
    } else if (id == 0xb9) {  //Actual battery capacity
      i += 5;
    } else if (id == 0xba) {  //Name of Manufacturer ID
      i += 25;
    } else if (id == 0xbb) {  //Restart the system
      i += 2;
    } else if (id == 0xbc) {  //Factory data reset
      i += 2;
    } else if (id == 0xbd) {  //Remote Upgrade Identification
      i += 2;
    } else if (id == 0xbe) {  //The cell turns off GPS with low voltage
      i += 3;
    } else if (id == 0xbf) {  //Cell low voltage recovery GPS
      i += 3;
    } else if (id == 0xc0) {  //Agreement version number
      version = bmsResponseFrameBuffer[i + 1];
      i += 2;
    } else if (id == 0xce) {  //unknown
      i += 2;
    } else if (id == 0xd0) {  //unknown
      i += 2;
    } else {
      Serial.print("=======UNKNOWN IDENTIFIER: ");
      for (uint16_t j = i; j < _bmsResponseIndex; j++) {
        if (bmsResponseFrameBuffer[j] < 16) Serial.print("0");
        Serial.print(bmsResponseFrameBuffer[j], HEX);
        Serial.print(" ");
      }
      Serial.println();
      break;
    }
  }
  int16_t curr = _bmsConvertCurrent(version, rawCurrent);
  if (bmsData.current != curr && !_isCurrentSpike(curr)) {
    bmsData.current = curr;
    boxFlags.battInfoScreenNeedToRedraw = true;
    boxFlags.battInfoUpdated = true;
  }
  bmsData.minVoltageCellIndex = 0;
  bmsData.maxVoltageCellIndex = 0;
  for (uint8_t i = 1; i < bmsData.numCells; i++) {
    if (bmsData.cellVoltages[i] < bmsData.cellVoltages[bmsData.minVoltageCellIndex]) bmsData.minVoltageCellIndex = i;
    if (bmsData.cellVoltages[i] > bmsData.cellVoltages[bmsData.maxVoltageCellIndex]) bmsData.maxVoltageCellIndex = i;
  }
  bmsData.version++;
  //Serial.println("PARSE ENDED");
}

void bmsPrintRawResponse() {
  if (boxFlags.bmsReadFailed && _bmsResponseIndex == 0) {
    Serial.println("Read from BMS Timeout");
    return;
  }
  if (boxFlags.bmsReadFailed || _bmsReadInProgress) Serial.print("Failed or Partial ");
  Serial.print("Response from BMS[");
  Serial.print(_bmsResponseIndex);
  Serial.print("]: ");
  for (uint16_t i = 0; i < _bmsResponseIndex; i++) {
    if (bmsResponseFrameBuffer[i] < 16) Serial.print("0");
    Serial.print(bmsResponseFrameBuffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void appendCellInfo(String &result, uint8_t i) {
  result += (i + 1);
  result += ": ";
  result += bmsData.cellVoltages[i] / 1000;
  result += ".";
  if (bmsData.cellVoltages[i] % 1000 < 10) result += "00";
  else if (bmsData.cellVoltages[i] % 1000 < 100) result += "0";
  result += bmsData.cellVoltages[i] % 1000;
}

String bmsSimpleStatus() {
  String result;
  //result.reserve();
  result += "Status: ";                                      //8
  result += boxFlags.bmsReadFailed ? "Fail\n\r" : "OK\n\r";  //4
  if (boxFlags.bmsReadFailed) return result;

  result += bmsData.soc;
  result += "% ";  //5
  result += constrain(bmsData.totalVoltage / 100, 0, 99);
  result += ".";
  if ((bmsData.totalVoltage % 100) < 10) result += "0";
  result += bmsData.totalVoltage % 100;
  result += "v ";  //7
  result += "\n\r";

  uint8_t rows = (bmsData.numCells + 1) / 2;
  Serial.print("rows");
  Serial.println(rows);
  for (uint8_t i = 0; i < rows; i++) {
    appendCellInfo(result, i);
    result += " ";
    if ((i + rows) < bmsData.numCells) appendCellInfo(result, i + rows);
    result += "\n\r";
  }

  result += "Max-Min:";
  uint16_t diff = bmsData.cellVoltages[bmsData.maxVoltageCellIndex] - bmsData.cellVoltages[bmsData.minVoltageCellIndex];
  result += diff / 1000;
  result += ".";
  if (diff % 1000 < 10) result += "00";
  else if (diff % 1000 < 100) result += "0";
  result += diff % 1000;
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
  Serial.print("=========== result.length() ==");
  Serial.println(result.length());
  return result;
}


/*

Response example:
4E 57 - header
01 07 - length
00 00 00 00 - BMS ID
06 - command
00 - frame source
01 - transport type
79 - Identifier(Singlebatteryvoltage)
12 - Length(18 bytes)
01 08 93 
02 08 84 
03 08 93 
04 08 93 
05 08 84 
06 08 93 
80 - Identifier(Readpowertube temperature)
00 1F
81 00 1A 
82 00 1A 
83 05 21 
84 00 00 
85 2C 
86 02
87 00 00 
89 00 00 00 00 
8A 00 06 
8B 00 00 
8C 00 07 
8E 06 54 
8F 04 38 
90 0A 8C 
91 0A 50 
92 00 03 
93 07 08 
94 07 3A 
95 00 05 
96 01 2C 
97 00 64 
98 01 2C 
99 00 64 
9A 00 3C 
9B 07 D0 
9C 00 0A 
9D 01 
9E 00 50 
9F 00 46 
A0 00 46 
A1 00 3C 
A2 00 14 
A3 00 46 
A4 00 46 
A5 FF F6 
A6 00 00 
A7 FF DD 
A8 FF E2 
A9 06 
AA 00 00 00 2D 
AB 01 
AC 01 
AD 00 01 
AE 01 
AF 02 
B0 00 0A 
B1 14 
B2 31 32 33 34 35 36 00 00 00 00 
B3 00 
B4 4A 4B 2D 42 4D 53 00 00 
B5 32 36 30 35 
B6 00 00 02 20 
B7 32 31 48 5F 5F 5F 5F 5F 53 32 31 2E 30 30 5F 
B8 00 
B9 00 00 00 2D 
BA 4A 4B 2D 42 4D 53 00 00 00 00 00 00 36 30 31 32 36 33 33 34 33 30 31 35 
C0 01 
CE 00 
D0 00 
00 00 00 00 - record number
68 - end of identity
00 00 43 70 - crc
*/