#include <Arduino.h>

// Definir los pines donde están conectados los LEDs
const int ledPin1 = 2; // Pin 2 de la ESP32
const int ledPin2 = 9; // Pin 9 de la ESP32

// Función para controlar el LED1 con un bucle for
void taskLed1(void *parameter) {
  for (;;) {  // Este bucle se repetirá 10 veces
    // Encender el LED 1
    digitalWrite(ledPin1, HIGH);
    vTaskDelay(700 / portTICK_PERIOD_MS); // Esperar 700 ms
    
    // Apagar el LED 1
    digitalWrite(ledPin1, LOW);
    vTaskDelay(700 / portTICK_PERIOD_MS); // Esperar 700 ms
  }
  vTaskDelete(NULL);  // Terminar la tarea una vez completados los 10 ciclos
}

// Función para controlar el LED2 con un bucle for
void taskLed2(void *parameter) {
  for (;;) {  // Este bucle se repetirá 10 veces
    // Encender el LED 2
    digitalWrite(ledPin2, HIGH);
    vTaskDelay(500 / portTICK_PERIOD_MS); // Esperar 500 ms
    
    // Apagar el LED 2
    digitalWrite(ledPin2, LOW);
    vTaskDelay(500 / portTICK_PERIOD_MS); // Esperar 500 ms
  }
  vTaskDelete(NULL);  // Terminar la tarea una vez completados los 10 ciclos
}

void setup() {
  // Inicializar los pines de los LEDs como salida
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  // Crear las tareas
  xTaskCreate(taskLed1, "Task Led1", 1000, NULL, 1, NULL); // Tarea para el LED 1
  xTaskCreate(taskLed2, "Task Led2", 1000, NULL, 1, NULL); // Tarea para el LED 2
}

void loop() {
  // El loop está vacío porque las tareas están manejando la lógica de los LEDs
}
