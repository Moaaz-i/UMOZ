/*
 * MicroTaskX Library for Arduino
 * ----------------------------------------------------------------------------
 * Description: An ultra-lightweight, high-performance Arduino library designed
 * to simplify multitasking and optimize system resource management.
 * Version: 2.5.١ (Optimized Memory & High-Efficiency Edition)
 * ----------------------------------------------------------------------------
*/

#ifndef MICROTASKX_H
#define MICROTASKX_H

#include "Arduino.h"

#if defined(__AVR__)
  #include <avr/sleep.h>
  #include <avr/power.h>
#endif

#ifdef MTX_DEBUG_MODE
  #define MTX_PRINT(x)   Serial.print(x)
  #define MTX_PRINTLN(x) Serial.println(x)
#else
  #define MTX_PRINT(x)
  #define MTX_PRINTLN(x)
#endif

#ifndef MTX_MAX_TASKS
  #define MTX_MAX_TASKS 5
#endif

typedef void (*mtx_task_t)();

enum MTXPriority : uint8_t {
    MTX_LOW    = 0,
    MTX_MEDIUM = 1,
    MTX_HIGH   = 2
};

struct MTX_Task {
    void (*taskFunction)();
    uint32_t interval;
    uint32_t lastRun;
    uint8_t priority : 2;
    uint8_t isActive : 1;
};

class MicroTaskX {
  private:
    uint8_t _pin;
    uint32_t _previousMillisBlink;
    int _smoothedAnalog;

    uint32_t _idleCounter;
    uint32_t _maxIdle;
    uint32_t _cpuCheckMillis;

    MTX_Task _tasks[MTX_MAX_TASKS];
    uint8_t _taskCount;
    bool _smartSleepEnabled;

    MicroTaskX();
    MicroTaskX(const MicroTaskX&) = delete;
    MicroTaskX& operator=(const MicroTaskX&) = delete;

  public:
    static MicroTaskX& getInstance() {
      static MicroTaskX instance;
      return instance;
    }

    void begin(uint8_t pin);
    void blink(uint32_t delayTime);
    void initLibrary();
    int  getCPUUsage();

    bool addTask(mtx_task_t func, uint32_t intervalMs, MTXPriority priority = MTX_MEDIUM);
    void runTasks();

    void enableSmartSleep(bool enable) { _smartSleepEnabled = enable; }
    void enterLowPowerSleep();

    inline void benchTick() { _idleCounter++; }

    template <uint8_t PIN>
    inline void toggleFast() {
      #if defined(__AVR__)
        *portInputRegister(digitalPinToPort(PIN)) = digitalPinToBitMask(PIN);
      #else
        digitalWrite(PIN, !digitalRead(PIN));
      #endif
    }

    inline void toggle(uint8_t pin) {
      digitalWrite(pin, !digitalRead(pin));
    }

    template <uint8_t ANALOG_PIN>
    int smoothReadFast() {
      int newRead = analogRead(ANALOG_PIN);
      _smoothedAnalog = (_smoothedAnalog * 7 + newRead) >> 3;
      return _smoothedAnalog;
    }

    int smoothRead(uint8_t analogPin);

    bool isButtonPressed(uint8_t buttonPin, uint32_t &lastDebounceTime) {
      if (digitalRead(buttonPin) == HIGH) {
        if (millis() - lastDebounceTime >= 50) {
          lastDebounceTime = millis();
          return true;
        }
      }
      return false;
    }

    bool isButtonPressed(uint8_t buttonPin);

    inline int toPercentage(int rawValue, int minRaw, int maxRaw) {
      return constrain(map(rawValue, minRaw, maxRaw, 0, 100), 0, 100);
    }
};

#define mtx MicroTaskX::getInstance()

#define MTX_START() \
void setup(); \
void __mtx_internal_setup() { mtx.initLibrary(); } \
void setup() { __mtx_internal_setup();

#define MTX_RUN() \
} \
void loop() { \
  mtx.benchTick(); \
  mtx.runTasks();

#define _MTX_CONCAT_INNER(a, b) a ## b
#define _MTX_CONCAT(a, b) _MTX_CONCAT_INNER(a, b)

#define MTX_EVERY_MS(ms) \
  static uint32_t _MTX_CONCAT(_prev_time_, __LINE__) = 0; \
  if (millis() - _MTX_CONCAT(_prev_time_, __LINE__) >= (ms) && (_MTX_CONCAT(_prev_time_, __LINE__) = millis(), true))

#define MTX_EVERY_HZ(hz) MTX_EVERY_MS((hz) == 0 ? 0 : 1000 / (hz))
#define MTX_EVERY(ms)    MTX_EVERY_MS(ms)

#define MTX_END \
}
#endif
