# ESP32 Sensor Network

Two ESP32 sensors (ESP2, ESP3) send temperature and humidity data via ESP-NOW
to a hub (ESP1) which displays it on an OLED screen and forwards it to a PC server.
The server logs live readings to the console, stores them in SQLite, and a
separate local LLM analyzer periodically reviews the stored data for anomalies.

## Hardware

- 3x ESP32 DevKit
- 2x DHT22 sensor module
- 1x 0.96" OLED display (SSD1306 I2C)
- 1x push button

## Wiring

**ESP1 (hub)**

- OLED: SDA -> GPIO21, SCL -> GPIO22, VCC -> 3.3V, GND -> GND
- Button: one leg -> GPIO15, other leg -> GND

**ESP2 / ESP3 (sensors)**

- DHT22: DATA -> GPIO4, VCC -> 3.3V, GND -> GND

## Setup

1. Install Arduino IDE and ESP32 board package
2. Install libraries: Adafruit SSD1306, Adafruit GFX, DHT sensor library
3. Get the MAC address of ESP1, update `sensor/sensor.ino`
4. Flash `sensor/sensor.ino` to ESP2 and ESP3
5. Get MAC addresses by running the MAC sketch, update `hub/hub.ino`
6. Set your WiFi credentials in `hub/hub.ino`
7. Flash `hub/hub.ino` to ESP1
8. Install Ollama and pull a model (llama3.2)
9. Install Python dependencies: `pip install requests`
10. Run `python server/server.py` on your PC to start receiving and storing data
11. Run `python server/analyzer.py` in a separate terminal to start periodic LLM analysis

## Button

Press to toggle display between ESP2 and ESP3 data.
