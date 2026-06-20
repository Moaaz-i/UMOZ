/*
 * UMOZ Library for Arduino
 * ----------------------------------------------------------------------------
 * Description: An ultra-lightweight, high-performance Arduino library designed
 * to simplify multitasking and optimize system resource management.
 * * Key Features:
 * - Non-blocking Multi-Timers using unique macro-line expansion.
 * - Non-blocking Background Task Scheduler via function pointers.
 * - Real-time CPU Usage Benchmarking (Idle Loop Calculation).
 * - High-speed digital toggle utilizing Direct Port Manipulation (AVR).
 * - Zero-overhead Analog Filtering via Exponential Moving Average (EMA).
 * - Built-in hardware switch debouncing and zero-cost debug logging.
 * - Memory-efficient Architecture utilizing the Singleton design pattern.
 * ----------------------------------------------------------------------------
 */

#ifndef UMOZ_h
#define UMOZ_h

#include "Arduino.h"

#ifdef UMOZ_DEBUG_MODE
  #define UMOZ_PRINT(x) Serial.print(x)
  #define UMOZ_PRINTLN(x) Serial.println(x)
#else
  #define UMOZ_PRINT(x)
  #define UMOZ_PRINTLN(x)
#endif

#define UMOZ_MAX_TASKS 5

typedef void (*umoz_task_t)();

struct UMOZ_Task {
  umoz_task_t func;
  uint32_t interval;
  uint32_t lastRun;
  bool active;
};

class UMOZ {
  private:
    uint8_t _pin;
    uint32_t _previousMillisBlink;
    uint32_t _previousMillisButton;
    int _smoothedAnalog;

    uint32_t _idleCounter;
    uint32_t _maxIdle;
    uint32_t _cpuCheckMillis;

    UMOZ_Task _tasks[UMOZ_MAX_TASKS];
    uint8_t _taskCount;

    UMOZ();
    UMOZ(const UMOZ&) = delete;
    UMOZ& operator=(const UMOZ&) = delete;

  public:
    static UMOZ& getInstance() {
      static UMOZ instance;
      return instance;
    }

    void begin(uint8_t pin);
    void blink(uint32_t delayTime);
    int smoothRead(uint8_t analogPin);
    bool isButtonPressed(uint8_t buttonPin);
    void initLibrary();
    int getCPUUsage();

    bool addTask(umoz_task_t func, uint32_t intervalMs);
    void runTasks();

    inline void benchTick() { _idleCounter++; }

    inline void toggle(uint8_t pin) {
      #if defined(__AVR__)
        *portInputRegister(digitalPinToPort(pin)) = digitalPinToBitMask(pin);
      #else
        digitalWrite(pin, !digitalRead(pin));
      #endif
    }

    inline int toPercentage(int rawValue, int minRaw, int maxRaw) {
      return constrain(map(rawValue, minRaw, maxRaw, 0, 100), 0, 100);
    }
};

#define tool UMOZ::getInstance()

#define UMOZ_START() \
void setup(); \
void __umoz_internal_setup() { tool.initLibrary(); } \
void setup() { __umoz_internal_setup();

#define UMOZ_RUN() \
} \
void loop() { \
  tool.benchTick(); \
  tool.runTasks();

#define _UMOZ_CONCAT_INNER(a, b) a ## b
#define _UMOZ_CONCAT(a, b) _UMOZ_CONCAT_INNER(a, b)

#define UMOZ_EVERY_MS(ms) \
  static uint32_t _UMOZ_CONCAT(_prev_time_, __LINE__) = 0; \
  if (millis() - _UMOZ_CONCAT(_prev_time_, __LINE__) >= (ms) && (_UMOZ_CONCAT(_prev_time_, __LINE__) = millis(), true))

#define UMOZ_EVERY_HZ(hz) UMOZ_EVERY_MS((hz) == 0 ? 0 : 1000 / (hz))
#define UMOZ_EVERY(ms) UMOZ_EVERY_MS(ms)

#define UMOZ_END \
}
#endif
