#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "SimplePWM.h"
#include "driver/ledc.h"
#define DEFAULT_PWM_CHANNEL LEDC_CHANNEL_0 

#define PIN_PIR 4
#define PIN_LDR 2
#define PIN_LED 19

#define RXD2 22
#define TXD2 23

int valorUART = 0;
String uartBuffer = "";

const char* ssid = "HONOR";
const char* password = "gocomedor1";

const char* mqtt_server = "87db8cfc6f614558958fb91c28b95044.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "frank";
const char* mqtt_pass = "IoT2025FERMIN";

char deviceId[30];
char telemetryTopic[100];

WiFiClientSecure espClient;
PubSubClient client(espClient);
SimplePWM ledControl(PIN_LED, 2000, DEFAULT_PWM_CHANNEL, 12);

unsigned long lastMsg = 0;
const long interval = 2000;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_LDR, INPUT);  // ahora digital
  ledControl.writeDuty(0);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");

  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(deviceId, sizeof(deviceId), "ESP32-%02X%02X%02X", mac[3], mac[4], mac[5]);
  snprintf(telemetryTopic, sizeof(telemetryTopic), "proyecto/farolas/%s/telemetria", deviceId);

  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(deviceId, mqtt_user, mqtt_pass)) break;
    delay(5000);
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Leer UART2
  while (Serial2.available() > 0) {
    char c = Serial2.read();
    if (c == '\n') {
      uartBuffer.trim();
      if (uartBuffer.length() > 0) {
        valorUART = uartBuffer.toInt();
      }
      uartBuffer = "";
    } else {
      uartBuffer += c;
    }
  }

  int pirState = digitalRead(PIN_PIR);
  int ldrState = digitalRead(PIN_LDR);      
  float factorPIR = pirState;

  float salidaNorm = (0.4 * ldrState + 0.6 * factorPIR);
  int salida = (int)(salidaNorm * 4096);
  salida = constrain(salida, 0, 4096);

  int salidaFinal = (int)(0.80 * salida + 0.20 * valorUART);
  if (salidaFinal < salida) salidaFinal = salida;
  int ledDuty = constrain(salidaFinal, 0, 4096);

  if (ledControl.readDuty() != ledDuty) ledControl.writeDuty(ledDuty);

  // Publicación MQTT cada interval
  unsigned long now = millis();
  if (now - lastMsg > interval) {
    lastMsg = now;

    float dutyPercent = ledDuty / 4095.0;
    float voltaje = 12.0 * sqrt(dutyPercent);

    float R_foco = 12.0 / 0.180;
    float corriente = (voltaje / R_foco) * 1000; // mA

    float potencia = voltaje * (corriente / 1000.0) * 1000.0; // mW

    char jsonBuffer[350];
    snprintf(jsonBuffer, sizeof(jsonBuffer),
      "{\"pir_state\": %d, \"led_duty\": %d, \"ldr_val\": %d, \"voltage_V\": %.2f, \"current_mA\": %.2f, \"power_mW\": %.2f}",
      pirState, ledDuty, ldrState, voltaje, corriente, potencia
    );

    client.publish(telemetryTopic, jsonBuffer);
    Serial.print("LDR: "); Serial.print(ldrState);
    Serial.print(" | PIR: "); Serial.print(pirState);
    Serial.print(" | Enviando: "); Serial.println(jsonBuffer);
  }
}