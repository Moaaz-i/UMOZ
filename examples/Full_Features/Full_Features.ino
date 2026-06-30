/*
 * MicroTaskX Full Features Example
 */
#include "MicroTaskX.h"

// Pin definitions
const uint8_t LED_PIN = 13;
const uint8_t BUTTON_PIN = 2;
const uint8_t SENSOR_PIN = A0;

int sensorHistory = 0;
MTXButton btnState;

// ============================================================================
// TASKS (معدلة لتطابق الـ Kernel)
// ============================================================================
void readSensorTask(void* arg) {
  int filteredValue = MTXUtils::smoothReadFast(SENSOR_PIN, sensorHistory);
  int percentage = MTXUtils::toPercentage(filteredValue, 0, 1023);

  Serial.print(F("[Task 1] Sensor Value: "));
  Serial.print(filteredValue);
  Serial.print(F(" | Percentage: "));
  Serial.print(percentage);
  Serial.println(F("%"));
}

void monitorCPUTask(void* arg) {
  int cpuLoad = mtx.getCPUUsage();
  if (cpuLoad != -1) {
    Serial.print(F("[Task 2] Real-time CPU Usage: "));
    Serial.print(cpuLoad);
    Serial.println(F("%"));
  }
}

void alertTask(void* arg) {
  Serial.println(F("[Task 3] ⚠️ Alert! This one-shot task runs ONLY ONCE after 5 seconds."));
}

void statusTask(void* arg) {
  Serial.println(F("╔════════════════════════════════════════════════════════╗"));
  Serial.println(F("║           MicroTaskX Status Report                    ║"));
  Serial.println(F("╚════════════════════════════════════════════════════════╝"));
}

void controlTask(void* arg) {
  static uint8_t stage = 0;

  switch (stage) {
    case 0:
      Serial.println(F("\n[Control] Stage 1: Normal operation"));
      stage++;
      break;
    case 1:
      Serial.println(F("[Control] Stage 2: Increasing sensor reading frequency..."));
      mtx.setInterval(readSensorTask, 200);
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
      stage = 0;
      break;
  }
}

// ============================================================================
// SETUP
// ============================================================================
MTX_START()
Serial.begin(115200);
delay(500);

Serial.println(F("\n╔════════════════════════════════════════════════════════╗"));
Serial.println(F("║         MicroTaskX Full Features Example              ║"));
Serial.println(F("╚════════════════════════════════════════════════════════╝\n"));

pinMode(LED_PIN, OUTPUT);
pinMode(BUTTON_PIN, INPUT);

btnState.lastDebounceTime = 0;
btnState.lastState = HIGH;
btnState.isLongPressed = false;

// Register Tasks
mtx.addTask(readSensorTask, 500, MTX_MEDIUM);
mtx.addTask(monitorCPUTask, 2000, MTX_LOW);
mtx.addTask(statusTask, 10000, MTX_LOW);
mtx.addTask(controlTask, 8000, MTX_MEDIUM);

mtx.addOneShotTask(alertTask, 5000, MTX_HIGH);

mtx.enableSmartSleep(true);

Serial.println(F("Ready! Starting scheduler...\n"));
MTX_RUN()

// Independent Timers
MTX_EVERY_MS(1000) {
  MTXUtils::toggleFast<LED_PIN>();
  Serial.println(F("💡 [Timer] LED toggled"));
}

MTX_EVERY_HZ(1) {
  Serial.println(F("❤️ [Timer] Heartbeat"));
}

MTX_EVERY_MS(100) {
  // Fast operations here
}

if (MTXUtils::isButtonPressed(BUTTON_PIN, btnState, 50)) {
  Serial.println(F("🔘 Button pressed!"));
}

MTX_END
