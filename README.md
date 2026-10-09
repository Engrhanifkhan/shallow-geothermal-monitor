# Design and Development of a Low-Cost Modular Shallow-Geothermal Heat Exchanger (HGHE)

**Energy-Efficient Building Cooling and Heating Telemetry System**  
*Engineered by Engr. Hanif Khan*

---

## 📖 Project Overview

This repository contains the embedded firmware, cloud telemetry integration, and real-time visualization dashboard for a modular **Horizontal Ground Heat Exchanger (HGHE)**. The system utilizes an HDPE slinky coil buried at a depth of 2.5 meters to evaluate ground heat dissipation and extraction:

$$Q_{\text{water}} = \dot{m} \cdot C_p \cdot (T_{\text{inlet}} - T_{\text{outlet}})$$

An **ESP32 Dev Module** continuously monitors 6 high-precision digital temperature points, computes the thermal differential ($\Delta T$), and streams live telemetry to **Adafruit IO** cloud over Wi-Fi.

---

## 🗂️ Repository Structure

```text
├── firmware/
│   ├── GeothermalMonitor/           # Primary ESP32 firmware
│   │   ├── GeothermalMonitor.ino    # Main application sketch
│   │   ├── Config.h                 # Sensor pin mapping & publish interval
│   │   ├── Ds18b20Bus.h / .cpp      # Multi-bus Dallas DS18B20 driver
│   │   ├── AdafruitCloud.h / .cpp   # Wi-Fi & Adafruit IO MQTT telemetry client
│   │   ├── Secrets.h.example        # Credentials template
│   ├── AddressScanner/              # Diagnostic utility to verify sensor wiring
│   │   └── AddressScanner.ino
│   ├── setup_adafruit_io.py         # Python cloud provisioning script
│   └── setup_pro_dashboard.ps1      # PowerShell cloud dashboard generator
├── web_dashboard/                   # Standalone interactive executive dashboard
│   └── index.html                   # Interactive browser UI with SVG schematic & CSV export
└── README.md                        # Documentation
```

---

## 🔌 Hardware Architecture & Wiring

All six Dallas DS18B20 temperature sensors share common **3.3 V** and **GND** rails. Each sensor's DATA line connects to its own dedicated ESP32 GPIO with an individual **4.7 kΩ pull-up resistor** to 3.3 V.

| Sensor Name | Function / Placement | ESP32 GPIO | Feed Key |
| :--- | :--- | :--- | :--- |
| **Room** | Conditioned Indoor Air | **GPIO 4** | `room-temp` |
| **Outdoor** | Ambient Atmospheric Air | **GPIO 5** | `outdoor-temp` |
| **Inlet** | Water entering ground slinky coil | **GPIO 18** | `inlet-temp` |
| **Outlet** | Water exiting ground slinky coil | **GPIO 19** | `outlet-temp` |
| **Pipe Wall** | HDPE Pipe-Soil Boundary Interface | **GPIO 21** | `pipe-wall-temp` |
| **Ground** | Undisturbed Deep Soil (2.5 m depth) | **GPIO 22** | `ground-temp` |
| **$\Delta T$** | Thermal Drop ($T_{\text{in}} - T_{\text{out}}$) | *Calculated* | `delta-t` |

```
3.3V ----+--------+--------+--------+--------+-------- DS18B20 VDD (all 6)
         |        |        |        |        |
        4.7k     4.7k     4.7k     4.7k     4.7k     (one per DATA line)
         |        |        |        |        |
       GPIO4    GPIO5    GPIO18   GPIO19   GPIO21   GPIO22
         |        |        |        |        |
        DATA     DATA     DATA     DATA     DATA     DATA
```

---

## 🚀 Getting Started

### 1. Arduino IDE Setup
1. Install **ESP32 Board Support** in Arduino IDE (Boards Manager).
2. Install required libraries from Library Manager (`Ctrl + Shift + I`):
   - `OneWire` (by Paul Stoffregen)
   - `DallasTemperature` (by Miles Burton)
   - `Adafruit IO Arduino` (by Adafruit) — click *"Install All"* for dependencies.
3. Copy `firmware/GeothermalMonitor/Secrets.h.example` to `firmware/GeothermalMonitor/Secrets.h` and fill in your Wi-Fi and Adafruit IO credentials.
4. Select board **ESP32 Dev Module**, select your COM port, and upload `GeothermalMonitor.ino`.
5. Open Serial Monitor at **115200 baud**.

### 2. Cloud Dashboard
- Feeds and dashboard widgets can be generated on Adafruit IO automatically by running:
  ```powershell
  powershell -ExecutionPolicy Bypass -File firmware/setup_pro_dashboard.ps1
  ```

### 3. Executive Web Dashboard
- Open `web_dashboard/index.html` in any web browser.
- Displays a cross-sectional SVG schematic of the 2.5m trench, real-time KPI metrics, dynamic heat dissipation rate ($Q$), Chart.js trends, and one-click **CSV data export** for research data analysis.

---

## 👨‍💻 System Engineering & Lead Developer

This complete monitoring system, embedded telemetry firmware, Adafruit IO cloud infrastructure, and interactive research dashboard was engineered and developed by:

### **Engr. Hanif Khan**
*Lead Embedded Systems, IoT & Firmware Development Engineer*

* 📧 **Email:** [Hanifullahshams@gmail.com](mailto:Hanifullahshams@gmail.com)
* 🌐 **LinkedIn:** [Engr. Hanif Khan](https://www.linkedin.com/in/engr-hanif-khan-628a04259/)
* 🐙 **GitHub:** [@Engrhanifkhan](https://github.com/Engrhanifkhan)
* 💬 **WhatsApp / Contact:** [+92 311 8221686](https://wa.me/923118221686)

#### Engineering Contributions & Scope:
- **Hardware Architecture:** Designed 6-channel dedicated GPIO DS18B20 multi-bus circuitry with individual 4.7 kΩ pull-up networks for maximum reliability and fault isolation.
- **Embedded Firmware:** Developed modular ESP32 C++ firmware featuring non-blocking 2.4 GHz auto-scanning, diagnostic fail-safes, and thermal differential ($\Delta T$) calculation.
- **Cloud Infrastructure:** Engineered automated Adafruit IO provisioning scripts and dual-chart MQTT telemetry pipelines.
- **Executive Visualizer:** Developed custom responsive Web Dashboard with cross-sectional SVG subsurface rendering, dynamic heat transfer computation ($Q$), and Excel CSV data logging.

---

## 📜 License
Developed and maintained by **Engr. Hanif Khan**. All rights reserved.

