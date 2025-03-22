#include <WiFi.h>
#include <WebServer.h>

// Definir el SSID y contraseña de la red WiFi AP
const char* ssid_ap = "ESP32-AccessPoint";  // Nombre de la red WiFi creada por el ESP32
const char* password_ap = "12345678";       // Contraseña de la red WiFi

WebServer server(80); // Crear el servidor web en el puerto 80

// Contenido HTML de la página web
String HTML = "<!DOCTYPE html>\
<html>\
<head>\
    <title>ESP32 AP Mode</title>\
    <meta charset='UTF-8'>\
    <style> body { font-family: Arial, sans-serif; text-align: center; } </style>\
</head>\
<body>\
    <h1>ESP32 en Modo AP 🚀</h1>\
    <p>Conéctate a esta red WiFi sin necesidad de un router.</p>\
</body>\
</html>";

// Función para manejar la solicitud a la página principal "/"
void handle_root() {
  server.send(200, "text/html", HTML); 
}

void setup() {
  Serial.begin(115200);
  Serial.println("\nConfigurando Access Point...");

  // Configurar el ESP32 en modo AP
  WiFi.softAP(ssid_ap, password_ap);

  // Obtener la IP asignada en modo AP (normalmente 192.168.4.1)
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Dirección IP del ESP32 AP: ");
  Serial.println(IP);

  // Configurar el servidor web
  server.on("/", handle_root);
  server.begin();
  Serial.println("Servidor Web en modo AP iniciado");
}

void loop() {
  server.handleClient();
}