#include <Arduino.h>

static const int RX2_PIN = 16;
static const int TX2_PIN = 17;

void setup () {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);

    Serial.println(F("ESP32 pret, Attente lignes CSV 'T;N;P' ..."));
}

void loop() {
    if (Serial2.available()) {
        String line = Serial2.readStringUntil('\n');
        line.trim();
        if (!line.length()) return;
    

    Serial.print(F("Recu : "));
    Serial.println(line);

    int s1 = line.indexOf(';');
    int s2 = line.indexOf(';', s1 + 1);
    if (s1 > 0 && s2 > s1) {
        float T = line.substring(0, s1).toFloat();
        float H = line.substring(s1 + 1, s2).toFloat();
        float P = line.substring(s2 + 1).toFloat();

        Serial.printf("T=%.2f C H=%.2f P=%.1f hPa\n", T, H, P / 100.0f);
        }
    }

    if (Serial.available()) {
        Serial2.write(Serial.read());
    }
}