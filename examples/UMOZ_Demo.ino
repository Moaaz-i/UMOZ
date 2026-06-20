/*
 * UMOZ Library - Full Feature Example
 * ----------------------------------------------------------------------------
 * This example demonstrates the core features of the UMOZ library:
 * 1. Background Task Scheduling (addTask)
 * 2. Real-time CPU Usage Benchmarking
 * 3. Fast non-blocking Multi-Timers (UMOZ_EVERY)
 * 4. Zero-overhead Analog Filtering (smoothRead)
 * 5. Non-blocking Hardware Debouncing (isButtonPressed)
 * ----------------------------------------------------------------------------
 */

// #define UMOZ_DEBUG_MODE // Uncomment this line to enable UMOZ internal logging if needed
#include <UMOZ.h>

const uint8_t LED_PIN = 13;
const uint8_t BUTTON_PIN = 2;
const uint8_t SENSOR_PIN = A0;

// --- Background Task 1: Read and Filter Analog Sensor ---
void readSensorTask() {
  int filteredValue = tool.smoothRead(SENSOR_PIN);
  int percentage = tool.toPercentage(filteredValue, 0, 1023);

  Serial.print(F("[Task 1] Sensor Raw Filtered: "));
  Serial.print(filteredValue);
  Serial.print(F(" | Percentage: "));
  Serial.print(percentage);
  Serial.println(F("%"));
}

// --- Background Task 2: Monitor System CPU Load ---
void monitorCPUTask() {
  int cpuLoad = tool.getCPUUsage();
  if (cpuLoad != -1) {
    Serial.print(F("[Task 2] Real-time CPU Usage: "));
    Serial.print(cpuLoad);
    Serial.println(F("%"));
  }
}

UMOZ_START()
  // Initialize Serial communication
  Serial.begin(9600);
  while (!Serial); // Wait for serial port to connect (Targeting Leonardo/Micro boards)

  Serial.println(F("===================================="));
  Serial.println(F("     UMOZ Framework Initialized     "));
  Serial.println(F("===================================="));

  // Initialize Hardware Components
  tool.begin(LED_PIN);
  pinMode(BUTTON_PIN, INPUT);

  // Register Background Tasks to the Scheduler
  tool.addTask(readSensorTask, 500);  // Runs every 500ms
  tool.addTask(monitorCPUTask, 2000); // Runs every 2000ms

UMOZ_RUN()
  // 1. Independent Multi-Timer Example (Flashes LED every 1000ms)
  UMOZ_EVERY_MS(1000) {
    tool.toggle(LED_PIN);
    Serial.println(F("[Timer] LED Toggled via UMOZ_EVERY_MS"));
  }

  // 2. High-Frequency independent Timer Example (Runs at 1Hz / once per second)
  UMOZ_EVERY_HZ(1) {
    Serial.println(F("[Timer] Heartbeat ticking at 1 Hz..."));
  }

  // 3. Built-in Debounced Button Press Detection
  if (tool.isButtonPressed(BUTTON_PIN)) {
    Serial.println(F("[Event] Button pressed! (Hardware Debounced Successfully)"));
    // Add custom button action here
  }

UMOZ_END
