/*
 * MicroTaskX Full Features Example
 *
 * This example demonstrates all major features of MicroTaskX:
 * - Dynamic task scheduling and management
 * - Priority-based task execution
 * - One-shot tasks
 * - Real-time CPU profiling
 * - Smart sleep modes
 * - Hardware utilities (button debouncing, analog smoothing, fast GPIO)
 * - Independent macro timers (MTX_EVERY_MS, MTX_EVERY_HZ)
 *
 * Hardware Requirements:
 * - Arduino board (AVR or ESP32)
 * - LED connected to pin 13
 * - Button connected to pin 2
 * - Analog sensor connected to pin A0
 * - Serial monitor at 115200 baud
 *
 * Author: Moaaz Yahia Shrif
 * License: MIT
 */

#include <MicroTaskX.h>

// Pin definitions
const uint8_t LED_PIN    = 13;
const uint8_t BUTTON_PIN = 2;
const uint8_t SENSOR_PIN = A0;

// Variables to hold history for independent analog smoothing
int sensorHistory = 0;

MTXButton btnState;

// ============================================================================
// TASK 1: Read and Filter Analog Sensor
// ============================================================================
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

// ============================================================================
// TASK 2: Monitor Real-Time CPU Usage
// ============================================================================
void monitorCPUTask() {
  int cpuLoad = mtx.getCPUUsage();
  if (cpuLoad != -1) {
    Serial.print(F("[Task 2] Real-time CPU Usage: "));
    Serial.print(cpuLoad);
    Serial.println(F("%"));
  }
}

// ============================================================================
// TASK 3: One-Shot Alert (Executes Only Once)
// ============================================================================
void alertTask() {
  Serial.println(F("[Task 3] ⚠️ Alert! This one-shot task runs ONLY ONCE after 5 seconds."));
}

// ============================================================================
// TASK 4: Display System Status
// ============================================================================
void statusTask() {
  Serial.println(F("╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║           MicroTaskX Status Report                    ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝"));
}

// ============================================================================
// TASK 5: Dynamic Control Task (Demonstrates Runtime Modifications)
// ============================================================================
void controlTask() {
  static uint8_t stage = 0;

  switch(stage) {
    case 0:
      Serial.println(F("\n[Control] Stage 1: Normal operation"));
      stage++;
      break;
    case 1:
      Serial.println(F("[Control] Stage 2: Increasing sensor reading frequency..."));
      mtx.setInterval(readSensorTask, 200);  // Change interval at runtime
      stage++;
      break;
    case 2:
      Serial.println(F("[Control] Stage 3: Pausing sensor task..."));
      mtx.pauseTask(readSensorTask);
      stage++;
      break;
    case 3:
      Serial.println(F("[Control] Stage 4: Resuming sensor task..."));
      mtx.resumeTask(readSensorTask);
      stage = 0;  // Reset cycle
      break;
  }
}

// ============================================================================
// SETUP: Initialize all tasks and configurations
// ============================================================================
MTX_START()
  // Initialize Serial Communication
  Serial.begin(115200);
  delay(500);

  Serial.println(F("\n╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║         MicroTaskX Full Features Example              ║"));
  Serial.println(F("║         Version 3.1.1                                 ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝\n"));

  // Configure pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  btnState.lastDebounceTime = 0;
  btnState.lastState = HIGH;
  btnState.isLongPressed = false;

  // ========================================================================
  // Register Recurring Tasks (with priorities: MTX_HIGH, MTX_MEDIUM, MTX_LOW)
  // ========================================================================

  // Task 1: Read sensor every 500ms (Medium Priority)
  mtx.addTask(readSensorTask, 500, MTX_MEDIUM);
  Serial.println(F("✓ Task 1 registered: Sensor reading (500ms)"));

  // Task 2: Monitor CPU every 2000ms (Low Priority)
  mtx.addTask(monitorCPUTask, 2000, MTX_LOW);
  Serial.println(F("✓ Task 2 registered: CPU monitoring (2000ms)"));

  // Task 4: Display status every 10000ms (Low Priority)
  mtx.addTask(statusTask, 10000, MTX_LOW);
  Serial.println(F("✓ Task 4 registered: Status display (10000ms)"));

  // Task 5: Control/Dynamic modification every 8000ms (Medium Priority)
  mtx.addTask(controlTask, 8000, MTX_MEDIUM);
  Serial.println(F("✓ Task 5 registered: Control task (8000ms)"));

  // ========================================================================
  // Register One-Shot Task (Executes once after 5000ms)
  // ========================================================================
  mtx.addOneShotTask(alertTask, 5000, MTX_HIGH);
  Serial.println(F("✓ Task 3 registered: One-shot alert (5000ms, HIGH priority)\n"));

  // ========================================================================
  // Enable Smart Low-Power Modes
  // ========================================================================
  mtx.enableSmartSleep(true);
  Serial.println(F("✓ Smart Sleep Mode: ENABLED\n"));

  Serial.println(F("════════════════════════════════════════════════════════"));
  Serial.println(F("Ready! Starting scheduler loop...\n"));

// ============================================================================
// MAIN LOOP: Independent Timers and Real-Time Event Handling
// ============================================================================
MTX_RUN()

  // ────────────────────────────────────────────────────────────────────────
  // Independent Timer 1: LED Toggle every 1000ms (1 Hz)
  // Uses zero-overhead token-pasting macro expansion
  // ────────────────────────────────────────────────────────────────────────
  MTX_EVERY_MS(1000) {
    MTXUtils::toggleFast<LED_PIN>();
    Serial.println(F("💡 [Timer 1] LED toggled via MTX_EVERY_MS (1000ms)"));
  }

  // ────────────────────────────────────────────────────────────────────────
  // Independent Timer 2: Heartbeat every 1 second (1 Hz)
  // ────────────────────────────────────────────────────────────────────────
  MTX_EVERY_HZ(1) {
    Serial.println(F("❤️  [Timer 2] Heartbeat at 1 Hz..."));
  }

  // ────────────────────────────────────────────────────────────────────────
  // Independent Timer 3: Frequent event every 100ms
  // ────────────────────────────────────────────────────────────────────────
  MTX_EVERY_MS(100) {
    // Fast, non-blocking operations here
    // This runs 10 times per second
  }

  // ────────────────────────────────────────────────────────────────────────
  // Hardware Button Monitoring (Non-Blocking with Debouncing)
  // ────────────────────────────────────────────────────────────────────────
  if (MTXUtils::isButtonPressed(BUTTON_PIN, btnState, 50)) {
    Serial.println(F("🔘 [Event] Button pressed! (Debounced)"));
  }

// ============================================================================
// End of Scheduler Loop
// ============================================================================
MTX_END
