#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif

#define ONE_WIRE_BUS 12
#define RESOLUTION 12

//Basic Wifi and MQTT-Server Information
const char* ssid = "Oversun";
const char* password = "****";
const char* server = "192.168.31.100";
const char* mqttTopic = "home/outdoor/temp1";

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

WiFiClient wifiClient;

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

PubSubClient client(server, 1883, callback, wifiClient);

void connectWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void connectMQTT() {
  while (!client.connected()) {

    while (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }

    Serial.print("Attempting MQTT connection...");

    String clientName;
    clientName += "ESP8266Client";
    uint8_t mac[6];
    WiFi.macAddress(mac);
    clientName += macToStr(mac);

    if (client.connect((char*) clientName.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(ONE_WIRE_BUS, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  delay(10);

  sensors.begin();
}

void publish(char* str) {
  if (client.publish(mqttTopic, str)) {
    Serial.println("Publish ok");
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    Serial.println("Publish failed");
  }
}


void loop() {
  if (!client.connected()) {
    connectMQTT();
  }

  sensors.requestTemperatures();

  float tempC = sensors.getTempCByIndex(0);

  char charBuf[4];
  dtostrf(tempC, 2, 2, charBuf);

  Serial.println(tempC);

  publish(charBuf);

  delay(10000);
}

