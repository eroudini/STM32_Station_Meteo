# Station Météo – STM32F407 + ESP32

Projet PlatformIO : la STM32 lit **DHT22** (T/H) et **BMP280** (pression), affiche sur **OLED SSD1306**, puis envoie les données à une **ESP32** qui les publie en **MQTT**.

---

## 🔌 Câblage

| Module              | STM32                | ESP32 |
| ------------------- | -------------------- | ----- |
| BMP280 / OLED (I²C) | PB6 = SCL, PB7 = SDA | –     |
| DHT22               | PA8                  | –     |
| UART TX             | PA2 → GPIO16 (RX2)   |       |
| GND commun          | GND ↔ GND            |       |

---

## ⚙️ Fonctionnement

1. STM32 (HAL) lit les capteurs toutes 2 s.
2. Affiche T/H/P sur OLED.
3. Envoie une trame CSV `T;H;P\r\n` à l’ESP32.
4. ESP32 publie un JSON MQTT :

```json
{"t":23.4,"h":45.1,"p":101215,"ts":1730014501}
```

Topic : `weather/station1`

---

## 🧩 PlatformIO

`platformio.ini` contient 2 environnements :

* **stm32f407vet6** → firmware capteurs + OLED
* **esp32dev** → réception UART + MQTT (PubSubClient)

Commandes utiles :

```
pio run -e stm32f407vet6 -t upload    # flash STM32 via DFU
pio run -e esp32dev -t upload         # flash ESP32 via USB
pio device monitor -e esp32dev        # voir les JSON MQTT
```

---

## 🧱 Robustesse

* Moyenne glissante (N=5)
* Recovery I²C auto si BMP280 muet
* Watchdog 2 s (IWDG)
