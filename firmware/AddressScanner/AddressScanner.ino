#include <DallasTemperature.h>
#include <OneWire.h>

// Same GPIOs as Config.h — one DS18B20 DATA pin each.
static const uint8_t kPins[] = {4, 5, 18, 19, 21, 22};
static const char *kNames[] = {
    "room", "outdoor", "inlet", "outlet", "pipe_wall", "ground"};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(F("Pin check: one DS18B20 on each GPIO"));

  for (uint8_t i = 0; i < 6; i++) {
    OneWire wire(kPins[i]);
    DallasTemperature sensors(&wire);
    sensors.begin();
    Serial.print(kNames[i]);
    Serial.print(F(" GPIO "));
    Serial.print(kPins[i]);
    Serial.print(F(" -> "));
    Serial.print(sensors.getDeviceCount());
    Serial.println(F(" device(s)"));
  }
}

void loop() {}
