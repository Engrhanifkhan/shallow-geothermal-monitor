#include "Ds18b20Bus.h"

Ds18b20Bus::Ds18b20Bus() {
  for (uint8_t i = 0; i < kSensorCount; i++) {
    wires_[i] = nullptr;
    sensors_[i] = nullptr;
  }
}

Ds18b20Bus::~Ds18b20Bus() {
  for (uint8_t i = 0; i < kSensorCount; i++) {
    delete sensors_[i];
    delete wires_[i];
    sensors_[i] = nullptr;
    wires_[i] = nullptr;
  }
}

void Ds18b20Bus::begin() {
  Serial.println(F("[DS18B20] one sensor per pin"));

  for (uint8_t i = 0; i < kSensorCount; i++) {
    wires_[i] = new OneWire(kSensors[i].pin);
    sensors_[i] = new DallasTemperature(wires_[i]);
    sensors_[i]->begin();
    sensors_[i]->setResolution(kTempResolutionBits);
    sensors_[i]->setWaitForConversion(true);

    Serial.print(F("  "));
    Serial.print(kSensors[i].name);
    Serial.print(F(" GPIO "));
    Serial.print(kSensors[i].pin);
    Serial.print(F(" devices="));
    Serial.println(sensors_[i]->getDeviceCount());
  }
}

uint8_t Ds18b20Bus::deviceCount() {
  uint8_t total = 0;
  for (uint8_t i = 0; i < kSensorCount; i++) {
    if (sensors_[i] != nullptr) {
      total = static_cast<uint8_t>(total + sensors_[i]->getDeviceCount());
    }
  }
  return total;
}

void Ds18b20Bus::readAll(SensorReading *out) {
  for (uint8_t i = 0; i < kSensorCount; i++) {
    out[i].name = kSensors[i].name;
    out[i].feedKey = kSensors[i].feedKey;

    float c = DEVICE_DISCONNECTED_C;
    if (sensors_[i] != nullptr) {
      sensors_[i]->requestTemperatures();
      c = sensors_[i]->getTempCByIndex(0);
    }

    out[i].valid = (c != DEVICE_DISCONNECTED_C) && (c > -55.0f) && (c < 125.0f);
    out[i].celsius = out[i].valid ? c : NAN;
  }
}
