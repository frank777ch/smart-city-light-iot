#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "SimplePWM.h" // Tu librería del LED

// ================= CREDENCIALES =================
const char* ssid = "HONOR";          // <--- CAMBIAR
const char* password = "gocomedor1";      // <--- CAMBIAR

const char* mqtt_server = "87db8cfc6f614558958fb91c28b95044.s1.eu.hivemq.cloud"; // <--- TU URL HIVEMQ
const int mqtt_port = 8883;
const char* mqtt_user = "frank";
const char* mqtt_pass = "IoT2025FERMIN";

// ================= VARIABLES DINÁMICAS =================
char deviceId[30];       // Aquí guardaremos el ID único (ej. ESP32-A1B2C3)
char telemetryTopic[100]; // Aquí guardaremos el topic final

// ================= HARDWARE =================
const int PIN_PIR = 13;
const int PIN_LED = 5;

WiFiClientSecure espClient;
PubSubClient client(espClient);
SimplePWM ledControl(PIN_LED, 5000);

unsigned long lastMsg = 0;
const long interval = 2000; 

void setup() {
  Serial.begin(115200);
  pinMode(PIN_PIR, INPUT);

  // 1. CONEXIÓN WIFI
  delay(10);
  Serial.print("\nConectando a WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");

  // 2. GENERAR ID ÚNICO (MAC ADDRESS)
  uint8_t mac[6];
  WiFi.macAddress(mac);
  // Formato: ESP32-XXYYZZ (Últimos 3 pares de la MAC)
  snprintf(deviceId, sizeof(deviceId), "ESP32-%02X%02X%02X", mac[3], mac[4], mac[5]);
  
  Serial.print("--- ID DE DISPOSITIVO: ");
  Serial.println(deviceId);

  // 3. GENERAR TOPIC DINÁMICO
  // El topic será: proyecto/farolas/ESP32-A1B2C3/telemetria
  snprintf(telemetryTopic, sizeof(telemetryTopic), "proyecto/farolas/%s/telemetria", deviceId);
  Serial.print("--- TOPIC DE ENVIO: ");
  Serial.println(telemetryTopic);

  // 4. CONFIGURAR HIVEMQ SEGURO
  espClient.setInsecure(); // Saltamos validación estricta de certificado (necesario en ESP32 simples)
  client.setServer(mqtt_server, mqtt_port);

  // Iniciar LED suave
  ledControl.writeDuty(51); 
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando a HiveMQ...");
    // Usamos el deviceId como ClientID para MQTT también
    if (client.connect(deviceId, mqtt_user, mqtt_pass)) {
      Serial.println("¡Conectado!");
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- LÓGICA SENSORES ---
  int pirState = digitalRead(PIN_PIR);
  int ledDuty = (pirState == HIGH) ? 255 : 51;
  
  if(ledControl.readDuty() != ledDuty) ledControl.writeDuty(ledDuty);

  // --- ENVIAR DATOS CADA 2 SEGUNDOS ---
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    // Simulación de valores (Hasta que tengas el INA219)
    float voltaje = 5.0 + ((random(-10, 10)) / 100.0);
    float corriente = (pirState == HIGH) ? 120.0 : 30.0; 
    float potencia = voltaje * corriente; 

    // Crear JSON
    char jsonBuffer[256];
    // NOTA: Ya no enviamos el ID dentro del JSON, porque Node-RED lo saca del Topic.
    // Pero si quieres enviarlo por seguridad, puedes dejarlo.
    snprintf(jsonBuffer, sizeof(jsonBuffer), 
      "{\"pir_state\": %d, \"led_duty\": %d, \"voltage_V\": %.2f, \"current_mA\": %.2f, \"power_mW\": %.2f}",
      pirState, ledDuty, voltaje, corriente, potencia
    );

    Serial.print("Publicando en [");
    Serial.print(telemetryTopic);
    Serial.print("]: ");
    Serial.println(jsonBuffer);

    client.publish(telemetryTopic, jsonBuffer);
  }
}