#pragma once
// Settings shared by every animation. Change them here and all the animations follow.

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const uint8_t LED_PIN = 6;        // Arduino pin wired to the strip's DIN
const uint16_t LED_COUNT = 12;    // number of WS2812 LEDs on the strip
const uint8_t BRIGHTNESS = 100;   // 0-255. Caps the current drawn from USB. Don't go much higher.

// The audio is split into 7 frequency bands. Band 0 is the lowest (bass) and
// band 6 is the highest (treble). Each band arrives as a number from 0 to 255.
const uint8_t BAND_COUNT = 7;

// Every band has its own colour, so you can tell them apart:
//
//   band:  0     1       2       3      4     5     6
//   colour red   orange  yellow  green  cyan  blue  violet
//
// Colours are picked with a "hue" number from 0 to 65535 that goes round a colour
// wheel (0 = red, 21845 = green, 43690 = blue, and 65535 is back to red).
const uint16_t HUE_PER_BAND = 8500;

// Returns the hue of a band (0 to 6).
inline uint16_t bandHue(uint8_t band) {
  return (uint16_t)band * HUE_PER_BAND;
}

// Makes a colour from a hue and a brightness (0-255). The gamma step makes dim colours
// look right to the eye. Without it, half brightness looks almost full.
inline uint32_t hueColor(uint16_t hue, uint8_t brightness) {
  return Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::ColorHSV(hue, 255, brightness));
}

// Same, but from red, green and blue amounts (0-255 each).
inline uint32_t rgbColor(uint8_t red, uint8_t green, uint8_t blue) {
  return Adafruit_NeoPixel::gamma32(Adafruit_NeoPixel::Color(red, green, blue));
}
