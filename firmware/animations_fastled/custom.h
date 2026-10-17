#pragma once
// ANIMATION 4: CUSTOM  (this is YOURS: change anything you like!)
//
// This animation is already switched on. Run it with:
//     python scripts/audio_stream.py --port <your port> --file audio/melody7.wav --animation 4
//
// As it stands, it does one small thing so you can see it working: the whole strip
// is one colour, and it gets brighter when the bass is loud. Edit the code in
// custom() below, upload it, and watch what changes. Nothing else needs to change.
//
// HOW AN ANIMATION WORKS
//   The function custom() is called about 43 times a second, once for every "frame".
//   Each time it must decide what colour every LED should be. It does that in 3 steps:
//     1. LOOK at the sound (levels[]) and the clock (now).
//     2. DECIDE what to show.
//     3. DRAW it by setting leds[]. (main.cpp then sends leds[] to the real LEDs.)
//
// THINGS YOU CAN USE
//   levels[0] ... levels[6]   How loud each frequency band is, 0 (silent) to 255 (loud).
//                             levels[0] is bass, levels[6] is treble.
//   now                       Time in milliseconds since the Arduino started. Use it to
//                             make things happen at the right speed.
//   LED_COUNT                 How many LEDs there are (12).
//   leds[n]                   The colour of LED number n. The first LED is 0.
//
//   CRGB(255, 0, 0)           A colour from red, green and blue amounts (0-255). This is red.
//   CHSV(hue, 255, bright)    A colour from a hue (0-255 round the colour wheel), full colour,
//                             and a brightness (0-255). HUE_RED, HUE_ORANGE, HUE_YELLOW,
//                             HUE_GREEN, HUE_AQUA, HUE_BLUE, HUE_PURPLE and HUE_PINK are handy names.
//   CRGB::Black               Off.
//   BAND_HUES[band]           The colour of each band (see config.h).
//
//   fill_solid(leds, LED_COUNT, colour)   Set every LED to one colour.
//   fadeToBlackBy(leds, LED_COUNT, 60)    Make every LED a bit darker. Do it every frame and
//                                         anything you draw fades out slowly, like a trail.
//   random8(n)                            A random number from 0 to n-1.
//   map(value, 0, 255, 0, LED_COUNT)      Turn a level (0-255) into a position on the strip (0-12).
//   beatsin8(bpm, low, high)              A number that swings smoothly between low and high,
//                                         bpm times a minute. Good for pulsing and sweeping.
//
// SOME IDEAS TO TRY
//   - Make the strip's colour follow the treble instead of the bass (use levels[6]).
//   - Light one LED per band: LED 0 follows levels[0], LED 1 follows levels[1], and so on.
//   - Light the LED at position  map(levels[3], 0, 255, 0, LED_COUNT - 1)  and leave a trail
//     by calling fadeToBlackBy() first.
//   - Flash the whole strip white when levels[0] goes over a number you choose.

#include "config.h"

// ---- Settings you can play with ----
// Put numbers you may want to change here, with a note about what they do. Using a name like
// this is easier to read than a bare number in the middle of the code.
const uint8_t CUSTOM_HUE = HUE_GREEN;   // the colour of the strip (try HUE_RED, HUE_BLUE...)

// ---- What the animation remembers between frames ----
// Variables made here keep their value from one frame to the next. Use them for anything the
// animation needs to remember, such as a position, a timer or a smoothed level. For example:
//     static uint8_t position = 0;
// (Nothing to remember yet, because the example below only uses the current frame.)

// Called when the animation is chosen. Put your remembered variables back to their
// starting values here, so every time you pick this animation it starts fresh.
void customReset() {
}

// Draws one frame into leds[]. `levels` holds the 7 band values (0-255), `now` is millis().
void custom(const uint8_t levels[], uint32_t now) {

  // 1. LOOK: how loud is the bass?
  uint8_t bass = levels[0];

  // 2. DECIDE: use the bass level as the brightness.
  uint8_t brightness = bass;

  // 3. DRAW: set every LED to the same colour, at that brightness.
  //    DELETE these lines and write your own!
  fill_solid(leds, LED_COUNT, CHSV(CUSTOM_HUE, 255, brightness));
}
