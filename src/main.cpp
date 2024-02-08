#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "project_config.h"

#define PORT             23
#define LOW_COOLANT_PIN  4
#define HIGH_COOLANT_PIN 5

#define COOLANT_OFF  "OFF"
#define COOLANT_LOW  "LOW"
#define COOLANT_HIGH "HIGH"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDRESS 0x3D

WiFiServer server(PORT);
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);
bool display_installed = false;

void reboot() {
  for(int i = 3; i > 0; i--) {
      printf("\rRestarting in %d");
    }
    esp_restart();
}

void connect_to_ap(const char* ssid, const char* psk) {
  if(WiFi.status() == WL_CONNECTED) {
    printf("Connected with IP address: %s\n", WiFi.localIP().toString().c_str());
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, psk);
  if(WiFi.waitForConnectResult(3600000) == WL_CONNECTED) {
    printf("Connected with IP address: %s\n", WiFi.localIP().toString().c_str());
    return;
  }

  printf("Failed to connect to '%s'\n", ssid);
  reboot();
}

void indicate_readiness() {
  for(int i = 0; i < 3; i++) {
    digitalWrite(LOW_COOLANT_PIN, HIGH);
    delay(500);
    digitalWrite(LOW_COOLANT_PIN, LOW);
    delay(200);
  }
}

void show(String status) {
  if(!display_installed) {
    return;
  }

  display.setTextSize(1);
  display.println("Coolanter v1.0");
  display.println();
  if(status.equals("CONNECTING")) {
    display.printf("Connecting to %s...\n", SSID);
  } else {
    display.printf("Connected to %s\nIP address %s\nCoolant %s", SSID, WiFi.localIP().toString().c_str(), status.c_str());
  }
  
  display.display();
}

void switch_coolant(String command) {
  show(command);
  if(command.equals(COOLANT_LOW)) {
    printf("LOW coolant is on.\n");
    digitalWrite(LOW_COOLANT_PIN, HIGH);
    digitalWrite(HIGH_COOLANT_PIN, LOW);
  } else if(command.equals(COOLANT_HIGH)) {
    printf("HIGH coolant is on.\n");
    digitalWrite(LOW_COOLANT_PIN, LOW);
    digitalWrite(HIGH_COOLANT_PIN, HIGH);
  } else {
    printf("Coolant is OFF.\n");
    digitalWrite(LOW_COOLANT_PIN, LOW);
    digitalWrite(HIGH_COOLANT_PIN, LOW);
  }
}

void process_commands() {
  while(1) {
    WiFiClient client = server.available();
    if(client) {
      printf("client %s connected.\n", client.remoteIP().toString().c_str());
      while(client.available()) {
        String command = client.readString();
        printf("Received: %s\n", command.c_str());
        switch_coolant(command);
      }

      client.stop();
    }
  }
}

void setup() {
  display_installed = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  show("CONNECTING");
  pinMode(LOW_COOLANT_PIN, OUTPUT);
  pinMode(HIGH_COOLANT_PIN, OUTPUT);
  switch_coolant(COOLANT_OFF);
  connect_to_ap(SSID, PASS);
  server.begin();
  indicate_readiness();
  process_commands();
}

void loop() {}