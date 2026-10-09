#include "AdafruitCloud.h"
#include "Config.h"
#include "Ds18b20Bus.h"

Ds18b20Bus sensorBus;
AdafruitCloud cloud;
SensorReading readings[kSensorCount];
unsigned long lastPublishMs = 0;

void printReadings() {
  for (uint8_t i = 0; i < kSensorCount; i++) {
    Serial.print(readings[i].name);
    Serial.print(F(": "));
    if (readings[i].valid) {
      Serial.print(readings[i].celsius, 2);
      Serial.println(F(" C"));
    } else {
      Serial.println(F("no reading"));
    }
  }

  if (readings[kInlet].valid && readings[kOutlet].valid) {
    Serial.print(F("deltaT (inlet-outlet): "));
    Serial.print(readings[kInlet].celsius - readings[kOutlet].celsius, 2);
    Serial.println(F(" C"));
  }
  Serial.println(F("----"));
}

void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println(F("\n==========================================================="));
  Serial.println(F(" Shallow Geothermal Heat Exchanger (HGHE) Monitor"));
  Serial.println(F(" System Designed & Engineered by Engr. Hanif Khan"));
  Serial.println(F("==========================================================="));

  Serial.println(F("\n[1/2] Initializing DS18B20 Temperature Probes..."));
  sensorBus.begin();

  Serial.println(F("\n[2/2] Establishing Wi-Fi & Cloud Telemetry..."));
  cloud.connect();

  Serial.println(F("\n>> Telemetry loop started (20s interval). Streaming data...\n"));
  lastPublishMs = millis() - kPublishIntervalMs;
}

void loop() {
  cloud.run();

  if (millis() - lastPublishMs < kPublishIntervalMs) {
    return;
  }
  lastPublishMs = millis();

  sensorBus.readAll(readings);
  printReadings();
  cloud.publish(readings);
}
