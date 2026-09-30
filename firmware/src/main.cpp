#include <Arduino.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("hello from S2L Workshop firmware");
  delay(1000);
}
