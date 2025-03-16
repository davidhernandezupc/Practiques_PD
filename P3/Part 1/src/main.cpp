#include <WiFi.h>
#include <WebServer.h>

// SSID & Password
const char* ssid = "Nautilus"; // Enter your SSID here
const char* password = "20000Leguas"; //Enter your Password here

// Object of WebServer (HTTP port, 80 is default)
WebServer server(80);

// HTML & CSS contents which display on web server
String HTML = "<!DOCTYPE html>\
<html>\
<body>\
<h1>My Primera Pagina con ESP32 - Station Mode &#128522; :)</h1>\
</body>\
</html>";

// Handle root url (/)
void handle_root() {
  server.send(200, "text/html", HTML);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Try Connecting to ");
  Serial.println(ssid);

  // Connect to your Wi-Fi modem
  WiFi.begin(ssid, password);

  // Check Wi-Fi is connected to Wi-Fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP()); // Show ESP32 IP on serial

  // Define the handler for root URL
  server.on("/", handle_root);
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle incoming client requests
  server.handleClient();
}
