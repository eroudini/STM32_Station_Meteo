#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "secrets.h"


static const int RX2_PIN = 16;
static const int TX2_PIN = 17;

WiFiClient espClient;
PubSubClient mqtt(espClient);

void WiFiConnect() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    while (WiFi.status() != WL_CONNECTED { delay(300); Serial.print("."); }
    Serial.printf("\nWiFi OK: %s IP=%s\n", WIFI_SSID, Wifi.localIP().toString().c_str()));
}

void mqttConnect() {
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    while (!mqtt.connected()) {
        Serial.printf("MQTT -> %s:%d ... \n", MQTT_HOST, MQTT_PORT);
        if (mqtt.connect("esp32-weather-client")) break;
        delay(1000);
    }
    Serial.println("MQTT OK");
}


void setup () {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);

    Serial.println(F("ESP32 pret, Attente lignes CSV 'T;N;P' ..."));
}

void loop() {
    if (!mqtt.connected()) mqttConnect();
    mqtt.loop();

    if (Serial2.available()) {
        String line = Serial2.readStringUntil('\n');
        line.trim();
        if (!line.length()) return;
    
    int s1 = line.indexOf(';');
    int s2 = line.indexOf(';', s1 + 1);
    if (s1 > 0 && s2 > s1) {
        float T = line.substring(0, s1).toFloat();
        float H = line.substring(s1 + 1, s2).toFloat();
        float P = line.substring(s2 + 1).toFloat();

        char json[128];
        snprintf(json, sizeof(json), "{\"t\":%.2f,\"h\":%.2f,\"p\":%.0f}", T, H, P);


        bool ok = mqtt.publish(MQTT_TOPIC, json);
        Serial.printf("PUB %s -> %s  [%s]\n", MQTT_TOPIC, json, ok ? "OK" : "KO");
        }
    }

}
