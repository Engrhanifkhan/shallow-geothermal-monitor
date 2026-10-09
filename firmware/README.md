# ESP32 geothermal monitor (Adafruit IO)

Six DS18B20 sensors on **six separate GPIOs**. ESP32 reads each pin and sends values to [Adafruit IO](https://io.adafruit.com) every 20 seconds.

## Wiring (6 pins)

All sensors share **3.3 V** and **GND**. Each DATA wire goes to its own GPIO. Each DATA line needs its own **4.7 kΩ** pull-up to 3.3 V.

| Sensor | ESP32 DATA pin |
| --- | --- |
| Room | GPIO **4** |
| Outdoor | GPIO **5** |
| Inlet water | GPIO **18** |
| Outlet water | GPIO **19** |
| Pipe wall | GPIO **21** |
| Ground | GPIO **22** |

```
3.3V ----+--------+--------+--------+--------+-------- DS18B20 VDD (all 6)
         |        |        |        |        |
        4.7k     4.7k     4.7k     4.7k     4.7k     (one per DATA)
         |        |        |        |        |
       GPIO4    GPIO5    GPIO18   GPIO19   GPIO21   GPIO22
         |        |        |        |        |
        DATA     DATA     DATA     DATA     DATA     DATA
```

Change pins in `GeothermalMonitor/Config.h` (`kSensors[].pin`) if your board is different.

## Arduino IDE

1. Board: **ESP32 Dev Module**.
2. Libraries: OneWire, DallasTemperature, Adafruit IO Arduino.
3. Open `firmware/GeothermalMonitor/GeothermalMonitor.ino`.
4. Edit `Secrets.h` (Wi-Fi + Adafruit IO username and **My Key**).
5. Upload. Serial Monitor **115200**.

`AddressScanner.ino` prints how many devices are on each GPIO (expect 1 per pin).

## Auto-create feeds and dashboard

Do not send your Adafruit password in chat. After login at [io.adafruit.com](https://io.adafruit.com):

1. Copy username and AIO Key (My Key) into `GeothermalMonitor/Secrets.h`.
2. Run `python firmware/setup_adafruit_io.py`

Feeds: `room-temp`, `outdoor-temp`, `inlet-temp`, `outlet-temp`, `pipe-wall-temp`, `ground-temp`.

Dashboard: **Geothermal FYP** (6 gauges + 24 h chart). Phone: Adafruit IO app.

## Code modules

| File | Role |
| --- | --- |
| `Config.h` | Six pins, feed names, interval |
| `Secrets.h` | Wi-Fi and Adafruit IO key |
| `Ds18b20Bus` | One OneWire bus per pin |
| `AdafruitCloud` | MQTT publish |
| `GeothermalMonitor.ino` | Setup / loop |
