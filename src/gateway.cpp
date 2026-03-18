#include "secrets.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EspNowFloodingMesh.h>
#include <SimpleMqtt.h>

#define DEVICENAME "gateway_master"

// Initialize SimpleMqtt
SimpleMQTT simpleMqtt = SimpleMQTT(ttl, DEVICENAME, 23, 40, 10);

void setup() {
  Serial.begin(115200);
  delay(2000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println("\n--- MESH GATEWAY STARTING ---");

  espNowFloodingMesh_secredkey(secredKey);
  espNowFloodingMesh_setAesInitializationVector(iv);
  espNowFloodingMesh_setToMasterRole(true, ttl);

  // The 'false' here is specific to the leodesigner fork
  espNowFloodingMesh_begin(ESP_NOW_CHANNEL, shared_bsid, false);

  // Handle incoming MQTT-like messages over the Mesh!
  simpleMqtt.handleEvents([](const char *src_node, const char *msgid,
                             char command, const char *topic,
                             const char *value) {
    Serial.println("\n--- [ NEW MESH MQTT MESSAGE ] ---");
    Serial.printf("From Node : %s\n", src_node);
    Serial.printf("Topic     : %s\n", topic);
    Serial.printf("Payload   : %s\n", value);
    Serial.println("---------------------------------");
  });

  Serial.println("Gateway Ready. Listening for Slave nodes...");
}

void loop() {
  espNowFloodingMesh_loop();
  simpleMqtt.resend_loop(); // Handles MQTT queue and retries
}