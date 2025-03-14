#include<Arduino.h>

struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
  };
  Button button1 = {18, 0, false};
  void IRAM_ATTR isr() { // funció a la que es crida quan passa la interrupció
    button1.numberKeyPresses += 1;
    button1.pressed = true;
  }
  void setup() {
    Serial.begin(115200);
    delay(1000);
    pinMode(button1.PIN, INPUT_PULLUP);
    attachInterrupt(button1.PIN, isr, FALLING);
    Serial.println("ESP32-S3 iniciado corectamnte!");
  }
  void loop() {
    if (button1.pressed) {
        Serial.printf("Button 1 has been pressed %u times\n",
  button1.numberKeyPresses);
        button1.pressed = false;
    }
    //Detach Interrupt after 1 Minute
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 60000) {
    lastMillis = millis();
    detachInterrupt(button1.PIN);
     Serial.println("Interrupt Detached!");
  }
}