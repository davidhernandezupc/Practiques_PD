#include <Arduino.h>

#define LED_BUILTIN 2
#define DELAY 500

void setup() {
pinMode(LED_BUILTIN, OUTPUT);
}
void loop() {
Serial.begin(115200);
digitalWrite(LED_BUILTIN, HIGH);
delay(DELAY);
Serial.println("OFF");
digitalWrite(LED_BUILTIN, LOW);
delay(DELAY);
Serial.println("ON");
}