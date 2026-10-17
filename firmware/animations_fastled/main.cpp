#include <Arduino.h>
#include <FastLED.h>

#include "config.h"
#include "vu_meter.h"
#include "shooting_lights.h"
#include "beat_ripples.h"
#include "custom.h"

// Shows one of four animations on a strip of WS2812 LEDs, driven by the audio
// that scripts/audio_stream.py sends over USB. See README.md in this folder.
//
// This file does the "plumbing" and has no animation code in it:
//   - reads lines from the serial port,
//   - turns a line like  12,45,200,88,3,0,9  into the 7 band levels,
//   - hands the levels to the chosen animation and shows the result on the strip.
//
// Each animation lives in its own file: vu_meter.h, shooting_lights.h, beat_ripples.h and
// custom.h. (custom.h is an empty template for you to write your own.)
//
// Lines from the computer look like this:
//   12,45,200,88,3,0,9   seven band levels (0-255), lowest frequency first
//   2                    a single digit 1-4 picks an animation

const uint8_t ANIMATION_VU_METER = 1;
const uint8_t ANIMATION_SHOOTING_LIGHTS = 2;
const uint8_t ANIMATION_BEAT_RIPPLES = 3;
const uint8_t ANIMATION_CUSTOM = 4;
const uint8_t ANIMATION_COUNT = 4;

const bool ECHO_FRAMES = false;     // true = send "ok <levels>" back for every frame (for debugging)
const uint16_t SILENCE_MS = 500;    // if no frame arrives for this long, treat the sound as silent
const uint8_t LINE_MAX = 40;        // longest valid line is 27 chars: "255,255,255,255,255,255,255"

uint8_t currentAnimation = ANIMATION_VU_METER;
uint8_t levels[BAND_COUNT];   // the latest band levels (0-255)
char line[LINE_MAX];          // the line being received
uint8_t lineLen = 0;
bool overflowed = false;
uint32_t lastFrameMs = 0;     // when the last frame arrived

// Turns "a,b,c,d,e,f,g" into the levels list. Returns false if the line is wrong.
bool parseFrame(char *text) {
  uint8_t parsed[BAND_COUNT];
  char *p = text;
  for (uint8_t i = 0; i < BAND_COUNT; i++) {
    char *end;
    long value = strtol(p, &end, 10);
    if (end == p || value < 0 || value > 255) return false;
    parsed[i] = (uint8_t)value;

    if (i < BAND_COUNT - 1) {
      if (*end != ',') return false;
      p = end + 1;
    } else if (*end != '\0') {
      return false;  // junk after the last number
    }
  }
  memcpy(levels, parsed, BAND_COUNT);  // only change levels once the whole line was good
  return true;
}

// Runs the chosen animation for one frame and shows it on the strip.
void drawFrame() {
  uint32_t now = millis();
  switch (currentAnimation) {
    case ANIMATION_VU_METER:         vuMeter(levels); break;
    case ANIMATION_SHOOTING_LIGHTS:  shootingLights(levels, now); break;
    case ANIMATION_BEAT_RIPPLES:     beatRipples(levels, now); break;
    case ANIMATION_CUSTOM:           custom(levels, now); break;
  }
  // show() pauses the Arduino's serial for about half a millisecond. That is safe here
  // because we only call it right after a whole line has arrived, and the next one is
  // about 20 ms away.
  FastLED.show();
}

void chooseAnimation(uint8_t number) {
  currentAnimation = number;
  vuMeterReset();
  shootingLightsReset();
  beatRipplesReset();
  customReset();
  FastLED.clear();

  Serial.print(F("animation "));
  Serial.print(number);
  switch (number) {
    case ANIMATION_VU_METER:         Serial.println(F(": vu meter")); break;
    case ANIMATION_SHOOTING_LIGHTS:  Serial.println(F(": shooting lights")); break;
    case ANIMATION_BEAT_RIPPLES:     Serial.println(F(": beat ripples")); break;
    case ANIMATION_CUSTOM:           Serial.println(F(": custom")); break;
  }
}

void echoFrame() {
  Serial.print(F("ok "));
  for (uint8_t i = 0; i < BAND_COUNT; i++) {
    if (i > 0) Serial.print(',');
    Serial.print(levels[i]);
  }
  Serial.println();
}

// Called each time a whole line has been received.
void handleLine() {
  line[lineLen] = '\0';

  if (overflowed) {
    Serial.println(F("err (line too long)"));
  } else if (lineLen == 1 && line[0] >= '1' && line[0] < '1' + ANIMATION_COUNT) {
    chooseAnimation(line[0] - '0');                      // a single digit: pick an animation
  } else if (parseFrame(line)) {                         // otherwise it should be 7 levels
    lastFrameMs = millis();
    if (ECHO_FRAMES) echoFrame();
    drawFrame();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));  // blink the board's LED for each frame
  } else {
    Serial.print(F("err "));
    Serial.println(line);
  }

  lineLen = 0;
  overflowed = false;
}

// When the audio stops, keep running the animation with silent levels so it can
// finish: the bar falls, the lights and ripples run off the end.
void drawSilenceIfIdle() {
  static uint32_t lastDrawMs = 0;
  uint32_t now = millis();
  if (now - lastFrameMs < SILENCE_MS || now - lastDrawMs < 25) return;

  lastDrawMs = now;
  memset(levels, 0, BAND_COUNT);
  drawFrame();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Tell FastLED what is connected: WS2812 LEDs on LED_PIN, with colours in
  // green-red-blue order, stored in the leds[] list.
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true);  // start with everything off

  Serial.println(F("S2L animations (FastLED) ready. Send 1, 2, 3 or 4 to pick an animation."));
  chooseAnimation(ANIMATION_VU_METER);
}

void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      handleLine();
    } else if (lineLen < LINE_MAX - 1) {
      line[lineLen++] = c;
    } else {
      overflowed = true;  // keep reading until the end of the line, then report it
    }
  }
  drawSilenceIfIdle();
}
