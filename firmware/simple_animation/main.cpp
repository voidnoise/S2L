#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// Receives one frame per line from scripts/audio_stream.py:
//
//     12,45,200,88,3,0,9\n
//
// 7 comma-separated band levels, each 0-255 (bass first), and shows them on a
// strip of 12 WS2812 LEDs. The strip is split into 7 zones, one per band, each
// with its own colour (red = bass ... blue/violet = treble) whose brightness
// follows the band's level. See led_test/main.cpp for wiring.
//
// Anything that isn't a valid frame is reported as "err <line>". Set
// ECHO_FRAMES to also get "ok <levels>" back for every valid frame.

const uint8_t LED_PIN = 6;
const uint16_t LED_COUNT = 12;
const uint8_t BAND_COUNT = 7;
const uint8_t BRIGHTNESS = 100;      // 0-255, caps the current drawn from USB
const bool ECHO_FRAMES = false;

const uint8_t LINE_MAX = 40;         // longest valid line is 27 chars: "255,255,255,255,255,255,255"
const uint16_t HUE_PER_BAND = 8500;  // hue step between bands, 0 (red) to ~51000 (blue/violet)
const uint8_t DECAY_SHIFT = 2;       // levels fall by 1/4 of the gap each frame (rise is instant)
const uint16_t TIMEOUT_MS = 500;     // fade to black if no frame arrives for this long

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint8_t levels[BAND_COUNT];  // latest values received
uint8_t shown[BAND_COUNT];   // smoothed values actually displayed
char line[LINE_MAX];
uint8_t lineLen = 0;
bool overflowed = false;
uint32_t lastFrameMs = 0;

// Parses "a,b,c,d,e,f,g" into levels[]. Returns false if the line is malformed.
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
      return false;  // trailing junk after the last value
    }
  }
  memcpy(levels, parsed, BAND_COUNT);  // only overwrite once the whole line is valid
  return true;
}

// Rise instantly, fall smoothly, so the display punches on beats without flicker.
void smoothLevels() {
  for (uint8_t i = 0; i < BAND_COUNT; i++) {
    if (levels[i] >= shown[i]) shown[i] = levels[i];
    else shown[i] -= (shown[i] - levels[i] + (1 << DECAY_SHIFT) - 1) >> DECAY_SHIFT;
  }
}

void drawStrip() {
  for (uint16_t led = 0; led < LED_COUNT; led++) {
    uint8_t band = (uint32_t)led * BAND_COUNT / LED_COUNT;  // 12 LEDs over 7 bands: zones of 2,2,2,1,2,2,1
    uint16_t hue = (uint16_t)band * HUE_PER_BAND;
    strip.setPixelColor(led, strip.gamma32(strip.ColorHSV(hue, 255, shown[band])));
  }
  // show() blocks serial interrupts for ~0.4 ms. That is safe here because it runs
  // right after a frame's newline, and the next frame is ~20 ms away.
  strip.show();
}

void echoFrame() {
  Serial.print(F("ok "));
  for (uint8_t i = 0; i < BAND_COUNT; i++) {
    if (i > 0) Serial.print(',');
    Serial.print(levels[i]);
  }
  Serial.println();
}

void handleLine() {
  line[lineLen] = '\0';
  if (!overflowed && parseFrame(line)) {
    lastFrameMs = millis();
    if (ECHO_FRAMES) echoFrame();
    smoothLevels();
    drawStrip();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  } else {
    Serial.print(F("err "));
    if (overflowed) Serial.println(F("(line too long)"));
    else Serial.println(line);
  }
  lineLen = 0;
  overflowed = false;
}

// When the stream stops, let the display fade out instead of freezing on the last frame.
void fadeIfIdle() {
  static uint32_t lastFadeMs = 0;
  uint32_t now = millis();
  if (now - lastFrameMs < TIMEOUT_MS || now - lastFadeMs < 25) return;
  lastFadeMs = now;
  memset(levels, 0, BAND_COUNT);
  smoothLevels();
  drawStrip();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  strip.begin();
  strip.setBrightness(BRIGHTNESS);
  strip.show();  // start with everything off
  Serial.println(F("S2L ready: send 7 comma-separated values 0-255, e.g. 1,2,3,4,5,6,7"));
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
      overflowed = true;  // keep draining until the newline, then report it
    }
  }
  fadeIfIdle();
}
