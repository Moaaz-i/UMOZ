# MicroTaskX 🚀

An ultra-lightweight, high-performance micro-RTOS and multitasking framework designed for Arduino (AVR & ESP32 architectures). **MicroTaskX** maximizes CPU efficiency by eliminating blocking delays, introducing independent macro-based timers, an advanced background task scheduler with dynamic controls, and real-time CPU load profiling.

---

## ✨ Features

- **Zero-Overlap Modular Architecture:** Separates the core scheduling kernel (`MTXKernel`) from hardware utilities (`MTXUtils`) to minimize memory consumption.
- **Dynamic Compile-Time Configuration:** Uses C++ templates to customize maximum task bounds (`MicroTaskXKernel<MAX_TASKS>`) at compile time.
- **Runtime Task Management:** Built-in APIs to pause, resume, and modify task intervals dynamically during execution.
- **One-Shot Task Support:** Schedule delayed actions that execute exactly once before automatic deactivation.
- **Smart Sleep Integration:** Automatic power management (Idle Sleep for AVR, precise Light Sleep for ESP32) mapped to the timing of the next scheduled task.
- **Independent Macro Timers (`MTX_EVERY`):** Create non-blocking independent intervals easily on any line using token-pasting macro expansions.
- **Real-time CPU Profiling:** Monitor actual CPU load percentage dynamically via free idle-loop calculation.
- **Cross-Talk-Free Smoothing:** Upgraded Exponential Moving Average (EMA) filter that uses variable references to safely handle multiple isolated analog pins.
- **Direct Port Manipulation:** High-speed `toggleFast<PIN>()` template routine that executes in a single clock cycle on AVR microcontrollers.
- **Hardware Switch Debouncing:** Built-in non-blocking button press stabilization.

---

## 📦 Project Structure

````text
MicroTaskX/
├── src/
│   ├── MicroTaskX.h         // Main inclusion header (Creates default 'mtx' instance)
│   ├── MTXKernel.h          // Micro-kernel and Scheduler (Template-based)
│   ├── MTXUtils.h           // Hardware Utilities header
│   └── MTXUtils.cpp         // Hardware Utilities implementation
├── examples/
│   └── Full_Features/       // Practical usage example
└── library.properties       // Arduino IDE metadata

## 🛠 Usage Example
Here is a complete example demonstrating the new modular syntax, dynamic controls, and isolated analog smoothing:
````

```cpp
#include <MicroTaskX.h>

const uint8_t LED_PIN    = 13;
const uint8_t BUTTON_PIN = 2;
const uint8_t SENSOR_PIN = A0;

// Variables to hold the history for independent analog smoothing
int sensorHistory = 0;

// Background Task 1: Read and filter analog sensor
void readSensorTask() {
  // Pass the history variable as a reference to keep this reading isolated
  int filteredValue = MTXUtils::smoothReadFast(SENSOR_PIN, sensorHistory);
  int percentage    = MTXUtils::toPercentage(filteredValue, 0, 1023);

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

// Background Task 3: One-Shot alert
void alertTask() {
  Serial.println(F("[Task 3] Alert! This runs ONLY ONCE after 5 seconds."));
}

MTX_START()
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT);

  // Registering recurring tasks
  mtx.addTask(readSensorTask,   500, MTX_MEDIUM); // every 500ms
  mtx.addTask(monitorCPUTask,  2000, MTX_LOW);    // every 2000ms

  // Registering a One-Shot task (Executes once after 5000ms)
  mtx.addOneShotTask(alertTask, 5000, MTX_HIGH);

  // Enable smart low-power modes (Automatically manages AVR & ESP32)
  mtx.enableSmartSleep(true);

MTX_RUN()
  // High-speed compile-time hardware toggle
  MTX_EVERY_MS(1000) {
    MTXUtils::toggleFast<LED_PIN>();
    Serial.println(F("[Timer] LED toggled via MTX_EVERY_MS"));
  }

  MTX_EVERY_HZ(1) {
    Serial.println(F("[Timer] Heartbeat at 1 Hz..."));
  }

  // Monitor hardware buttons
  if (MTXUtils::isButtonPressed(BUTTON_PIN)) {
    Serial.println(F("[Event] Button pressed! (Debounced)"));
  }

  // Dynamic Runtime Modification
  MTX_EVERY(10000) {
    Serial.println(F("[Control] Speeding up Sensor Task to 100ms..."));
    mtx.setInterval(readSensorTask, 100);
  }

  MTX_EVERY(15000) {
    Serial.println(F("[Control] Pausing Sensor Task completely..."));
    mtx.pauseTask(readSensorTask);
  }
MTX_END
````

## ⚠️ Important Migration Notes from v2.x

- **MTX_END Syntax Change:** The `MTX_END` macro no longer takes parenthesis (). Writing `MTX_END()` will cause a compilation error.
- **Isolated smoothRead Functions:** To avoid data cross-talk between multiple analog inputs, `smoothRead` and `smoothReadFast` now require passing an `int` variable by reference to store the reading's specific history.
- **Static Utilities:** Hardware utility functions like `toggleFast`, `smoothReadFast`, and `isButtonPressed` are now grouped inside the static class `MTXUtils`, removing unnecessary instance overhead.

---

## 📜 License

This project is licensed under the MIT License - see the LICENSE file for details.
