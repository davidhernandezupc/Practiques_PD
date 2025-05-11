#include <TinyGPS++.h>

// Definir la instancia del GPS y el puerto serial
TinyGPSPlus gps;
HardwareSerial mySerial(1);  // Usamos UART1 en el ESP32 (puedes usar UART0 o UART2 también)

#define RX_PIN 16  // Pin RX para el GPS
#define TX_PIN 17  // Pin TX para el GPS

void setup()
{
  // Inicializa la comunicación con el monitor serie
  Serial.begin(115200);
  while (!Serial);

  // Inicializa la comunicación serial con el GPS
  mySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  Serial.println("Esperando coordenadas del GPS...");
}

void loop()
{
  // Lee los datos del GPS
  while (mySerial.available() > 0)
  {
    gps.encode(mySerial.read());  // Decodifica cada byte recibido del GPS

    if (gps.location.isUpdated())  // Si la ubicación ha sido actualizada
    {
      // Muestra la latitud y longitud en formato decimal
      float latitude = gps.location.lat();
      float longitude = gps.location.lng();

      Serial.print("Latitud: ");
      Serial.print(latitude, 6);  // Muestra la latitud con 6 decimales
      Serial.print("  Longitud: ");
      Serial.print(longitude, 6);  // Muestra la longitud con 6 decimales
      Serial.println();

      // Opcional: también puedes mostrar más datos, como la cantidad de satélites
      Serial.print("Satélites: ");
      Serial.println(gps.satellites.value());
    }
  }
}