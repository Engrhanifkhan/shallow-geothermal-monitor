#pragma once

#include "Ds18b20Bus.h"
#include <WiFi.h>
#include "AdafruitIO_WiFi.h"

class AdafruitCloud {
public:
  AdafruitCloud();

  void connect();
  void run();
  bool isConnected();
  void publish(const SensorReading *readings);

private:
  AdafruitIO_WiFi io_;
  AdafruitIO_Feed *feeds_[kSensorCount];
  AdafruitIO_Feed *deltaFeed_;
};
