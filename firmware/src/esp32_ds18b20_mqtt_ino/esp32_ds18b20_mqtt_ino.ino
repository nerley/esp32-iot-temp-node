#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ---------- CONFIG ----------
#define ONE_WIRE_BUS 4

const char* WIFI_SSID = "Luz Mary";
const char* WIFI_PASS = "Mar1n1ta";

const char* MQTT_BROKER = "test.mosquitto.org";
const int   MQTT_PORT   = 1883;
const char* MQTT_TOPIC  = "nerley/iot/temperature";0
// ----------------------------

// DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// WiFi & MQTT
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;
const long publishInterval = 5000; // 5 seconds

void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_DS18B20_Client")) {
      Serial.println(" connected");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  sensors.begin();

  connectWiFi();

  client.setServer(MQTT_BROKER, MQTT_PORT);
  connectMQTT();
}

void loop() {
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastPublish >= publishInterval) {
    lastPublish = now;

    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);

    StaticJsonDocument<128> doc;
    doc["device_id"] = "esp32-node-01";
    doc["temperature_c"] = tempC;

    char payload[128];
    serializeJson(doc, payload);

    Serial.print("Publishing: ");
    Serial.println(payload);

    client.publish(MQTT_TOPIC, payload);
  }
}
