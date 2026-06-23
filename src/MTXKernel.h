#ifndef MTX_KERNEL_H
#define MTX_KERNEL_H

#include "Arduino.h"

#if defined(__AVR__)
  #include <avr/sleep.h>
  #include <avr/power.h>
#elif defined(ARDUINO_ARCH_ESP32)
  #include "esp_sleep.h"
#endif

#ifdef MTX_DEBUG_MODE
  #define MTX_PRINT(x)   Serial.print(x)
  #define MTX_PRINTLN(x) Serial.println(x)
#else
  #define MTX_PRINT(x)
  #define MTX_PRINTLN(x)
#endif

typedef void (*mtx_task_t)(void*);

enum MTXPriority : uint8_t {
    MTX_LOW    = 0,
    MTX_MEDIUM = 1,
    MTX_HIGH   = 2
};

struct MTX_Task {
    mtx_task_t taskFunction;
    void* arg;
    uint32_t interval;
    uint32_t lastRun;
    uint8_t priority : 2;
    uint8_t isActive : 1;
    uint8_t isOneShot : 1;
    uint8_t isMicros : 1;
};

template <uint8_t MAX_TASKS = 5>
class MicroTaskXKernel {
  private:
    uint32_t _idleCounter;
    uint32_t _maxIdle;
    uint32_t _cpuCheckMillis;
    MTX_Task _tasks[MAX_TASKS];
    bool _smartSleepEnabled;

    MicroTaskXKernel() {
      _idleCounter       = 0;
      _maxIdle           = 0;
      _cpuCheckMillis    = 0;
      _smartSleepEnabled = false;
      for (uint8_t i = 0; i < MAX_TASKS; i++) {
        _tasks[i].taskFunction = nullptr;
        _tasks[i].isActive     = false;
      }
    }

    MicroTaskXKernel(const MicroTaskXKernel&) = delete;
    MicroTaskXKernel& operator=(const MicroTaskXKernel&) = delete;

  public:
    static MicroTaskXKernel& getInstance() {
      static MicroTaskXKernel instance;
      return instance;
    }

    void initLibrary() {
      #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        ADCSRA = (ADCSRA & 0xF8) | 0x04;
      #endif

      uint32_t startWin = millis();
      _idleCounter = 0;
      while (millis() - startWin < 50) {
        _idleCounter++;
      }
      _maxIdle        = _idleCounter * 20;
      _idleCounter    = 0;
      _cpuCheckMillis = millis();
      MTX_PRINTLN(F("[MTX] Kernel Initialized Successfully."));
    }

    int getCPUUsage() {
      uint32_t currentMillis = millis();
      if (currentMillis - _cpuCheckMillis >= 1000) {
        if (_maxIdle == 0) _maxIdle = 1;
        long usage = 100 - (((long)_idleCounter * 100) / _maxIdle);
        usage = constrain(usage, 0, 100);
        _idleCounter    = 0;
        _cpuCheckMillis = currentMillis;
        return (int)usage;
      }
      return -1;
    }

    bool addTask(mtx_task_t func, uint32_t intervalMs, MTXPriority priority = MTX_MEDIUM, void* arg = nullptr) {
      return _addInternal(func, intervalMs, priority, false, arg, false);
    }
    bool addTaskMicros(mtx_task_t func, uint32_t intervalUs, MTXPriority priority = MTX_MEDIUM, void* arg = nullptr) {
      return _addInternal(func, intervalUs, priority, false, arg, true);
    }

    bool addOneShotTask(mtx_task_t func, uint32_t delayMs, MTXPriority priority = MTX_MEDIUM, void* arg = nullptr) {
      return _addInternal(func, delayMs, priority, true, arg, false);
    }

    bool removeTask(mtx_task_t func) {
      for (uint8_t i = 0; i < MAX_TASKS; i++) {
        if (_tasks[i].taskFunction == func) {
          _tasks[i].taskFunction = nullptr;
          _tasks[i].isActive     = false;
          MTX_PRINTLN(F("[MTX] Task removed."));
          return true;
        }
      }
      MTX_PRINTLN(F("[MTX] Warning: Task to remove not found."));
      return false;
    }

    bool pauseTask(mtx_task_t func) {
      for (uint8_t i = 0; i < MAX_TASKS; i++) {
        if (_tasks[i].taskFunction == func) {
          _tasks[i].isActive = false;
          return true;
        }
      }
      return false;
    }

    bool resumeTask(mtx_task_t func) {
      for (uint8_t i = 0; i < MAX_TASKS; i++) {
        if (_tasks[i].taskFunction == func) {
          _tasks[i].isActive = true;
          _tasks[i].lastRun  = _tasks[i].isMicros ? micros() : millis();
          return true;
        }
      }
      return false;
    }

    bool setInterval(mtx_task_t func, uint32_t newInterval) {
      for (uint8_t i = 0; i < MAX_TASKS; i++) {
        if (_tasks[i].taskFunction == func) {
          _tasks[i].interval = newInterval;
          return true;
        }
      }
      return false;
    }

    void runTasks() {
      uint32_t currentMillis   = millis();
      uint32_t currentMicros   = micros();
      int      targetIndex     = -1;
      int      highestPriority = -1;
      uint32_t minTimeToNextMs = 0xFFFFFFFF;
      bool     hasMicrosTasks  = false;

      for (uint8_t i = 0; i < MAX_TASKS; i++) {
        if (_tasks[i].taskFunction != nullptr && _tasks[i].isActive) {
          uint32_t now         = _tasks[i].isMicros ? currentMicros : currentMillis;
          uint32_t timeElapsed = now - _tasks[i].lastRun;

          if (_tasks[i].isMicros) hasMicrosTasks = true;

          if (timeElapsed >= _tasks[i].interval) {
            int effectivePriority = static_cast<int>(_tasks[i].priority);
            if (timeElapsed > (_tasks[i].interval * 2)) {
              effectivePriority += 2;
            }

            if (effectivePriority > highestPriority) {
              highestPriority = effectivePriority;
              targetIndex     = i;
            }
          } else {
            if (!_tasks[i].isMicros) {
              uint32_t timeLeft = _tasks[i].interval - timeElapsed;
              if (timeLeft < minTimeToNextMs) {
                minTimeToNextMs = timeLeft;
              }
            }
          }
        }
      }

      if (targetIndex != -1) {
        _tasks[targetIndex].taskFunction(_tasks[targetIndex].arg);

        if (_tasks[targetIndex].isOneShot) {
          _tasks[targetIndex].taskFunction = nullptr;
          _tasks[targetIndex].isActive     = false;
        } else {
          _tasks[targetIndex].lastRun = _tasks[targetIndex].isMicros ? micros() : millis();
        }
      } else {
        if (_smartSleepEnabled && !hasMicrosTasks && minTimeToNextMs > 5) {
          enterLowPowerSleep(minTimeToNextMs);
        }
      }
    }

    void enableSmartSleep(bool enable) { _smartSleepEnabled = enable; }

    void enterLowPowerSleep(uint32_t MS_to_sleep) {
      #if defined(__AVR__)
        (void)MS_to_sleep;
        set_sleep_mode(SLEEP_MODE_IDLE);
        sleep_enable();
        power_adc_disable();
        power_spi_disable();
        sleep_cpu();
        sleep_disable();
        power_all_enable();
        _idleCounter += 120;
      #elif defined(ARDUINO_ARCH_ESP32)
        if (MS_to_sleep > 10) {
          esp_sleep_enable_timer_wakeup((MS_to_sleep - 2) * 1000);
          esp_light_sleep_start();
        }
      #endif
    }

    inline void benchTick() { _idleCounter++; }

  private:
    bool _addInternal(mtx_task_t func, uint32_t interval, MTXPriority priority, bool oneShot, void* arg, bool isMicros) {
      for (uint8_t i = 0; i < MAX_TASKS; i++) {
        if (_tasks[i].taskFunction == func) {
          _tasks[i].interval  = interval;
          _tasks[i].priority  = static_cast<uint8_t>(priority);
          _tasks[i].isOneShot = oneShot;
          _tasks[i].arg       = arg;
          _tasks[i].isMicros  = isMicros;
          _tasks[i].isActive  = true;
          return true;
        }
      }

      for (uint8_t i = 0; i < MAX_TASKS; i++) {
        if (_tasks[i].taskFunction == nullptr) {
          _tasks[i].taskFunction = func;
          _tasks[i].interval     = interval;
          _tasks[i].lastRun      = isMicros ? micros() : millis();
          _tasks[i].priority     = static_cast<uint8_t>(priority);
          _tasks[i].isOneShot    = oneShot;
          _tasks[i].arg          = arg;
          _tasks[i].isMicros     = isMicros;
          _tasks[i].isActive     = true;
          return true;
        }
      }

      MTX_PRINTLN(F("[MTX] Error: Task List Full! Increase MAX_TASKS template parameter."));
      return false;
    }
};

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
