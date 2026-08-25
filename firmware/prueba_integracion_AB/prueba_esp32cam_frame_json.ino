/*
  Frente A - Prueba de integración ESP32-CAM -> Frente B
  Flujo: Wi-Fi -> captura JPEG -> HTTP POST /frame -> JSON

  Hardware:
    ESP32-CAM AI-Thinker + OV2640
    Sin microSD

  Dependencias:
    - Paquete de placas ESP32 para Arduino IDE
    - ArduinoJson 7.x
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "esp_camera.h"

// ===== CONFIGURACIÓN =====
const char* WIFI_SSID     = "NOMBRE_DE_TU_WIFI"; // nombre de la red donde estarán conectados la ESP32 y el PC del Frente B
const char* WIFI_PASSWORD = "CONTRASENA_DE_TU_WIFI";

// Ejemplo: http://192.168.1.25:8000/frame // usar IP real del PC del frente B
const char* SERVER_URL = "http://192.168.1.X:8000/frame"; 

const unsigned long SEND_INTERVAL_MS = 3000;

// ===== PINES CÁMARA: ESP32-CAM AI-THINKER =====
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define FLASH_GPIO_NUM     4  // reservado para flash integrado

unsigned long lastSend = 0;

bool conectarWiFi() {
  if (WiFi.status() == WL_CONNECTED) return true;

  Serial.print("\nConectando a Wi-Fi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long inicio = millis();
  const unsigned long timeoutMs = 20000;

  while (WiFi.status() != WL_CONNECTED && millis() - inicio < timeoutMs) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nERROR: no fue posible conectar al Wi-Fi.");
    return false;
  }

  Serial.println("\nWi-Fi conectado.");
  Serial.print("IP ESP32-CAM: ");
  Serial.println(WiFi.localIP());
  return true;
}

bool iniciarCamara() {
  camera_config_t config = {};

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk  = XCLK_GPIO_NUM;
  config.pin_pclk  = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href  = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Frente B acepta distintos tamaños y redimensiona internamente.
  if (psramFound()) {
    config.frame_size   = FRAMESIZE_VGA;   // 640x480
    config.jpeg_quality = 12;
    config.fb_count     = 2;
    config.grab_mode    = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size   = FRAMESIZE_QVGA;  // 320x240
    config.jpeg_quality = 14;
    config.fb_count     = 1;
    config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("ERROR inicializando cámara: 0x%x\n", err);
    return false;
  }

  Serial.println("Cámara inicializada correctamente.");
  return true;
}

void mostrarRespuestaJSON(const String& payload) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("No se pudo interpretar el JSON: ");
    Serial.println(error.c_str());
    Serial.println("Respuesta cruda:");
    Serial.println(payload);
    return;
  }

  bool gatoDetectado = doc["gato_detectado"] | false;
  float velocidad = doc["velocidad"] | 0.0f;
  const char* direccion = doc["direccion"] | "desconocida";
  float confianza = doc["confianza"] | 0.0f;

  Serial.println("\n===== RESPUESTA FRENTE B =====");
  Serial.print("gato_detectado: ");
  Serial.println(gatoDetectado ? "true" : "false");

  if (!doc["gato_x"].isNull() && !doc["gato_y"].isNull()) {
    Serial.print("gato_x: ");
    Serial.println(doc["gato_x"].as<float>(), 4);
    Serial.print("gato_y: ");
    Serial.println(doc["gato_y"].as<float>(), 4);
  } else {
    Serial.println("gato_x: null");
    Serial.println("gato_y: null");
  }

  Serial.print("velocidad: ");
  Serial.println(velocidad, 4);

  Serial.print("direccion: ");
  Serial.println(direccion);

  Serial.print("confianza: ");
  Serial.println(confianza, 4);

  Serial.println("==============================\n");
}

bool capturarYEnviarFrame() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Reconectando...");
    if (!conectarWiFi()) return false;
  }

  Serial.println("Capturando JPEG...");
  camera_fb_t* fb = esp_camera_fb_get();

  if (!fb) {
    Serial.println("ERROR: no se pudo capturar la imagen.");
    return false;
  }

  Serial.print("JPEG capturado: ");
  Serial.print(fb->len);
  Serial.println(" bytes");

  WiFiClient client;
  HTTPClient http;

  Serial.print("POST -> ");
  Serial.println(SERVER_URL);

  if (!http.begin(client, SERVER_URL)) {
    Serial.println("ERROR: no se pudo iniciar HTTP.");
    esp_camera_fb_return(fb);
    return false;
  }

  http.setTimeout(15000);
  http.addHeader("Content-Type", "image/jpeg");

  // Equivalente al curl del Frente B:
  // curl -X POST -H "Content-Type: image/jpeg"
  // --data-binary "@imagen.jpg" http://IP:8000/frame
  int httpCode = http.POST(fb->buf, fb->len);

  esp_camera_fb_return(fb);

  if (httpCode <= 0) {
    Serial.print("ERROR HTTP: ");
    Serial.println(http.errorToString(httpCode));
    http.end();
    return false;
  }

  Serial.print("Código HTTP: ");
  Serial.println(httpCode);

  String payload = http.getString();
  http.end();

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Imagen recibida correctamente por Frente B.");
    mostrarRespuestaJSON(payload);
    return true;
  }

  Serial.println("El servidor respondió, pero no con 200 OK:");
  Serial.println(payload);
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println();
  Serial.println("======================================");
  Serial.println(" Frente A -> Frente B");
  Serial.println(" ESP32-CAM -> JPEG -> /frame -> JSON");
  Serial.println("======================================");

  if (!iniciarCamara()) {
    Serial.println("Fallo crítico de cámara.");
    while (true) delay(1000);
  }

  conectarWiFi();

  // Primera captura casi inmediatamente.
  lastSend = millis() - SEND_INTERVAL_MS;
}

void loop() {
  if (millis() - lastSend >= SEND_INTERVAL_MS) {
    lastSend = millis();
    capturarYEnviarFrame();
  }

  delay(20);
}
