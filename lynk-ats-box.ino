#include <Adafruit_NeoPixel.h>

#define LED_PIN 8
#define NUMPIXELS 1
#define LED_DELAY 500


Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
}

void loop() {
  pixels.clear();
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    pixels.show();
    delay(LED_DELAY);
    pixels.setPixelColor(i, pixels.Color(150, 0, 0));
    pixels.show();
    delay(LED_DELAY);
    pixels.setPixelColor(i, pixels.Color(0, 0, 1500));
    pixels.show();
    delay(LED_DELAY);
  }
}
