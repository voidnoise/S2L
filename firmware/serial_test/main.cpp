#include <Arduino.h>

// Receives one frame per line from scripts/audio_stream.py:
//
//     12,45,200,88,3,0,9\n
//
// 7 comma-separated band levels, each 0-255. Each valid frame is echoed back
// as "ok 12,45,200,88,3,0,9" and anything else as "err <line>", and the
// on-board LED toggles per valid frame so you can see data arriving.

const uint8_t BAND_COUNT = 7;
const uint8_t LINE_MAX = 40;  // longest valid line is 27 chars: "255,255,255,255,255,255,255"

uint8_t levels[BAND_COUNT];
char line[LINE_MAX];
uint8_t lineLen = 0;
bool overflowed = false;

// Parses "a,b,c,d,e,f,g" into levels[]. Returns false if the line is malformed.
bool parseFrame(char *text) {
  char *p = text;
  for (uint8_t i = 0; i < BAND_COUNT; i++) {
    char *end;
    long value = strtol(p, &end, 10);
    if (end == p || value < 0 || value > 255) return false;
    levels[i] = (uint8_t)value;

    if (i < BAND_COUNT - 1) {
      if (*end != ',') return false;
      p = end + 1;
    } else if (*end != '\0') {
      return false;  // trailing junk after the last value
    }
  }
  return true;
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
    echoFrame();
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  } else {
    Serial.print(F("err "));
    if (overflowed) Serial.println(F("(line too long)"));
    else Serial.println(line);
  }
  lineLen = 0;
  overflowed = false;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
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
}
