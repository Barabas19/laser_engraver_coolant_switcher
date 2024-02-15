#include <Arduino.h>

#if BOARD == ESP8266

#include <ESP8266WiFi.h>
#include "project_config.h"

#define PORT             23
#define COOLANT_PIN      4
#define INDICATOR_PIN    5

#define COOLANT_ON_CMD   "ON"

WiFiServer server(PORT);

void reboot() {
  for(int i = 3; i > 0; i--) {
      Serial.printf("\rRestarting in %d", i);
    }
    ESP.restart();
}

void connect_to_ap(const char* ssid, const char* psk) {
  if(WiFi.status() == WL_CONNECTED) {
    Serial.printf("Connected with IP address: %s\n", WiFi.localIP().toString().c_str());
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setHostname("CoolantSwitcher");
  WiFi.begin(ssid, psk);
  if(WiFi.waitForConnectResult(3600000) == WL_CONNECTED) {
    Serial.printf("Connected with IP address: %s\n", WiFi.localIP().toString().c_str());
    return;
  }

  Serial.printf("Failed to connect to '%s'\n", ssid);
  reboot();
}

void switch_coolant(bool on) {
  if(on) {
    digitalWrite(COOLANT_PIN, HIGH);
    digitalWrite(INDICATOR_PIN, LOW);
  } else {
    digitalWrite(COOLANT_PIN, LOW);
    digitalWrite(INDICATOR_PIN, HIGH);
  }
}

void indicate_readiness() {
  for(int i = 0; i < 3; i++) {
    switch_coolant(true);
    delay(200);
    switch_coolant(false);
    delay(200);
  }
}

void process_commands() {
  while(1) {
    WiFiClient client = server.accept();
    if(client) {
      Serial.printf("client %s connected.\n", client.remoteIP().toString().c_str());
      while(client.available()) {
        String command = client.readString();
        Serial.printf("Received: %s\n", command.c_str());
        switch_coolant(command.equals(COOLANT_ON_CMD));
      }

      client.stop();
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(COOLANT_PIN, OUTPUT);
  pinMode(INDICATOR_PIN, OUTPUT);
  switch_coolant(false);
  connect_to_ap(SSID, PASS);
  server.begin();
  indicate_readiness();
  process_commands();
}

#elif BOARD == MEGA2560

void setup() {
  uint64_t baud_rate = 115200;
  Serial.begin(baud_rate);
  Serial1.begin(baud_rate);
  Serial2.begin(baud_rate);
  delay(3000);
  Serial.println("Start communication between Serial1 and Serial2...");
  while(1) {
    if(Serial1.available()) {
      String str = Serial1.readString();
      Serial2.print(str);
      Serial.print("Serial1: ");
      Serial.println(str);
    }
    if(Serial2.available()) {
      String str = Serial2.readString();
      Serial1.print(str);
      Serial.print("Serial2: ");
      Serial.println(str);
    }
  }

  Serial.println("Baudrate detection finished.");
}

#endif

void loop() {}