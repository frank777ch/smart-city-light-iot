#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "CONDOR-5G";
const char* password = "";
const char* mqtt_server = "192.168.1.210";

WiFiClient espClient;
PubSubClient client(espClient);

char deviceId[20];
char telemetryTopic[50];

void setup() {
  Serial.begin(115220);
  delay(10);

  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(deviceId, sizeof(deviceId), "ESP32-%02X%02X%02X", mac[3], mac[4], mac[5]);
  Serial.print("Mi ID de dispositivo unico es: ");
  Serial.println(deviceId);

  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("Mi IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);

  snprintf(telemetryTopic, sizeof(telemetryTopic), "proyecto/farolas/%s/telemetria", deviceId);
  Serial.print("Publicare en el topic: ");
  Serial.println(telemetryTopic);
}

void reconnect() {
  
    while (!client.connected()) {
    Serial.print("Intentando conexion MQTT...");
    
    if (client.connect(deviceId)) {
      Serial.println("conectado!");
    
      client.publish(telemetryTopic, "{\"status\": \"online\"}");
    } else {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

    static unsigned long lastMsg = 0;
  if (millis() - lastMsg > 10000) {
    lastMsg = millis();
    char msg[50];
    snprintf(msg, sizeof(msg), "{\"timestamp\": %lu, \"status\": \"alive\"}", millis());
    client.publish(telemetryTopic, msg);
    Serial.print("Mensaje publicado en ");
    Serial.println(telemetryTopic);
    Serial.println(msg);
  }
}
