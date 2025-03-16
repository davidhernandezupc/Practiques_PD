#include <Arduino.h>

const int ledPin = 2;        // Pin donde está conectado el LED
unsigned long previousMillis = 0;
int blinkFrequency = 1000;    // Frecuencia inicial (1000ms)
bool ledState = false;

// Variables para simular los pulsadores
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;   // Filtro de rebote (200ms)
bool lastButtonStateUp = LOW;
bool lastButtonStateDown = LOW;

void setup() {
  Serial.begin(115200);  
  pinMode(ledPin, OUTPUT);
  Serial.println("Programa iniciado...");
}

void loop() {
  unsigned long currentMillis = millis();

  // Verifica si el monitor serial está funcionando
  Serial.println("Esperando pulsaciones...");

  // Simulamos la presion de un pulsador para aumentar la frecuencia
  if (digitalRead(23) == HIGH && currentMillis - lastDebounceTime > debounceDelay) { 
    blinkFrequency = max(100, blinkFrequency - 100);  // Limita la frecuencia mínima a 100ms
    Serial.print("Frecuencia aumentada: ");
    Serial.print(blinkFrequency);
    Serial.println("ms");
    lastDebounceTime = currentMillis;
  }

  // Simula la presion de otro pulsador para disminuir la frecuencia
  if (digitalRead(22) == HIGH && currentMillis - lastDebounceTime > debounceDelay) { 
    blinkFrequency = min(5000, blinkFrequency + 100);  // Limita la frecuencia máxima a 5000ms
    Serial.print("Frecuencia disminuida: ");
    Serial.print(blinkFrequency);
    Serial.println("ms");
    lastDebounceTime = currentMillis;
  }

  // Control del LED usando el temporizador
  if (currentMillis - previousMillis >= blinkFrequency) {
    previousMillis = currentMillis;

    // Cambia el estado del LED
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}