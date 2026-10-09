#pragma once

#include "Config.h"
#include <DallasTemperature.h>
#include <OneWire.h>

struct SensorReading {
  const char *name;
  const char *feedKey;
  float celsius;
  bool valid;
};

class Ds18b20Bus {
public:
  Ds18b20Bus();
  ~Ds18b20Bus();

  void begin();
  uint8_t deviceCount();
  void readAll(SensorReading *out);

private:
  OneWire *wires_[kSensorCount];
  DallasTemperature *sensors_[kSensorCount];
};
