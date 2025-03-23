#include <Arduino.h>

// Declaración de funciones para tareas FreeRTOS
void ledON(void *pvParameters);
void ledOFF(void *pvParameters);

int LED = 2;  // Pin del LED

// Crear dos semáforos binarios
SemaphoreHandle_t semaforo_ON;
SemaphoreHandle_t semaforo_OFF;

void setup() {
    Serial.begin(115200);
    pinMode(LED, OUTPUT);

    // Crear los semáforos
    semaforo_ON = xSemaphoreCreateBinary();
    semaforo_OFF = xSemaphoreCreateBinary();

    if (semaforo_ON != NULL && semaforo_OFF != NULL) {
        xSemaphoreGive(semaforo_ON);  // Comenzamos con `ledON` habilitado
    }

    // Crear tareas con la misma prioridad para alternar perfectamente
    xTaskCreate(ledON, "LED ON", 1000, NULL, 1, NULL);
    xTaskCreate(ledOFF, "LED OFF", 1000, NULL, 1, NULL);
}

void loop() {
    vTaskDelete(NULL);  // No se usa loop() en FreeRTOS
}

// Tarea para encender el LED
void ledON(void *pvParameters) {
    for (;;) {
        if (xSemaphoreTake(semaforo_ON, portMAX_DELAY)) {  // Espera a su turno
            Serial.println("Ejecutando tarea: LED ON");
            digitalWrite(LED, HIGH);
            vTaskDelay(pdMS_TO_TICKS(1000));  // LED encendido por 1 seg
            xSemaphoreGive(semaforo_OFF);  // Habilita la tarea `ledOFF`
        }
    }
}

// Tarea para apagar el LED
void ledOFF(void *pvParameters) {
    for (;;) {
        if (xSemaphoreTake(semaforo_OFF, portMAX_DELAY)) {  // Espera su turno
            Serial.println("Ejecutando tarea: LED OFF");
            digitalWrite(LED, LOW);
            vTaskDelay(pdMS_TO_TICKS(1000));  // LED apagado por 1 seg
            xSemaphoreGive(semaforo_ON);  // Habilita la tarea `ledON`
        }
    }
}