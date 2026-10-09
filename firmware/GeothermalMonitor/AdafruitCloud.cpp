#include "AdafruitCloud.h"

AdafruitCloud::AdafruitCloud()
    : io_(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS), deltaFeed_(nullptr) {
  for (uint8_t i = 0; i < kSensorCount; i++) {
    feeds_[i] = nullptr;
  }
}

void AdafruitCloud::connect() {
  // Step 1: Clean Wi-Fi state & disable power sleep
  WiFi.disconnect(true);
  delay(300);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false); // Prevents ESP32 radio from sleeping during connection

  Serial.println();
  Serial.print(F("[Wi-Fi] Target SSID: \""));
  Serial.print(WIFI_SSID);
  Serial.println(F("\""));

  // Check if the phone hotspot is broadcasting on 2.4 GHz
  Serial.println(F("[Wi-Fi] Scanning for 2.4 GHz broadcast..."));
  int n = WiFi.scanNetworks(false, false, false, 300);
  bool found = false;
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == WIFI_SSID) {
      found = true;
      Serial.print(F("  [OK] Found \""));
      Serial.print(WIFI_SSID);
      Serial.print(F("\" with signal: "));
      Serial.print(WiFi.RSSI(i));
      Serial.println(F(" dBm"));
      break;
    }
  }

  if (!found) {
    Serial.println(F("  [!] WARNING: \"Geothermal\" was NOT detected in 2.4 GHz scan!"));
    if (n > 0) {
      Serial.println(F("  Networks detected nearby:"));
      for (int i = 0; i < n; i++) {
        Serial.print(F("    - \""));
        Serial.print(WiFi.SSID(i));
        Serial.print(F("\" ("));
        Serial.print(WiFi.RSSI(i));
        Serial.println(F(" dBm)"));
      }
    } else {
      Serial.println(F("  No 2.4 GHz networks found at all."));
    }
    Serial.println(F("  Troubleshooting steps:"));
    Serial.println(F("  1. Hotspot AP Band MUST be set to 2.4 GHz (not 5.0 GHz)."));
    Serial.println(F("  2. Check spelling and capital 'G' in \"Geothermal\"."));
    Serial.println(F("  3. Ensure Hotspot is ON and Security is WPA2-Personal."));
  }

  // Attempt connection
  Serial.print(F("[Wi-Fi] Connecting"));
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print('.');
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println(F("[Wi-Fi] Connected successfully!"));
    Serial.print(F("[Wi-Fi] Assigned IP: "));
    Serial.println(WiFi.localIP());
    Serial.print(F("[Wi-Fi] Signal Strength (RSSI): "));
    Serial.print(WiFi.RSSI());
    Serial.println(F(" dBm"));
  } else {
    Serial.println();
    Serial.print(F("[Wi-Fi] Connection failed! Error code: "));
    wl_status_t st = WiFi.status();
    Serial.println(st);
    if (st == WL_NO_SSID_AVAIL) {
      Serial.println(F("  -> Cause: SSID not found. Hotspot is either OFF, 5 GHz, or hidden."));
    } else if (st == WL_CONNECT_FAILED) {
      Serial.println(F("  -> Cause: Authentication failed. Check password or change Security to WPA2."));
    } else {
      Serial.println(F("  -> Cause: Timeout. Restart phone hotspot and try again."));
    }
    return;
  }

  // Step 2: Connect to Adafruit IO now that Wi-Fi is ready
  Serial.println(F("[Adafruit IO] Connecting to io.adafruit.com..."));
  io_.connect();

  unsigned long started = millis();
  while (io_.status() < AIO_CONNECTED) {
    Serial.print('.');
    delay(500);
    if (millis() - started > 30000) {
      Serial.println(F("\n[Adafruit IO] Timeout! Will retry in background loop."));
      return;
    }
  }

  Serial.println();
  Serial.println(F("[Adafruit IO] Connected successfully!"));
  Serial.println(io_.statusText());

  for (uint8_t i = 0; i < kSensorCount; i++) {
    feeds_[i] = io_.feed(kSensors[i].feedKey);
  }
  deltaFeed_ = io_.feed("delta-t");
}

void AdafruitCloud::run() {
  // Guard: Ensure Wi-Fi is active first
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastWifiRetry = 0;
    if (millis() - lastWifiRetry > 10000) {
      lastWifiRetry = millis();
      Serial.println(F("[Wi-Fi] Connection lost! Reconnecting to Wi-Fi..."));
      WiFi.reconnect();
    }
    return;
  }

  io_.run();

  if (io_.status() < AIO_CONNECTED) {
    static unsigned long lastTry = 0;
    if (millis() - lastTry > 10000) {
      lastTry = millis();
      Serial.println(F("[Adafruit IO] Reconnecting to Adafruit IO..."));
      io_.connect();
      if (io_.status() >= AIO_CONNECTED) {
        for (uint8_t i = 0; i < kSensorCount; i++) {
          feeds_[i] = io_.feed(kSensors[i].feedKey);
        }
        deltaFeed_ = io_.feed("delta-t");
      }
    }
  }
}

bool AdafruitCloud::isConnected() {
  return (WiFi.status() == WL_CONNECTED) && (io_.status() >= AIO_CONNECTED);
}

void AdafruitCloud::publish(const SensorReading *readings) {
  if (!isConnected()) {
    Serial.println(F("[Adafruit IO] skip publish (offline)"));
    return;
  }

  for (uint8_t i = 0; i < kSensorCount; i++) {
    if (feeds_[i] == nullptr) {
      continue;
    }
    if (!readings[i].valid) {
      Serial.print(F("[Adafruit IO] skip invalid "));
      Serial.println(readings[i].name);
      continue;
    }

    bool ok = feeds_[i]->save(readings[i].celsius);
    Serial.print(F("[Adafruit IO] "));
    Serial.print(readings[i].feedKey);
    Serial.print(F(" = "));
    Serial.print(readings[i].celsius, 2);
    Serial.println(ok ? F(" C  ok") : F(" C  FAIL"));
  }

  // Publish thermal drop delta-t (Tinlet - Toutlet)
  if (deltaFeed_ != nullptr && readings[kInlet].valid && readings[kOutlet].valid) {
    float deltaT = readings[kInlet].celsius - readings[kOutlet].celsius;
    bool ok = deltaFeed_->save(deltaT);
    Serial.print(F("[Adafruit IO] delta-t = "));
    Serial.print(deltaT, 2);
    Serial.println(ok ? F(" C  ok") : F(" C  FAIL"));
  }
}

