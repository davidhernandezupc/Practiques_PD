#include <Arduino.h>

#define ADC_PIN 2       // Pin de entrada analógica (ADC)
#define DAC_PIN 25       // Pin de salida analógica (DAC)

// Configurar el canal de PWM
#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8 // Resolución de 8 bits (0-255)

void setup() {
    Serial.begin(115200);  // Iniciar la comunicación por el puerto serie

    // Configurar el ADC
    analogReadResolution(12); // Resolución de 12 bits (0-4095)
    analogSetAttenuation(ADC_0db);  // Atenuación para que el rango de entrada sea de 0-3.3V

    // Configurar el PWM para el DAC
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);  // Configurar el canal PWM
    ledcAttachPin(DAC_PIN, PWM_CHANNEL);  // Asignar el pin al canal PWM
}

void loop() {
    // Leer valor del ADC (entrada analógica)
    int adcValue = analogRead(ADC_PIN);

    // Enviar el valor por el puerto serie
    Serial.print("ADC Value: ");
    Serial.println(adcValue);

    // Mapeamos el valor de ADC (0-4095) a (0-255) para la salida PWM
    int pwmValue = map(adcValue, 0, 4095, 0, 255);

    // Escribir el valor al pin DAC a través del PWM
    ledcWrite(PWM_CHANNEL, pwmValue);

    delay(100);  // Pausa de 100 ms
}