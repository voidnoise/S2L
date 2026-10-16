#pragma once
// ANIMATION 2: SHOOTING LIGHTS
//
// When the music gets loud in any band, a light shoots along the strip from the
// first LED to the last.
//
// How it works:
//   1. THRESHOLD: if any band is louder than SHOT_THRESHOLD, we want to fire a light.
//   2. COOLDOWN: after firing, we can't fire again until SHOT_COOLDOWN_MS has
//      passed. This stops a long loud sound from firing a light every frame.
//   3. COLOUR: the light's colour is the mix of every band that was over the
//      threshold at that moment. Each band has its own colour (see config.h) and we
//      add them together, so bass (red) plus treble (violet) gives purple, and
//      bass plus mid (red + green) gives yellow.
//   4. MOVEMENT: each light moves one LED along every SHOT_STEP_MS. It leaves a
//      fading trail behind it. When it has gone off the end, it is removed.
//
// Several lights can be on the strip at once. Each one is stored in the `shots` list.

#include "config.h"

// ---- Settings you can play with ----
const uint8_t SHOT_THRESHOLD = 200;      // a band must be louder than this (0-255) to fire
const uint16_t SHOT_COOLDOWN_MS = 150;   // wait this long (in thousandths of a second) between shots
const uint16_t SHOT_STEP_MS = 40;        // a light moves one LED every 40 ms. Smaller = faster
const uint8_t SHOT_TRAIL = 3;            // how many fading LEDs follow the bright head

// ---- What the animation remembers between frames ----

// One light travelling along the strip.
struct Shot {
  bool active;           // is this slot in use?
  int8_t position;       // the LED the bright head is on (0 = first LED)
  uint8_t red, green, blue;  // its colour
  uint32_t lastStepMs;   // when it last moved
};

const uint8_t MAX_SHOTS = 6;       // the most lights on the strip at once
static Shot shots[MAX_SHOTS];
static uint32_t lastFireMs = 0;    // when we last fired

void shootingLightsReset() {
  for (uint8_t i = 0; i < MAX_SHOTS; i++) shots[i].active = false;
  lastFireMs = 0;
}

// Mixes the colours of all bands that are over the threshold.
// Returns false if no band is over the threshold.
static bool mixTriggeredBands(const uint8_t levels[], uint8_t &red, uint8_t &green, uint8_t &blue) {
  uint16_t redSum = 0, greenSum = 0, blueSum = 0;
  bool anyTriggered = false;

  for (uint8_t band = 0; band < BAND_COUNT; band++) {
    if (levels[band] > SHOT_THRESHOLD) {
      uint32_t colour = Adafruit_NeoPixel::ColorHSV(bandHue(band));  // this band's full colour
      redSum += (colour >> 16) & 0xFF;
      greenSum += (colour >> 8) & 0xFF;
      blueSum += colour & 0xFF;
      anyTriggered = true;
    }
  }
  if (!anyTriggered) return false;

  // Adding colours can give numbers bigger than 255, so scale them down until the
  // strongest colour is exactly 255. This keeps the mix as bright as possible.
  uint16_t strongest = max(redSum, max(greenSum, blueSum));
  red = (uint32_t)redSum * 255 / strongest;
  green = (uint32_t)greenSum * 255 / strongest;
  blue = (uint32_t)blueSum * 255 / strongest;
  return true;
}

// Fires a new light if one is allowed.
static void tryToFire(const uint8_t levels[], uint32_t now) {
  if (now - lastFireMs < SHOT_COOLDOWN_MS) return;  // too soon since the last one

  uint8_t red, green, blue;
  if (!mixTriggeredBands(levels, red, green, blue)) return;  // nothing loud enough

  // Put the new light in the first free slot (if all slots are busy, skip it).
  for (uint8_t i = 0; i < MAX_SHOTS; i++) {
    if (!shots[i].active) {
      shots[i] = Shot{true, 0, red, green, blue, now};
      lastFireMs = now;
      return;
    }
  }
}

// Draws one frame. `levels` holds the 7 band values (0-255), `now` is millis().
void shootingLights(Adafruit_NeoPixel &strip, const uint8_t levels[], uint32_t now) {

  tryToFire(levels, now);

  strip.clear();  // start from a dark strip and draw every light on top

  for (uint8_t i = 0; i < MAX_SHOTS; i++) {
    Shot &shot = shots[i];
    if (!shot.active) continue;

    // Move on by one LED if it is time. (Adding the step time, rather than using
    // `now`, keeps the speed steady even if a frame arrives a little late.)
    if (now - shot.lastStepMs >= SHOT_STEP_MS) {
      shot.position++;
      shot.lastStepMs += SHOT_STEP_MS;
    }

    // Once the whole trail has gone off the end of the strip, free the slot.
    if (shot.position - SHOT_TRAIL >= (int16_t)LED_COUNT) {
      shot.active = false;
      continue;
    }

    // Draw the head (k = 0) and then the trail, each one dimmer than the last.
    for (uint8_t k = 0; k <= SHOT_TRAIL; k++) {
      int16_t led = shot.position - k;
      if (led < 0 || led >= (int16_t)LED_COUNT) continue;  // off the strip

      uint8_t fade = 255 - k * (255 / (SHOT_TRAIL + 1));   // 255 at the head, less behind it
      strip.setPixelColor(led, rgbColor((uint16_t)shot.red * fade / 255,
                                        (uint16_t)shot.green * fade / 255,
                                        (uint16_t)shot.blue * fade / 255));
    }
  }
}
