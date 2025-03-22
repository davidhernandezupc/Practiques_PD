#include <WiFi.h>
#include <WebServer.h>
#include "FS.h"
#include "SPIFFS.h"  // Librería para manejar archivos en el ESP32

const char* ssid = "Nautilus"; 
const char* password = "20000Leguas";

WebServer server(80);

// Función para leer y servir el archivo HTML
void handle_root() {
  File file = SPIFFS.open("data/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Error al cargar el archivo HTML");
    return;
  }
  String htmlContent = file.readString();
  file.close();
  server.send(200, "text/html", htmlContent);
}

void setup() {
  Serial.begin(115200);

  // Iniciar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.print("IP asignada: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_root);
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
}