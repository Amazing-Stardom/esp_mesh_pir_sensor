#include "secrets.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <EspNowFloodingMesh.h>
#include <SimpleMqtt.h>
#include <SimpleTimer.h>

// Required to read internal voltage
ADC_MODE(ADC_VCC);

#define DEVICENAME "pir2round"

SimpleMQTT simpleMqtt = SimpleMQTT(ttl, DEVICENAME, 23, 40, 10);
SimpleTimer timer;

// Function to read voltage and send to Master
void sendSensorData() {
  int vdd = ESP.getVcc();
  char payload[32];
  snprintf(payload, sizeof(payload), "%d", vdd);

  Serial.print("Publishing Voltage to Mesh: ");
  Serial.println(payload);

  // Publish using the offline SimpleMqtt wrapper
  simpleMqtt.publish("m", "/int/sensor/value", payload);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.println("\n--- SLAVE NODE STARTING ---");

  espNowFloodingMesh_secredkey(secredKey);
  espNowFloodingMesh_setAesInitializationVector(iv);
  espNowFloodingMesh_setToMasterRole(false, ttl);

  // Set to FALSE so this node stays awake and routes traffic for other nodes!
  espNowFloodingMesh_setToBatteryNode(false);

  espNowFloodingMesh_begin(ESP_NOW_CHANNEL, shared_bsid, false);

  Serial.print("Syncing with Master...");
  // The specific sync function from the leodesigner fork
  while (!espNowFloodingMesh_syncTimeAnnonceAndWait(
      (uint8_t *)DEVICENAME, strlen(DEVICENAME), 10, 12, 10)) {
    Serial.println("\nSync failed. Retrying...");
    delay(2000);
  }
  Serial.println("\nSynced with Master!");

  // Use the SimpleTimer to send data every 5 seconds without blocking the loop
  timer.setInterval(5000, sendSensorData);
}

void loop() {
  espNowFloodingMesh_loop();
  simpleMqtt.resend_loop();
  timer.run();
}