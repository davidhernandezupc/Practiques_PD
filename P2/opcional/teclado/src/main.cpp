#include <Arduino.h>

const int ledPin = 2;        // Pin donde está conectado el LED
unsigned long previousMillis = 0;
int blinkFrequency = 1000;    // Frecuencia inicial (1000ms)
bool ledState = false;

void setup() {
  Serial.begin(115200);  
  pinMode(ledPin, OUTPUT);
  Serial.println("Programa iniciado...");
  Serial.println("Usa 'u' para aumentar la frecuencia y 'd' para disminuirla.");
}

void loop() {
  unsigned long currentMillis = millis();

  // Verifica si hay datos disponibles en el Monitor Serial
  if (Serial.available() > 0) {
    char receivedChar = Serial.read();

    // Simula el comportamiento del pulsador UP (aumentar frecuencia)
    if (receivedChar == 'u') { 
      blinkFrequency = max(100, blinkFrequency - 100);  // Limita la frecuencia mínima a 100ms
      Serial.print("Frecuencia aumentada: ");
      Serial.print(blinkFrequency);
      Serial.println("ms");
    }

    // Simula el comportamiento del pulsador DOWN (disminuir frecuencia)
    if (receivedChar == 'd') { 
      blinkFrequency = min(5000, blinkFrequency + 100);  // Limita la frecuencia máxima a 5000ms
      Serial.print("Frecuencia disminuida: ");
      Serial.print(blinkFrequency);
      Serial.println("ms");
    }
  }

  // Control del LED usando el temporizador
  if (currentMillis - previousMillis >= blinkFrequency) {
    previousMillis = currentMillis;

    // Cambia el estado del LED
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}