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
//   3. SMOOTHING: the bar jumps up instantly but falls slowly, and the colour
//      glides to its new value. Without this the strip would flicker.

#include "config.h"

// ---- Settings you can play with ----
const uint8_t VU_FALL = 230;          // 0-255. Each frame the bar keeps this much of its height. Bigger = falls more slowly
const uint8_t VU_COLOUR_SPEED = 128;  // 0-255. How far the colour moves towards its target each frame. Bigger = faster

// ---- What the animation remembers between frames ----
static uint8_t vuVolume = 0;          // the volume shown on the bar (0-255), smoothed
static CRGB vuColour = CRGB::Red;     // the colour shown on the bar, smoothed

void vuMeterReset() {
  vuVolume = 0;
  vuColour = CRGB::Red;
}

// Draws one frame into leds[]. `levels` holds the 7 band values (0-255).
void vuMeter(const uint8_t levels[]) {

  // 1. Work out the volume and the target hue from all 7 bands.
  uint16_t total = 0;           // all bands added together
  uint16_t weightedTotal = 0;   // same, but band 0 counts once, band 1 twice ... band 6 seven times
  for (uint8_t band = 0; band < BAND_COUNT; band++) {
    total += levels[band];
    weightedTotal += levels[band] * (band + 1);
  }
  uint8_t volume = total / BAND_COUNT;

  // The weights 1+2+3+4+5+6+7 add up to 28, so dividing by 28 gives a hue from
  // 0 (red) when it is quiet up to 255 when every band is at full volume.
  uint8_t targetHue = weightedTotal / 28;

  // 2. Smooth the volume. The bar is either the new volume, or 90% of what it was
  //    last frame, whichever is bigger. So it rises at once and falls a bit at a time.
  vuVolume = max(volume, scale8(vuVolume, VU_FALL));

  // 3. Move the bar's colour part of the way towards the target colour.
  nblend(vuColour, CHSV(targetHue, 255, 255), VU_COLOUR_SPEED);

  // 4. Draw the bar. The strip holds 12 LEDs, so the volume is stretched across them.
  //    `fullLeds` LEDs are fully on and the next one is dimmed to show the part of an LED left over.
  uint16_t barLength = vuVolume * LED_COUNT;   // 0 to 255 x 12
  uint8_t fullLeds = barLength / 255;
  uint8_t partialBrightness = barLength % 255;

  for (uint16_t led = 0; led < LED_COUNT; led++) {
    if (led < fullLeds) {
      leds[led] = vuColour;
    } else if (led == fullLeds) {
      leds[led] = vuColour;
      leds[led].nscale8(partialBrightness);   // dim this LED to partialBrightness / 255
    } else {
      leds[led] = CRGB::Black;
    }
  }
}
