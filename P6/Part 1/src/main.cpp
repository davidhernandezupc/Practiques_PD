#include <SPI.h>
#include <SD.h>

#define CS_PIN    5
#define SCK_PIN   18
#define MISO_PIN  2
#define MOSI_PIN  15

SPIClass spi = SPIClass(FSPI); // Usamos el bus SPI1 (FSPI)
File myFile;

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Inicializa SPI en pines personalizados
  spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  Serial.print("Iniciando SD... ");
  if (!SD.begin(CS_PIN, spi, 1000000)) {
    Serial.println("Fallo. Verifica:");
    Serial.println("- Pines y conexiones");
    Serial.println("- Formato FAT32");
    Serial.println("- Resistencia pull-up");
    while (1);
  }

  Serial.println("Correcto");

  // Lee archivo
  myFile = SD.open("/myFile.txt");
  if (myFile) {
    Serial.println("Contenido:");
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close();
  } else {
    Serial.println("Error abriendo archivo");
  }
}

void loop() {}