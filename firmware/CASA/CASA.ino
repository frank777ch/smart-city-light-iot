#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "SimplePWM.h"

const char* ssid = "HONOR";
const char* password = "gocomedor1";

const char* mqtt_server = "87db8cfc6f614558958fb91c28b95044.s1.eu.hivemq.cloud"; 
const int mqtt_port = 8883;
const char* mqtt_user = "frank";
const char* mqtt_pass = "IoT2025FERMIN";

const int PIN_PIR = 13;
const int PIN_LED = 5;
const int PIN_LDR = 34;

const int UMBRAL_DIA = 2000; 

char deviceId[30];
char telemetryTopic[100];

WiFiClientSecure espClient;
PubSubClient client(espClient);
SimplePWM ledControl(PIN_LED, 5000);

unsigned long lastMsg = 0;
const long interval = 2000;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LDR, INPUT);

  delay(10);
  Serial.print("\nConectando a WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");

  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(deviceId, sizeof(deviceId), "ESP32-%02X%02X%02X", mac[3], mac[4], mac[5]);
  Serial.print("ID Dispositivo: ");
  Serial.println(deviceId);

  snprintf(telemetryTopic, sizeof(telemetryTopic), "proyecto/farolas/%s/telemetria", deviceId);
  Serial.print("Topic MQTT: ");
  Serial.println(telemetryTopic);

  espClient.setInsecure(); 
  client.setServer(mqtt_server, mqtt_port);

  ledControl.writeDuty(0); 
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando a HiveMQ...");
    if (client.connect(deviceId, mqtt_user, mqtt_pass)) {
      Serial.println("¡Conectado!");
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 5s...");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int pirState = digitalRead(PIN_PIR);
  int ldrValue = analogRead(PIN_LDR);
  int ledDuty = 0;

  bool esDeDia = (ldrValue > UMBRAL_DIA); 

  if (esDeDia) {
    ledDuty = 0; 
  } else {
    if (pirState == HIGH) {
      ledDuty = 255;
    } else {
      ledDuty = 51;
    }
  }

  if(ledControl.readDuty() != ledDuty) {
    ledControl.writeDuty(ledDuty);
  }

  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float voltaje = 5.0 + ((random(-10, 10)) / 100.0);
    float corriente = 0.0;

    if (ledDuty == 0) {
      corriente = 45.0 + random(-2, 2);
    } else if (ledDuty == 51) {
      corriente = 80.0 + random(-2, 2);
    } else {
      corriente = 180.0 + random(-5, 5);
    }
    
    float potencia = (voltaje * corriente); 

    char jsonBuffer[350];
    snprintf(jsonBuffer, sizeof(jsonBuffer), 
      "{\"pir_state\": %d, \"led_duty\": %d, \"ldr_val\": %d, \"voltage_V\": %.2f, \"current_mA\": %.2f, \"power_mW\": %.2f}",
      pirState, ledDuty, ldrValue, voltaje, corriente, potencia
    );

    client.publish(telemetryTopic, jsonBuffer);
    
    Serial.print("LDR: "); Serial.print(ldrValue);
    Serial.print(" | PIR: "); Serial.print(pirState);
    Serial.print(" | Enviando: "); Serial.println(jsonBuffer);
  }
}