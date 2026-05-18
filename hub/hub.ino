#include "env.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <esp_now.h>
#include <HTTPClient.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLDE_RESET -1
#define BUTTON_PIN 15

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLDE_RESET);

// MAC
uint8_t esp2MAC[] = MAC2;
uint8_t esp3MAC[] = MAC3;

// WiFi
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* serverUrl = SERVER_URL;

typedef struct {
  float temp;
  float hum;
} SensorData;

SensorData receivedData;
SensorData esp2;
SensorData esp3;

int currentEsp = 2;
bool lastButtonState = HIGH;

void onReceive(const esp_now_recv_info_t* info, const uint8_t* data, int len) {
  memcpy(&receivedData, data, sizeof(receivedData));

  if (memcmp(info->src_addr, esp2MAC, 6) == 0) {
    esp2 = receivedData;
  } else if (memcmp(info->src_addr, esp3MAC, 6) == 0) {
    esp3 = receivedData;
  }
}

// render display
void renderDisplay(SensorData data, String header) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(header);

  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print(data.temp);
  display.println(" C");

  display.setCursor(0, 42);
  display.print(data.hum);
  display.println(" %");

  display.display();
}

// setup
void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // inspaction
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED is not found");
    while (true)
      ;
  }

  WiFi.mode(WIFI_STA);

  // connecting to wifi
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // get currenct channel
  int channel = WiFi.channel();
  Serial.print("WiFi channel: ");
  Serial.println(channel);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi connected!");
  display.setCursor(0, 16);
  display.println(WiFi.localIP().toString());
  display.display();
  delay(2000);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW error");
    return;
  }

  esp_now_register_recv_cb(onReceive);
}

// loop
void loop() {
  bool currentButtonState = digitalRead(BUTTON_PIN);

  // toggle button logic
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    if (currentEsp == 2) {
      currentEsp = 3;
    } else {
      currentEsp = 2;
    }
  }
  lastButtonState = currentButtonState;


  // render data
  if (currentEsp == 2) {
    renderDisplay(esp2, "esp2 data");
  } else if (currentEsp == 3) {
    renderDisplay(esp3, "esp3 data");
  }

  // send data to PC
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.setTimeout(2000);
    http.addHeader("Content-Type", "application/json");
    String body = "{\"esp2_temp\":" + String(esp2.temp) + ",\"esp2_hum\":" + String(esp2.hum) + ",\"esp3_temp\":" + String(esp3.temp) + ",\"esp3_hum\":" + String(esp3.hum) + "}";
    http.POST(body);
    http.end();
  }
}
