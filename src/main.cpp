#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "project_config.h"

#define PORT             23
#define COOLANT_PIN_HIGH 4
#define COOLANT_PIN_LOW  5

#define LASER_ON_CMD     "M4"
#define LASER_OFF_CMD    "M5"
#define COOLANT_ON_CMD   "M8"
#define COOLANT_OFF_CMD  "M9"

WiFiServer server(PORT);
WiFiClient client;
WiFiClient lbrn_client;

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
    digitalWrite(COOLANT_PIN_HIGH, HIGH);
    digitalWrite(COOLANT_PIN_LOW, LOW);
  } else {
    digitalWrite(COOLANT_PIN_HIGH, LOW);
    digitalWrite(COOLANT_PIN_LOW, HIGH);
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

void process_communication() {
  if(!lbrn_client) {return;}
  while(1) {
    if(!lbrn_client.connected() || !client.connected()) {break;}
    if(lbrn_client.available()) {
      String command = lbrn_client.readString();
      Serial.printf("Received: %s\n", command.c_str());
      client.write(command.c_str());
      String response = client.readString();
      Serial.printf("Engraver:\n%s\n", response.c_str());
      lbrn_client.write(response.c_str());
    }
  }
}

void connect_lbrn_engraver() {
  while(1) {
    lbrn_client = server.accept();
    if(lbrn_client) {
      Serial.printf("Lightburn connected (%s).\n", lbrn_client.remoteIP().toString().c_str());
      if(client.connect(ENGRAVER_IP, ENGRAVER_PORT)) {
        Serial.printf("Engraver connected (%s).\n", ENGRAVER_IP);
        process_communication();
        client.stop();
      } else {
        Serial.printf("Engraver is not reachable on %s\n", ENGRAVER_IP);
      }

      lbrn_client.stop();
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(COOLANT_PIN_HIGH, OUTPUT);
  pinMode(COOLANT_PIN_LOW, OUTPUT);
  switch_coolant(false);
  connect_to_ap(SSID, PASS);
  server.begin();
  indicate_readiness();
  connect_lbrn_engraver();
}

void loop() {}