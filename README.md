Dựa trên những thay đổi lớn về kiến trúc mà chúng ta vừa thực hiện (chuyển sang mô hình **Monorepo**, tách **STM32** làm Sensor Node đọc Loadcell và **ESP32** làm Gateway xử lý DSP), đây là nội dung file `README.md` đã được cập nhật chuẩn kỹ thuật và thực tế nhất cho dự án của bạn.

Bạn có thể copy nội dung dưới đây đè vào file `README.md` ở thư mục gốc:

---

# Smart-Bed-IoT: Non-Invasive Health Monitoring System

**Smart-Bed-IoT** is a dual-MCU firmware implementation for a smart health monitoring bed. It utilizes a hybrid architecture combining **STM32** for high-precision sensor acquisition and **ESP32** for digital signal processing (DSP), IoT connectivity, and web hosting.

The system is designed to detect patient presence, determine sleeping posture, and monitor respiration rates without direct body contact, utilizing physics-based logic and signal analysis.

---

## 1. System Architecture

The system operates on a distributed processing model to optimize performance and modularity.

### Hardware Architecture

| Component | Device | Function |
| --- | --- | --- |
| **Gateway & DSP** | **ESP32-S3 DevKit** | Wi-Fi/WebSocket Server, Respiration DSP (MPU9250), Data aggregation. |
| **Sensor Node** | **STM32F4/F1 Series** | Real-time acquisition of 4x Load Cells (HX711), Raw data filtering, UART transmission. |
| **Sensors** | MPU-9250 (IMU) | Connected to ESP32 via I2C for vibration-based respiration monitoring. |
| **Sensors** | 50kg Load Cell (x4) | Connected to STM32 via HX711 for weight distribution analysis. |
| **Communication** | UART (Serial) | High-speed link between STM32 (TX) and ESP32 (RX). |

### Software Stack

* **ESP32 Firmware:** Built with **ESP-IDF / PlatformIO**. Handles FreeRTOS tasks, WebSocket telemetry, and Butterworth filtering for breathing detection.
* **STM32 Firmware:** Built with **Keil MDK & STM32 HAL**. Handles precise timing (DWT), HX711 bit-banging, and load cell calibration.

---

## 2. Directory Structure (Monorepo)

```text
Smart-Bed-IoT/
├── firmware_esp32/           # ESP32 Gateway Firmware (PlatformIO)
│   ├── src/
│   │   ├── main.c            # Task Scheduler & UART Parser
│   │   ├── dsp_filter.c      # Respiration Signal Processing
│   │   └── web_server.c      # WebSocket Handler
│   ├── include/
│   └── platformio.ini
│
├── firmware_stm32/           # STM32 Sensor Node Firmware (Keil MDK)
│   ├── MDK-ARM/              # Keil Project Files
│   ├── Core/Src/main.c       # Sensor Loop & Data transmission
│   ├── drv_loadcell/         # HX711 Driver (Custom HAL implementation)
│   └── ...
├── .gitignore
└── README.md                 # Project Documentation

```

---

## 3. Functional Logic

### A. Weight & Posture (STM32 Node)

* **Data Acquisition:** The STM32 polls 4 separate HX711 modules using DWT-based microsecond delays for timing accuracy.
* **Preprocessing:** Applies a Moving Average Filter to stabilize weight readings.
* **Transmission:** Sends a formatted packet (JSON or Binary) to the ESP32 via UART containing raw weight data from 4 quadrants (Front-Left, Front-Right, Back-Left, Back-Right).

### B. Signal Processing & Connectivity (ESP32 Gateway)

* **Presence Detection:** Aggregates total weight received from STM32. If Total > Threshold, the system enters ACTIVE mode.
* **Posture Recognition:** Calculates the **Center of Gravity (CoG)** based on the differential weight distribution to classify posture: *Supine, Left-Lateral, Right-Lateral*.
* **Respiration Monitoring:** Reads the MPU-9250 Z-axis accelerometer data at 100Hz. Applies a Low-Pass Butterworth Filter (Cutoff ~1Hz) to isolate breathing movements from noise.

---

## 4. Key Technical Highlights

1. **Hybrid Processing:** Offloading the "bit-banging" intensive task of reading 4 Load Cells to the STM32 ensures the ESP32's CPU is free for heavy Wi-Fi and DSP operations.
2. **Robust Driver Implementation:** Custom `drv_loadcell` for STM32 using DWT Cycle Counter ensures precise timing for the HX711 protocol, unaffected by HAL overhead.
3. **Real-time Telemetry:** Uses WebSockets to push sensor data to a client dashboard with low latency (< 200ms), avoiding the overhead of HTTP polling.

---

## 5. Getting Started

### Hardware Wiring

* **Inter-Chip Connection:**
* STM32 TX pin  ESP32 RX pin.
* GND  GND (Common Ground is mandatory).


* **Sensors:**
* Load Cells  STM32 GPIOs (Configured in CubeMX).
* MPU-9250  ESP32 I2C Pins.



### Build & Flash

**1. STM32 Node:**

* Open `firmware_stm32/MDK-ARM/test.uvprojx` in **Keil uVision**.
* Build (F7) and Download (F8) to the STM32 board.

**2. ESP32 Gateway:**

* Open `firmware_esp32` in **VS Code** (with PlatformIO extension).
* Connect ESP32 via USB.
* Run: `PlatformIO: Upload` and `PlatformIO: Monitor`.

---

## Disclaimer

This project is a prototype for educational and research purposes. It is **not** a certified medical device and should not be used as a replacement for professional medical equipment in critical care scenarios.

## Author

**Thanh Tung Bui** - [buitung161@gmail.com](mailto:buitung161@gmail.com)