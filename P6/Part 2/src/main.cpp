#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>

// Pines personalizados para ESP32-S3
#define SS_PIN 5    // SDA/SS
#define MOSI_PIN 6
#define MISO_PIN 7
#define SCK_PIN 8
#define RST_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Espera a que se abra el puerto serie

  // Inicializa SPI con tus pines personalizados
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);

  // Inicializa el módulo RC522
  mfrc522.PCD_Init(); 
  Serial.println("Lector RFID listo. Acerca una tarjeta...");
}

void loop() {
  // Revisa si hay una nueva tarjeta
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Intenta leer la tarjeta
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Muestra el UID de la tarjeta
  Serial.print("UID de la tarjeta: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    Serial.print(i < mfrc522.uid.size - 1 ? ":" : "");
  }
  Serial.println();

  // Finaliza comunicación con la tarjeta
  mfrc522.PICC_HaltA();
}