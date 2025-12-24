# ESP32-SHMB: Smart Health Monitoring Bed

**ESP32-SHMB** (Smart Health Monitoring Bed) is the firmware implementation for a non-invasive patient monitoring system. Built on the **ESP32** platform using **ESP-IDF**, it utilizes digital signal processing (DSP) to detect respiration rates and physics-based logic to determine patient posture without direct body contact.

---

## 1. System Architecture

The system uses a layered architecture separating hardware drivers, signal processing algorithms, and communication layers.

### Hardware Specifications

| Component | Model | Interface | Purpose |
| --- | --- | --- | --- |
| **MCU** | ESP32-S3 DevKit | - | Central Processing Unit & Web Server |
| **IMU** | MPU-9250 | I2C | Respiration monitoring (Vibration analysis) |
| **ADC** | HX711 (x4) | GPIO | 24-bit interface for Load Cells |
| **Sensors** | 5kg Load Cell (x4) | Analog | Weight distribution & Posture detection |
| **Camera** | ESP32-CAM | WiFi | Visual verification (Fall Detection) |
| **Alert** | Active Buzzer | GPIO | Local emergency alarm |

### Software Stack

* **Framework:** ESP-IDF v5.5 (FreeRTOS based)
* **Communication:** WebSocket over SoftAP (Low latency telemetry), JSON
* **Algorithms:**
* **Digital Signal Processing (DSP):** Low-pass filtering (Butterworth) and Peak Detection to extract breathing signals from noise.
* **Center of Gravity (CoG):** Differential weight calculation to determine lying posture.
* **Finite State Machine:** Managing patient states (Absent, Sleeping, Agitated, Emergency).



---

## 2. Directory Structure

```text
ESP32-SHMB/
├── include/
│   ├── app_config.h        # Pin mapping & Threshold constants
│   ├── communication.h     # UART & WebSocket handlers
│   ├── drv_loadcell.h      # HX711 Driver & CoG Logic
│   └── drv_mpu9250.h       # IMU Driver & DSP Filtering
├── src/
│   ├── main.c              # FreeRTOS Task Scheduler & FSM
│   ├── communication.c     # Data serialization & Transmission
│   ├── drv_loadcell.c      # Weight acquisition & Calibration
│   └── drv_mpu9250.c       # Signal acquisition & Processing
├── webpage/
│   └── index.html          # Embedded Dashboard
└── CMakeLists.txt          # Build configuration

```

---

## 3. Functional Description (Finite State Machine)

The system operates on a state-based logic to ensure accurate monitoring:

1. **IDLE:** System standby. Calibrates sensors when weight is near zero.
2. **PRESENCE_DETECT:** Triggered when total weight exceeds threshold (e.g., > 10kg). Transitions system to active monitoring.
3. **POSTURE_TRACKING:** continuously calculates the Center of Gravity (CoG) based on the four load cell quadrants to determine if the patient is Supine, Left-Lateral, or Right-Lateral.
4. **VITAL_MONITORING:** High-frequency sampling (100Hz) of the IMU Z-axis. Applies filtering to extract respiration rate (RPM).
5. **EMERGENCY_ALERT:** Triggered by specific conditions:
* **Fall Detection:** Sudden drop in weight to zero combined with specific visual cues.
* **Apnea:** Weight is present, but respiration signal is flat for > 10 seconds.
* **Prolonged Inactivity:** Posture remains unchanged for > 2 hours (Bedsore prevention).



---

## 4. Key Technical Challenges & Solutions

### A. Extracting Weak Respiration Signals

The breathing signal (0.2 - 0.5 Hz) is often drowned out by environmental noise and sensor jitter.

* **Solution:** Implemented a software **Low-Pass Filter (Cutoff 1Hz)** to remove high-frequency noise, followed by a **Moving Average Filter** to smooth the waveform for reliable peak counting.

### B. Determining Posture with Limited Sensors

Detecting patient orientation without cameras or wearable trackers.

* **Solution:** Implemented a **Center of Gravity (CoG)** algorithm. By analyzing the differential weight distribution between the left and right load cell pairs, the system mathematically determines the center of mass relative to the bed's geometry.

### C. Real-time Visualization

UART is insufficient for remote monitoring, and standard HTTP requests are too slow for live graphing.

* **Solution:** Developed an **Asynchronous Web Server** with **WebSocket** support. This allows pushing JSON telemetry packets to the client dashboard with < 200ms latency, enabling smooth graph rendering.

---

## 5. Getting Started

### Wiring

| ESP32 Pin | Component | Function |
| --- | --- | --- |
| GPIO 21 | MPU-9250 SDA | I2C Data |
| GPIO 22 | MPU-9250 SCL | I2C Clock |
| GPIO 12 | HX711 (TL) DOUT | Data Input (Top-Left) |
| GPIO 11 | HX711 (TL) SCK | Clock (Top-Left) |
| ... | ... | Check `app_config.h` for full map |

### Build & Flash

1. Clone the repository:
```bash
git clone https://github.com/justargldude/Smart-Bed-IoT.git

```


2. Configure project settings (WiFi credentials, Pins) in `app_config.h`.
3. Build and Flash:
```bash
idf.py build
idf.py -p COMx flash monitor

```



---

## Disclaimer

This project is a prototype for educational and research purposes. It is **not** a certified medical device and should not be used as a replacement for professional medical equipment in critical care scenarios.

## Author

**Thanh Tung Bui** - [buitung161@gmail.com](mailto:buitung161@gmail.com)