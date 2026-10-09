#pragma once

#include "Secrets.h"
#include <Arduino.h>

static const uint8_t kSensorCount = 6;
static const uint8_t kTempResolutionBits = 12;

// Adafruit IO free plan: 30 points/minute. Six feeds every 20 s = 18/min.
static const unsigned long kPublishIntervalMs = 20000;

enum SensorId : uint8_t {
  kRoom = 0,
  kOutdoor,
  kInlet,
  kOutlet,
  kPipeWall,
  kGround
};

struct SensorMap {
  SensorId id;
  const char *name;
  const char *feedKey;
  uint8_t pin;
};

// One DS18B20 DATA wire per GPIO. Each DATA line needs its own 4.7 kOhm
// pull-up to 3.3 V. Change pins here if your board uses different GPIOs.
static const SensorMap kSensors[kSensorCount] = {
    {kRoom, "room", "room-temp", 4},
    {kOutdoor, "outdoor", "outdoor-temp", 5},
    {kInlet, "inlet", "inlet-temp", 18},
    {kOutlet, "outlet", "outlet-temp", 19},
    {kPipeWall, "pipe_wall", "pipe-wall-temp", 21},
    {kGround, "ground", "ground-temp", 22},
};
