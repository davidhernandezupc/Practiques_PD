#include <Arduino.h>

#define LED_BUILTIN 2
#define DELAY 500

void setup() {
pinMode(LED_BUILTIN, OUTPUT);
Serial.begin(115200);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(DELAY);
    Serial.println("OFF");
    digitalWrite(LED_BUILTIN, LOW);
    delay(DELAY);
    Serial.println("ON");
}