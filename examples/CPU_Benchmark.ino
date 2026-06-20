/*
 * UMOZ Library Example - Advanced Performance Benchmarking
 * ----------------------------------------------------------------------------
 * Description: Showcases the power of the UMOZ Micro-RTOS idle loop diagnostics.
 * This script injects a heavy dummy mathematical calculation task into the background
 * to demonstrate how the library accurately measures real-time CPU load shifts.
 * ----------------------------------------------------------------------------
 */

#include <UMOZ.h>

// --- Background Task: Simulating a heavy CPU load ---
void heavyCalculationTask() {
  volatile long dummy = 0;
  for (int i = 0; i < 500; i++) {
    dummy += i;
  }
}

UMOZ_START()
  Serial.begin(9600);

  // Schedule the heavy payload simulation task to trigger every 300ms
  tool.addTask(heavyCalculationTask, 300);

UMOZ_RUN()
  // Monitor and output the actual CPU load percentage every 1 second
  UMOZ_EVERY_MS(1000) {
    int load = tool.getCPUUsage();
    if (load != -1) {
      Serial.print(F("Current Micro-RTOS CPU Load: "));
      Serial.print(load);
      Serial.println(F("%"));
    }
  }

  // High-Frequency Macro Timer execution running at 10Hz (10 times per second)
  UMOZ_EVERY_HZ(10) {
    // Highly efficient fast-cycle routines can be placed here
  }
UMOZ_END
