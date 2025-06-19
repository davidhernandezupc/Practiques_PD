// Librerías necesarias para WiFi, SD, SPI e I2S
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include <driver/i2s.h>

// Definición de pines para I2S (interfaz de audio digital)
#define I2S_WS      42
#define I2S_SD_IN   2
#define I2S_SD_OUT  15
#define I2S_SCK     41

// Definición de pines para SPI y SD
#define SD_CS       10
#define SPI_MOSI    11
#define SPI_MISO    13
#define SPI_SCK     12

// Datos de la red WiFi en modo punto de acceso
const char* ssid = "ESP32-David_Laia";
const char* password = "123456789";
WiFiServer server(80); // Puerto 80 para HTTP

// Variables globales de audio
File wavFile;
bool grabando = false;
bool pausado = false;
bool pitchActivo = false; // Efecto pitch activado o no

uint32_t dataChunkSize = 0;
unsigned long lastFlushTime = 0;
const unsigned long flushInterval = 1000;
const uint32_t sampleRate = 16000;
const uint16_t bitsPerSample = 16;
const uint16_t numChannels = 1;
unsigned long grabacionStartTime = 0;
unsigned long tiempoPausado = 0;
unsigned long pausaInicio = 0;

// Configuración del bus I2S
void setupI2S() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX),
    .sample_rate = sampleRate,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_SD_OUT,
    .data_in_num = I2S_SD_IN
  };

  // Instalación del controlador I2S
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0); // Limpiar buffer DMA
}

// Función para escribir el encabezado WAV en el archivo
void writeWavHeader(File &file, uint32_t dataLength) {
  uint32_t chunkSize = 36 + dataLength;
  uint16_t audioFormat = 1;
  uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
  uint16_t blockAlign = numChannels * bitsPerSample / 8;

  file.seek(0); // Inicio del archivo
  file.write((const uint8_t*)"RIFF", 4);
  file.write((uint8_t*)&chunkSize, 4);
  file.write((const uint8_t*)"WAVEfmt ", 8);
  uint32_t subchunk1Size = 16;
  file.write((uint8_t*)&subchunk1Size, 4);
  file.write((uint8_t*)&audioFormat, 2);
  file.write((uint8_t*)&numChannels, 2);
  file.write((uint8_t*)&sampleRate, 4);
  file.write((uint8_t*)&byteRate, 4);
  file.write((uint8_t*)&blockAlign, 2);
  file.write((uint8_t*)&bitsPerSample, 2);
  file.write((const uint8_t*)"data", 4);
  file.write((uint8_t*)&dataLength, 4);
}

// Buscar un nombre de archivo disponible tipo /audioX.wav
String getNextAvailableFilename() {
  int fileIndex = 1;
  String filename;
  while (true) {
    filename = "/audio" + String(fileIndex) + ".wav";
    if (!SD.exists(filename)) return filename;
    fileIndex++;
  }
}

// Extraer el número del nombre del archivo
int extractAudioIndex(const String& filename) {
  int start = filename.indexOf("audio") + 5;
  int end = filename.indexOf(".wav");
  return filename.substring(start, end).toInt();
}

// Listar archivos WAV grabados
String listAudioFiles() {
  String filenames[100];
  int indexes[100];
  int count = 0;
  File dir = SD.open("/");
  while (true) {
    File file = dir.openNextFile();
    if (!file) break;
    String name = file.name();
    if (name.endsWith(".wav") && name.startsWith("/audio")) {
      filenames[count] = name;
      indexes[count] = extractAudioIndex(name);
      count++;
    }
    file.close();
  }

  // Ordenar los nombres por número
  for (int i = 0; i < count - 1; i++) {
    for (int j = 0; j < count - i - 1; j++) {
      if (indexes[j] > indexes[j + 1]) {
        int tmpIdx = indexes[j];
        indexes[j] = indexes[j + 1];
        indexes[j + 1] = tmpIdx;
        String tmpName = filenames[j];
        filenames[j] = filenames[j + 1];
        filenames[j + 1] = tmpName;
      }
    }
  }

  // Generar HTML con audio players
  String output = "";
  for (int i = 0; i < count; i++) {
    output += "<div><audio controls src='" + filenames[i] + "'></audio> " + filenames[i] + "</div>";
  }
  return output;
}

// Configuración inicial del sistema
void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  if (!SD.begin(SD_CS)) {
    Serial.println("❌ Error inicializando SD.");
    while (true);
  }

  Serial.println("✅ SD OK");
  setupI2S();
  Serial.println("✅ I2S OK");

  WiFi.softAP(ssid, password);
  server.begin();
  Serial.print("🟢 IP Web Server: ");
  Serial.println(WiFi.softAPIP());
}

// Procesar comandos de teclado (serie)
void procesarComandoTeclado(char comando) {
  switch (comando) {
    case 'r':
      // Iniciar grabación
      Serial.println("🔴 [Serial] Iniciando grabación");
      if (!grabando) {
        String filename = getNextAvailableFilename();
        wavFile = SD.open(filename, FILE_WRITE);
        if (!wavFile) {
          Serial.println("❌ [Serial] Error al crear archivo WAV");
          return;
        }
        for (int i = 0; i < 44; i++) wavFile.write((uint8_t)0);
        dataChunkSize = 0;
        grabando = true;
        pausado = false;
        grabacionStartTime = millis();
        tiempoPausado = 0;
      }
      break;

    case 's':
      // Detener grabación
      Serial.println("⏹️ [Serial] Deteniendo grabación");
      if (grabando) {
        writeWavHeader(wavFile, dataChunkSize);
        wavFile.flush();
        wavFile.close();
        grabando = false;
        pausado = false;
      }
      break;

    case 'p':
      // Pausar o reanudar grabación
      if (grabando) {
        pausado = !pausado;
        if (pausado) {
          pausaInicio = millis();
          Serial.println("⏸ [Serial] Grabación pausada");
        } else {
          tiempoPausado += millis() - pausaInicio;
          Serial.println("▶ [Serial] Grabación reanudada");
        }
      }
      break;

    case 'e':
      // Activar o desactivar efecto pitch
      pitchActivo = !pitchActivo;
      Serial.println(pitchActivo ? "🎛️ [Serial] Pitch activado" : "🎚️ [Serial] Pitch desactivado");
      break;

    default:
      Serial.println("❓ [Serial] Comando no reconocido. Usa: r = grabar, s = stop, p = pausa, e = pitch");
      break;
  }
}

// Calcular duración real de la grabación
unsigned long calcularTiempoGrabacion() {
  if (!grabando) return 0;

  if (pausado) {
    return pausaInicio - grabacionStartTime - tiempoPausado;
  } else {
    return millis() - grabacionStartTime - tiempoPausado;
  }
}

// Bucle principal
void loop() {
  WiFiClient client = server.available();

  // Comando desde puerto serie
  if (Serial.available()) {
    char c = Serial.read();
    procesarComandoTeclado(c);
  }

  // Procesar petición web
  if (client) {
    String req = "";
    while (client.connected() && client.available()) {
      req += (char)client.read();
    }

    // Analizar comandos por URL: r, s, p, e
    if (req.indexOf("GET /?cmd=r") != -1) {
      Serial.println("🔴 Iniciando grabación");
      String filename = getNextAvailableFilename();
      wavFile = SD.open(filename, FILE_WRITE);
      if (!wavFile) {
        Serial.println("❌ Error al abrir archivo WAV.");
        return;
      }
      for (int i = 0; i < 44; i++) wavFile.write((uint8_t)0);
      dataChunkSize = 0;
      grabando = true;
      pausado = false;
      grabacionStartTime = millis();
      tiempoPausado = 0;
    }

    if (req.indexOf("GET /?cmd=s") != -1 && grabando) {
      Serial.println("⏹️ Deteniendo grabación");
      writeWavHeader(wavFile, dataChunkSize);
      wavFile.flush();
      wavFile.close();
      grabando = false;
      pausado = false;
    }

    if (req.indexOf("GET /?cmd=p") != -1 && grabando) {
      pausado = !pausado;
      if (pausado) {
        pausaInicio = millis();
        Serial.println("⏸ Grabación pausada");
      } else {
        tiempoPausado += millis() - pausaInicio;
        Serial.println("▶ Grabación reanudada");
      }
    }

    if (req.indexOf("GET /?cmd=e") != -1) {
      pitchActivo = !pitchActivo;
      Serial.println(pitchActivo ? "🎛️ Pitch activado" : "🎚️ Pitch desactivado");
    }

    // Enviar HTML con botones y temporizador
    unsigned long tiempo = calcularTiempoGrabacion();
    unsigned int minutos = tiempo / 60000;
    unsigned int segundos = (tiempo % 60000) / 1000;
    char buffer[10];
    sprintf(buffer, "%02d:%02d", minutos, segundos);

    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html\r\n");
    client.println("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>");
    client.println("<style>");
    client.println("body { font-family: sans-serif; text-align: center; background-color: #f5f5f5; margin: 0; padding: 20px; }");
    client.println("h1 { color: #333; }");
    client.println(".button { display: inline-block; margin: 10px; padding: 15px 30px; font-size: 18px; border: none; border-radius: 10px; cursor: pointer; transition: background 0.3s ease; }");
    client.println(".record { background-color: #e53935; color: white; }");
    client.println(".record:hover { background-color: #c62828; }");
    client.println(".stop { background-color: #757575; color: white; }");
    client.println(".stop:hover { background-color: #616161; }");
    client.println(".pause { background-color: #ffb300; color: white; }");
    client.println(".pause:hover { background-color: #ffa000; }");
    client.println(".effect { background-color: #3949ab; color: white; }");
    client.println(".effect:hover { background-color: #303f9f; }");
    client.println("</style></head><body>");
    client.println("<h1>Control de Grabacion</h1>");
    client.println("<p>Tiempo de grabacion: <span id='tiempo'>" + String(buffer) + "</span></p>");
    client.println("<script>");
    client.println("let tiempoMs = " + String(tiempo) + ";");
    client.println("let grabando = " + String(grabando ? "true" : "false") + ";");
    client.println("let pausado = " + String(pausado ? "true" : "false") + ";");
    client.println("setInterval(() => {");
    client.println("  if (grabando && !pausado) {");
    client.println("    tiempoMs += 1000;");
    client.println("    let mins = Math.floor(tiempoMs / 60000);");
    client.println("    let secs = Math.floor((tiempoMs % 60000) / 1000);");
    client.println("    document.getElementById('tiempo').textContent = String(mins).padStart(2, '0') + ':' + String(secs).padStart(2, '0');");
    client.println("  }");
    client.println("}, 1000);");
    client.println("</script>");
    client.println("<button class='button record' onclick=\"location.href='/?cmd=r'\">Grabar</button>");
    client.println("<button class='button stop' onclick=\"location.href='/?cmd=s'\">Detener</button>");
    client.println("<button class='button pause' onclick=\"location.href='/?cmd=p'\">" + String(pausado ? "Reanudar" : "Pausar") + "</button>");
    client.println("<button class='button effect' onclick=\"location.href='/?cmd=e'\">" + String(pitchActivo ? "Pitch OFF" : "Pitch ON") + "</button>");
    client.println(listAudioFiles());
    client.println("</body></html>");
    client.stop();
  }

  // Capturar y guardar audio en tiempo real
  if (grabando && !pausado) {
    int16_t samples[512];
    size_t bytesRead;
    esp_err_t result = i2s_read(I2S_NUM_0, &samples, sizeof(samples), &bytesRead, portMAX_DELAY);
    if (result == ESP_OK && bytesRead > 0) {
      size_t bytesWritten;
      if (pitchActivo) {
        // Procesar pitch agudo suavizado (interpolación)
        int16_t processed[256];
        int outIndex = 0;
        for (int i = 0; i < (bytesRead / 2) - 1; i += 2) {
          processed[outIndex++] = (samples[i] + samples[i + 1]) / 2;
        }
        int outBytes = outIndex * 2;
        wavFile.write((uint8_t*)processed, outBytes);
        dataChunkSize += outBytes;
        i2s_write(I2S_NUM_0, processed, outBytes, &bytesWritten, portMAX_DELAY);
      } else {
        wavFile.write((uint8_t*)samples, bytesRead);
        dataChunkSize += bytesRead;
        i2s_write(I2S_NUM_0, samples, bytesRead, &bytesWritten, portMAX_DELAY);
      }

      // Guardar a SD cada cierto intervalo
      if (millis() - lastFlushTime >= flushInterval) {
        wavFile.flush();
        lastFlushTime = millis();
      }
    }
  }
}