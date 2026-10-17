#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Simple test for a strip/ring of 12 WS2812 LEDs.
//
// Wiring: Arduino pin 6 -> (330 ohm resistor) -> DIN, 5V -> 5V, GND -> GND.
// Ideally add a ~470 uF capacitor across 5V/GND at the LEDs. Keep the Arduino
// and the LEDs on a common ground.
//
// Each pass runs three checks, then repeats:
//   1. whole strip red, green, blue, white  (wrong colours? change NEO_GRB below)
//   2. one white pixel chasing round        (skipped/dead pixels, wrong order)
//   3. rainbow                              (smooth colour across all pixels)

const uint8_t LED_PIN = 6;
const uint16_t LED_COUNT = 12;
// Full white draws ~60 mA per LED (720 mA for 12), more than a USB port should
// supply, so keep the brightness low. 0-255.
const uint8_t BRIGHTNESS = 40;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void fill(uint32_t color, uint16_t holdMs) {
  strip.fill(color);
  strip.show();
  delay(holdMs);
}

void colorCheck() {
  Serial.println(F("colours: red, green, blue, white"));
  fill(strip.Color(255, 0, 0), 800);
  fill(strip.Color(0, 255, 0), 800);
  fill(strip.Color(0, 0, 255), 800);
  fill(strip.Color(255, 255, 255), 800);
  fill(0, 300);
}

void chase() {
  Serial.println(F("chase"));
  for (uint8_t lap = 0; lap < 2; lap++) {
    for (uint16_t i = 0; i < LED_COUNT; i++) {
      strip.clear();
      strip.setPixelColor(i, strip.Color(255, 255, 255));
      strip.show();
      delay(80);
    }
  }
  fill(0, 300);
}

void rainbow() {
  Serial.println(F("rainbow"));
  // hue is 0-65535 around the colour wheel; spread it evenly across the pixels
  for (uint16_t offset = 0; offset < 3 * 65536UL; offset += 512) {
    for (uint16_t i = 0; i < LED_COUNT; i++) {
      uint16_t hue = offset + (uint32_t)i * 65536UL / LED_COUNT;
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(hue)));
    }
    strip.show();
    delay(15);
  }
  fill(0, 300);
}

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();  // start with everything off
  Serial.println(F("S2L WS2812 test: 12 LEDs on pin 6"));
}

void loop() {
  colorCheck();
  chase();
  rainbow();
}
