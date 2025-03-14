#include <Arduino.h>

void ledON(void *pvParameters);
void ledOFF(void *pvParameters);

int LED = 2;
SemaphoreHandle_t semaforo;

void setup() {
    Serial.begin(115200);
    pinMode(LED, OUTPUT);

    // Crear semáforo binario en lugar de mutex
    semaforo = xSemaphoreCreateBinary();
    if (semaforo != NULL) {
        xSemaphoreGive(semaforo); // Inicializar el semáforo en estado disponible
    }

    // Crear tareas con prioridades diferentes
    xTaskCreate(ledON, "LED ON", 1000, NULL, 2, NULL);
    xTaskCreate(ledOFF, "LED OFF", 1000, NULL, 1, NULL);
}

void loop() {
    // No es necesario en FreeRTOS, ya que las tareas manejan la ejecución
    vTaskDelete(NULL);
}

void ledON(void *pvParameters) {
    for (;;) {
        if (xSemaphoreTake(semaforo, portMAX_DELAY)) { // Intentar tomar el semáforo
            Serial.println("Ejecutando tarea: LED ON");
            digitalWrite(LED, HIGH);
            vTaskDelay(pdMS_TO_TICKS(100)); // Usar FreeRTOS delay
            xSemaphoreGive(semaforo); // Liberar semáforo
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Pequeño retardo para evitar sobrecarga de CPU
    }
}

void ledOFF(void *pvParameters) {
    for (;;) {
        if (xSemaphoreTake(semaforo, portMAX_DELAY)) {
            Serial.println("Ejecutando tarea: LED OFF");
            digitalWrite(LED, LOW);
            vTaskDelay(pdMS_TO_TICKS(100)); // Usar FreeRTOS delay
            xSemaphoreGive(semaforo);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
