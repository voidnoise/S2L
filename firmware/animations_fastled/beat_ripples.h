#pragma once
// ANIMATION 3: BEAT RIPPLES
//
// Every time there is a drum beat, a ripple spreads out from the middle of the
// strip to both ends, like a drop falling in a pond. Each ripple is a new colour.
// High notes add little white-blue sparkles on top.
//
// How it works:
//   1. FINDING A BEAT: a beat is a sudden jump in the bass. We keep a slow-moving
//      AVERAGE of the bass level. If the bass right now is well above that average,
//      that is a beat. (Just asking "is the bass loud?" would not work: a long
//      loud bass note is not a beat, and quiet music still has beats.)
//   2. THE RIPPLE: a new ripple starts at the two middle LEDs. Every
//      RIPPLE_STEP_MS it grows by one LED each way. Every frame the whole strip is
//      dimmed a little (fadeToBlackBy), which leaves a fading trail behind the
//      ripple. When it reaches the ends, it is removed.
//   3. THE COLOUR: each beat's ripple has a new hue, so the colours keep changing.
//   4. SPARKLES: if the top band (treble) is loud, one random LED flashes for that frame.
//
// This animation expects an even number of LEDs, so there are two in the middle.

#include "config.h"

// ---- Settings you can play with ----
const uint8_t BEAT_MARGIN = 40;         // bass must be this far above its average to count as a beat
const uint8_t BEAT_MINIMUM = 90;        // ...and at least this loud, so quiet noise doesn't count
const uint16_t BEAT_GAP_MS = 200;       // two beats can't be closer than this
const uint16_t RIPPLE_STEP_MS = 70;     // the ripple grows by one LED every 70 ms. Smaller = faster
const uint8_t RIPPLE_FADE = 60;         // 0-255. How much the strip dims each frame. Bigger = shorter trail
const uint8_t HUE_CHANGE_PER_BEAT = 40; // how far round the colour wheel (0-255) each new ripple jumps
const uint8_t SPARKLE_THRESHOLD = 150;  // treble must be louder than this (0-255) to sparkle

// ---- What the animation remembers between frames ----

// One ripple spreading out from the middle.
struct Ripple {
  bool active;         // is this slot in use?
  uint8_t radius;      // how many LEDs the ripple is from the middle (0 = the two middle LEDs)
  uint8_t hue;         // its colour
  uint32_t lastStepMs; // when it last grew
};

const uint8_t MAX_RIPPLES = 3;          // the most ripples at once
static Ripple ripples[MAX_RIPPLES];
static uint8_t bassAverage = 0;         // slow-moving average of the bass
static uint32_t lastBeatMs = 0;         // when the last beat was
static uint8_t nextHue = 0;             // the colour of the next ripple

// The two middle LEDs. With 12 LEDs these are LED 5 and LED 6.
const int16_t LEFT_MIDDLE = LED_COUNT / 2 - 1;
const int16_t RIGHT_MIDDLE = LED_COUNT / 2;
// How far the ripple can go before it reaches the end of the strip (5 with 12 LEDs).
const uint8_t MAX_RADIUS = LED_COUNT / 2 - 1;

void beatRipplesReset() {
  for (uint8_t i = 0; i < MAX_RIPPLES; i++) ripples[i].active = false;
  bassAverage = 0;
  lastBeatMs = 0;
}

// Starts a new ripple in the first free slot (if all slots are busy, skip it).
static void startRipple(uint32_t now) {
  for (uint8_t i = 0; i < MAX_RIPPLES; i++) {
    if (!ripples[i].active) {
      ripples[i] = Ripple{true, 0, nextHue, now};
      nextHue += HUE_CHANGE_PER_BEAT;  // the next ripple gets a different colour
      return;
    }
  }
}

// Draws one frame into leds[]. `levels` holds the 7 band values (0-255), `now` is millis().
void beatRipples(const uint8_t levels[], uint32_t now) {

  // Dim everything a little. Parts of ripples drawn in earlier frames slowly fade out.
  fadeToBlackBy(leds, LED_COUNT, RIPPLE_FADE);

  // 1. Look for a beat in the bass (bands 0 and 1).
  uint8_t bass = (levels[0] + levels[1]) / 2;
  bool isBeat = bass >= BEAT_MINIMUM &&
                bass > bassAverage + BEAT_MARGIN &&
                now - lastBeatMs >= BEAT_GAP_MS;
  if (isBeat) {
    startRipple(now);
    lastBeatMs = now;
  }

  // Update the average. It moves 1/32 of the way to the new value every frame, so it
  // follows the general loudness but ignores single beats.
  bassAverage = ((uint16_t)bassAverage * 31 + bass) / 32;

  // 2. Grow every ripple and draw it.
  for (uint8_t i = 0; i < MAX_RIPPLES; i++) {
    Ripple &ripple = ripples[i];
    if (!ripple.active) continue;

    if (now - ripple.lastStepMs >= RIPPLE_STEP_MS) {
      ripple.radius++;
      ripple.lastStepMs += RIPPLE_STEP_MS;
    }

    // Past the ends of the strip? Free the slot.
    if (ripple.radius > MAX_RADIUS) {
      ripple.active = false;
      continue;
    }

    CRGB colour = CHSV(ripple.hue, 255, 255);
    leds[LEFT_MIDDLE - ripple.radius] = colour;    // one side of the middle...
    leds[RIGHT_MIDDLE + ripple.radius] = colour;   // ...and the same on the other side
  }

  // 3. Sparkle: a flash on a random LED whenever the top band is loud.
  if (levels[BAND_COUNT - 1] > SPARKLE_THRESHOLD) {
    leds[random8(LED_COUNT)] = CRGB(150, 150, 255);
  }
}
