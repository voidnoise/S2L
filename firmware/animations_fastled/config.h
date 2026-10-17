#pragma once
// Settings shared by every animation. Change them here and all the animations follow.
//
// FASTLED IN 60 SECONDS
//   This program uses a library called FastLED to control the LEDs. You only need
//   to know a few things:
//
//   leds[]            A list with one entry per LED. Setting  leds[3] = CRGB::Red;
//                     changes the colour of LED number 3 (counting from 0).
//   CRGB              A colour made from Red, Green and Blue amounts (0-255 each).
//                     CRGB(255, 0, 0) is red. CRGB::Black is "off".
//   CHSV              A colour made from Hue, Saturation and Value (brightness),
//                     0-255 each. Hue is a position on the colour wheel, which is
//                     easier to work with than red/green/blue amounts, because
//                     changing only the hue changes only the colour.
//   FastLED.show()    Sends the leds[] list to the real LEDs. Nothing lights up
//                     until this is called. (main.cpp calls it once per frame.)

#include <Arduino.h>
#include <FastLED.h>

const uint8_t LED_PIN = 6;        // Arduino pin wired to the strip's DIN
const uint16_t LED_COUNT = 12;    // number of WS2812 LEDs on the strip
const uint8_t BRIGHTNESS = 100;   // 0-255. Caps the current drawn from USB. Don't go much higher.

// The colour of every LED. The animations draw into this list.
CRGB leds[LED_COUNT];

// The audio is split into 7 frequency bands. Band 0 is the lowest (bass) and
// band 6 is the highest (treble). Each band arrives as a number from 0 to 255.
const uint8_t BAND_COUNT = 7;

// Every band has its own colour, so you can tell them apart.
// (HUE_RED, HUE_ORANGE and so on are names for positions on the colour wheel,
// and they come with FastLED.)
const uint8_t BAND_HUES[BAND_COUNT] = {
  HUE_RED,     // band 0: lowest notes, bass drum
  HUE_ORANGE,  // band 1
  HUE_YELLOW,  // band 2
  HUE_GREEN,   // band 3: middle, voices
  HUE_AQUA,    // band 4
  HUE_BLUE,    // band 5
  HUE_PURPLE   // band 6: highest notes, cymbals
};
