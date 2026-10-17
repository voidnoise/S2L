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
//      threshold at that moment. We add the band colours together like shining
//      coloured torches on a wall: red + green makes yellow, and lots of colours
//      together make white. So bass (red) plus treble (purple) gives pink-purple.
//   4. MOVEMENT: each light moves one LED along every SHOT_STEP_MS. Every frame the
//      whole strip is dimmed a little (fadeToBlackBy), so the places the light has
//      been fade away slowly. That is what makes the trail. When the light goes off
//      the end, it is removed.
//
// Several lights can be on the strip at once. Each one is stored in the `shots` list.

#include "config.h"

// ---- Settings you can play with ----
const uint8_t SHOT_THRESHOLD = 200;      // a band must be louder than this (0-255) to fire
const uint16_t SHOT_COOLDOWN_MS = 150;   // wait this long (in thousandths of a second) between shots
const uint16_t SHOT_STEP_MS = 40;        // a light moves one LED every 40 ms. Smaller = faster
const uint8_t SHOT_FADE = 80;            // 0-255. How much the strip dims each frame. Bigger = shorter trail

// ---- What the animation remembers between frames ----

// One light travelling along the strip.
struct Shot {
  bool active;           // is this slot in use?
  int8_t position;       // the LED the light is on (0 = first LED)
  CRGB colour;           // its colour
  uint32_t lastStepMs;   // when it last moved
};

const uint8_t MAX_SHOTS = 6;       // the most lights on the strip at once
static Shot shots[MAX_SHOTS];
static uint32_t lastFireMs = 0;    // when we last fired

void shootingLightsReset() {
  for (uint8_t i = 0; i < MAX_SHOTS; i++) shots[i].active = false;
  lastFireMs = 0;
}

// Mixes the colours of all bands that are over the threshold into `mix`.
// Returns false if no band is over the threshold.
static bool mixTriggeredBands(const uint8_t levels[], CRGB &mix) {
  mix = CRGB::Black;
  bool anyTriggered = false;

  for (uint8_t band = 0; band < BAND_COUNT; band++) {
    if (levels[band] > SHOT_THRESHOLD) {
      mix += CHSV(BAND_HUES[band], 255, 255);   // += adds the colours (and stops at 255)
      anyTriggered = true;
    }
  }
  return anyTriggered;
}

// Fires a new light if one is allowed.
static void tryToFire(const uint8_t levels[], uint32_t now) {
  if (now - lastFireMs < SHOT_COOLDOWN_MS) return;  // too soon since the last one

  CRGB colour;
  if (!mixTriggeredBands(levels, colour)) return;   // nothing loud enough

  // Put the new light in the first free slot (if all slots are busy, skip it).
  for (uint8_t i = 0; i < MAX_SHOTS; i++) {
    if (!shots[i].active) {
      shots[i] = Shot{true, 0, colour, now};
      lastFireMs = now;
      return;
    }
  }
}

// Draws one frame into leds[]. `levels` holds the 7 band values (0-255), `now` is millis().
void shootingLights(const uint8_t levels[], uint32_t now) {

  // Dim everything a little. Lights drawn in earlier frames slowly fade out.
  fadeToBlackBy(leds, LED_COUNT, SHOT_FADE);

  tryToFire(levels, now);

  for (uint8_t i = 0; i < MAX_SHOTS; i++) {
    Shot &shot = shots[i];
    if (!shot.active) continue;

    // Move on by one LED if it is time. (Adding the step time, rather than using
    // `now`, keeps the speed steady even if a frame arrives a little late.)
    if (now - shot.lastStepMs >= SHOT_STEP_MS) {
      shot.position++;
      shot.lastStepMs += SHOT_STEP_MS;
    }

    // Off the end of the strip? Free the slot.
    if (shot.position >= (int16_t)LED_COUNT) {
      shot.active = false;
      continue;
    }

    leds[shot.position] = shot.colour;   // draw the light at full brightness
  }
}
