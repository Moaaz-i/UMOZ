# MicroTaskX 🚀

An ultra-lightweight, high-performance multitasking framework and micro-RTOS designed for Arduino.
MicroTaskX maximizes CPU efficiency by eliminating blocking delays, introducing independent macro-based timers, an advanced background task scheduler, and real-time CPU load profiling.

## ✨ Features

- **Micro-RTOS Task Scheduler:** Register background tasks using function pointers without dynamic memory allocation (RAM safe).
- **Independent Macro Timers (`MTX_EVERY`):** Create non-blocking independent intervals easily on any line using token-pasting macro expansions.
- **Real-time CPU Profiling:** Monitor actual CPU load percentage dynamically via free idle-loop calculation.
- **Turbo ADC Speed:** Injects hardware register tweaks to speed up analog readings by up to 8x on supported AVR boards.
- **Direct Port Manipulation:** High-speed `toggleFast()` routine that executes in a single clock cycle on AVR microcontrollers.
- **Zero-Overhead Smoothing:** Built-in Exponential Moving Average (EMA) filter for analog inputs to keep the loop fluid and non-blocking.
- **Hardware Switch Debouncing:** Built-in non-blocking button press stabilization.

## 🛠 Quick Start

```cpp
#include <MicroTaskX.h>

const uint8_t LED_PIN    = 13;
const uint8_t BUTTON_PIN = 2;
const uint8_t SENSOR_PIN = A0;

// Background Task 1: Read and filter analog sensor
void readSensorTask() {
  int filteredValue = mtx.smoothRead(SENSOR_PIN);
  int percentage    = mtx.toPercentage(filteredValue, 0, 1023);

  Serial.print(F("[Task 1] Sensor Value: "));
  Serial.print(filteredValue);
  Serial.print(F(" | Percentage: "));
  Serial.print(percentage);
  Serial.println(F("%"));
}

// Background Task 2: Monitor CPU load
void monitorCPUTask() {
  int cpuLoad = mtx.getCPUUsage();
  if (cpuLoad != -1) {
    Serial.print(F("[Task 2] Real-time CPU Usage: "));
    Serial.print(cpuLoad);
    Serial.println(F("%"));
  }
}

MTX_START()
  Serial.begin(9600);

  mtx.begin(LED_PIN);
  pinMode(BUTTON_PIN, INPUT);

  mtx.addTask(readSensorTask,   500);   // every 500ms
  mtx.addTask(monitorCPUTask,  2000);   // every 2000ms

MTX_RUN()
  MTX_EVERY_MS(1000) {
    mtx.toggle(LED_PIN);
    Serial.println(F("[Timer] LED toggled via MTX_EVERY_MS"));
  }

  MTX_EVERY_HZ(1) {
    Serial.println(F("[Timer] Heartbeat at 1 Hz..."));
  }

  if (mtx.isButtonPressed(BUTTON_PIN)) {
    Serial.println(F("[Event] Button pressed! (Debounced)"));
  }

MTX_END
```
