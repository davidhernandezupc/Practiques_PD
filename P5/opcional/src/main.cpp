#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include <LiquidCrystal_I2C.h>
#include "spo2_algorithm.h"


#define SDA_PIN 8
#define SCL_PIN 9
#define LCD_ADDRESS 0x27  // Cambia a 0x3F si fuera necesario
#define LCD_COLUMNS 16
#define LCD_ROWS 2

MAX30105 particleSensor;
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
uint16_t irBuffer[100];
uint16_t redBuffer[100];
#else
uint32_t irBuffer[100];
uint32_t redBuffer[100];
#endif

int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

byte pulseLED = 11;
byte readLED = 13;

void setup() {
  Serial.begin(115200);
  pinMode(pulseLED, OUTPUT);
  pinMode(readLED, OUTPUT);

  // Inicializar I2C con pines personalizados
  Wire.begin(SDA_PIN, SCL_PIN);

  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando LCD...");

  // Inicializar sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor MAX30105");
    lcd.setCursor(0, 1);
    lcd.print("no detectado!");
    while (1);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Coloca el dedo");

  byte ledBrightness = 60;
  byte sampleAverage = 4;
  byte ledMode = 2;
  byte sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  // Esperar que el usuario pulse una tecla en Serial
  Serial.println("Presiona una tecla para empezar");
  while (Serial.available() == 0);
  Serial.read();
}

void loop() {
  bufferLength = 100;

  for (byte i = 0; i < bufferLength; i++) {
    while (!particleSensor.available())
      particleSensor.check();

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer,
                                         &spo2, &validSPO2, &heartRate, &validHeartRate);

  while (1) {
    for (byte i = 25; i < 100; i++) {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    for (byte i = 75; i < 100; i++) {
      while (!particleSensor.available())
        particleSensor.check();

      digitalWrite(readLED, !digitalRead(readLED));
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();

      // Mostrar en la pantalla LCD
      lcd.setCursor(0, 0);
      lcd.print("HR:");
      if (validHeartRate)
        lcd.print(heartRate);
      else
        lcd.print("--");
      
      lcd.print(" bpm");

      lcd.setCursor(0, 1);
      lcd.print("SpO2:");
      if (validSPO2)
        lcd.print(spo2);
      else
        lcd.print("--");

      lcd.print(" %  ");

      // También puedes mantener Serial para debug si lo deseas
      Serial.print("HR=");
      Serial.print(heartRate);
      Serial.print(", SPO2=");
      Serial.print(spo2);
      Serial.println();
    }

    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength,
                                           redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}