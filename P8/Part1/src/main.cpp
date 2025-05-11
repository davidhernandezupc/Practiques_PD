#include <Arduino.h>
#define RXD1 16
#define TXD1 17

void setup() {
  Serial.begin(115200);               // Comunicación por USB
  while (!Serial);

  Serial1.begin(9600, SERIAL_8N1, RXD1, TXD1); // Configura UART1
  Serial.println("Bucle UART con ESP32-S3 iniciado");
}

void loop() {
  static String inputString = ""; // Usamos una cadena para almacenar los caracteres

  // Lee los caracteres desde el monitor serie
  if (Serial.available()) {
    char incomingByte = Serial.read(); // Lee un byte
    if (incomingByte == '\n' || incomingByte == '\r') { 
      // Cuando recibe un salto de línea, procesa la entrada
      if (inputString.length() > 0) {
        // Envía lo que se ha escrito a UART1
        Serial1.write(inputString.c_str()); 
        Serial.println("Enviado a UART1: " + inputString); // Opcional: muestra en consola
        inputString = ""; // Limpia la entrada para la próxima
      }
    } else {
      // Si no es un salto de línea, agrega el caracter a la cadena
      inputString += incomingByte;
    }
  }

  // Lee los datos de UART1 y los muestra por el monitor serie
  if (Serial1.available()) {
    char receivedByte = Serial1.read(); // Lee el byte de UART1
    Serial.write(receivedByte); // Muestra el byte en el monitor serie
  }
}