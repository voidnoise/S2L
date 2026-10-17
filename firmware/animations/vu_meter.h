#pragma once
// ANIMATION 1: VU METER
//
// A bar of light that grows with the volume, like the level meter on a stereo.
// The colour of the whole bar changes with the sound too.
//
// How it works:
//   1. VOLUME: add up the 7 bands and divide by 7. A quiet sound gives a short bar
//      and a loud sound lights the whole strip.
//   2. COLOUR: every band adds a little to the hue, and higher bands add more than
//      lower ones (band 6 adds 7 times as much as band 0). So:
//        - silence gives red,
//        - more volume pushes the hue further round the colour wheel,
//        - treble moves it faster than bass, so the same volume of treble
//          and bass gives different colours.
//      If every band is at full volume the hue goes almost exactly once round the
//      wheel and ends up back at red.
//   3. SMOOTHING: the bar jumps up instantly but falls slowly, and the hue
//      glides to its new value. Without this the strip would flicker.

#include "config.h"

// ---- Settings you can play with ----
const uint8_t VU_HUE_PER_UNIT = 9;  // how strongly the bands push the hue. Bigger = more colour changes
const uint8_t VU_FALL_SPEED = 4;    // bigger = bar falls more slowly
const uint8_t VU_HUE_SPEED = 4;     // bigger = hue glides more slowly

// ---- What the animation remembers between frames ----
static uint8_t vuVolume = 0;  // the volume shown on the bar (0-255), smoothed
static uint16_t vuHue = 0;    // the colour shown on the bar, smoothed

void vuMeterReset() {
  vuVolume = 0;
  vuHue = 0;
}

// Draws one frame. `levels` holds the 7 band values (0-255).
void vuMeter(Adafruit_NeoPixel &strip, const uint8_t levels[]) {

  // 1. Work out the volume and the target hue from all 7 bands.
  uint16_t total = 0;       // all bands added together (at most 7 x 255)
  uint32_t targetHue = 0;
  for (uint8_t band = 0; band < BAND_COUNT; band++) {
    total += levels[band];
    targetHue += (uint32_t)levels[band] * (band + 1) * VU_HUE_PER_UNIT;
  }
  uint8_t volume = total / BAND_COUNT;

  // 2. Smooth the volume: rise at once, fall a bit at a time.
  if (volume >= vuVolume) {
    vuVolume = volume;
  } else {
    uint8_t gap = vuVolume - volume;
    vuVolume -= (gap + VU_FALL_SPEED - 1) / VU_FALL_SPEED;  // always falls by at least 1
  }

  // 3. Glide the hue towards its target. The hue is a circle (65535 is next to 0), so we
  //    store the gap as a signed 16-bit number. That way the hue always takes the short
  //    way round, even when it crosses the red point at 0.
  int16_t hueGap = (int16_t)((uint16_t)targetHue - vuHue);
  vuHue += hueGap / VU_HUE_SPEED;

  // 4. Draw the bar. The strip holds 12 LEDs, so the volume is stretched across them.
  //    `fullLeds` LEDs are fully on and the next one is dimmed to show the part of an LED left over.
  uint16_t barLength = (uint16_t)vuVolume * LED_COUNT;  // 0 to 255 x 12
  uint8_t fullLeds = barLength / 255;
  uint8_t partialBrightness = barLength % 255;

  for (uint16_t led = 0; led < LED_COUNT; led++) {
    uint8_t brightness = 0;
    if (led < fullLeds) brightness = 255;
    else if (led == fullLeds) brightness = partialBrightness;

    strip.setPixelColor(led, hueColor(vuHue, brightness));
  }
}
