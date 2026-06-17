#define LED_BUILTIN 2 //8 ESP32C3mini, 2 ESP-WROOM-32-DEV
#define LED_INVERTED false //ESP32C3mini=true, false ESP-WROOM-32-DEV=false

bool _ledState = false;

void ledBegin() {
  pinMode(LED_BUILTIN, OUTPUT);

}

void ledOn() {
 _ledState = true;
 digitalWrite(LED_BUILTIN, _ledState ^ LED_INVERTED);
}

void ledOff() {
  _ledState = false;
  digitalWrite(LED_BUILTIN, _ledState ^ LED_INVERTED);
}

void ledFlip() {
  _ledState = !_ledState;
  digitalWrite(LED_BUILTIN, _ledState ^ LED_INVERTED);
}

void simpleBlink(uint32_t duration = 100L) {
  ledOn();
  delay(duration);
  ledOff();
}

