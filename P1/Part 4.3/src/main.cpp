#include <Arduino.h>

#define LED_BUILTIN 4

void setup() {
pinMode(LED_BUILTIN, OUTPUT);
Serial.begin(115200);
}

void loop() {
Serial.begin(115200);
digitalWrite(LED_BUILTIN, HIGH);

digitalWrite(LED_BUILTIN, LOW);

}