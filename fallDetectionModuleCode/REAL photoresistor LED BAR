#include <Adafruit_NeoPixel.h>

const int LED_PIN = 11;
const int NUMPIXELS = 8;
const int LDR_PIN = A0;
const int DARK_THRESHOLD = 200;

Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
  pixels.clear();
  pixels.show();
  Serial.begin(9600);
}

void loop() {
  int lightLevel = analogRead(LDR_PIN);
  Serial.println(lightLevel);

  if (lightLevel < DARK_THRESHOLD) {
    // Dark: Turn all LEDs red
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // Red
    }
    pixels.show();
  } else {
    // Bright: Turn all LEDs off
    pixels.clear();
    pixels.show();
  }

  delay(100);
}
