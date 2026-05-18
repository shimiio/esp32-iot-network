#include "env.h"
#include <WiFi.h>
#include <esp_now.h>
#include <DHT.h>
#include <esp_wifi.h>

#define DHT_PIN 4
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

uint8_t esp1MAC[] = MAC1;

typedef struct {
  float temp;
  float hum;
} SensorData;

SensorData data;

void onSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Sending: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Ok" : "Error");
}

// setup
void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  delay(500);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Error");
    return;
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peer;
  memset(&peer, 0, sizeof(peer));
  memcpy(peer.peer_addr, esp1MAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  esp_wifi_set_channel(3, WIFI_SECOND_CHAN_NONE);

  int channel = WiFi.channel();
  Serial.print("WiFi channel: ");
  Serial.println(channel);
}

// loop
void loop() {
  data.temp = dht.readTemperature();
  data.hum = dht.readHumidity();

  if (isnan(data.temp) || isnan(data.hum)) {
    Serial.println("Error DHT22");
    delay(2000);
    return;
  }

  esp_now_send(esp1MAC, (uint8_t *)&data, sizeof(data));

  delay(2000);
}
