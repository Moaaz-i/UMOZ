/*
 * UMOZ Library Implementation File
 * ----------------------------------------------------------------------------
 * Implements Cross-platform abstractions, Smart Sleep low-power management,
 * CPU benchmarking, and the core priority-aware task execution loop.
 * ----------------------------------------------------------------------------
 */

#include "UMOZ.h"

UMOZ::UMOZ() {
  _previousMillisBlink = 0;
  _previousMillisButton = 0;
  _smoothedAnalog = 0;
  _idleCounter = 0;
  _maxIdle = 0;
  _cpuCheckMillis = 0;
  _taskCount = 0;
  _smartSleepEnabled = false;

  for(uint8_t i = 0; i < UMOZ_MAX_TASKS; i++) {
     _tasks[i].isActive = false;
  }
}

void UMOZ::begin(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
}

void UMOZ::initLibrary() {
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADCSRA = (ADCSRA & 0xf8) | 0x04;
  #elif defined(ARDUINO_ARCH_ESP32)
  #endif

  uint32_t startWin = millis();
  _idleCounter = 0;
  while (millis() - startWin < 100) {
    _idleCounter++;
  }
  _maxIdle = _idleCounter * 10;
  _idleCounter = 0;
  _cpuCheckMillis = millis();
}

int UMOZ::getCPUUsage() {
  if (millis() - _cpuCheckMillis >= 1000) {
    if (_idleCounter > _maxIdle) _maxIdle = _idleCounter;

    int usage = 100 - ((_idleCounter * 100) / (_maxIdle == 0 ? 1 : _maxIdle));
    usage = constrain(usage, 0, 100);

    _idleCounter = 0;
    _cpuCheckMillis = millis();
    return usage;
  }
  return -1;
}

void UMOZ::blink(uint32_t delayTime) {
  uint32_t currentMillis = millis();
  if (currentMillis - _previousMillisBlink >= delayTime) {
    _previousMillisBlink = currentMillis;
    toggle(_pin);
  }
}

int UMOZ::smoothRead(uint8_t analogPin) {
  int newRead = analogRead(analogPin);
  _smoothedAnalog = (_smoothedAnalog * 7 + newRead) >> 3;
  return _smoothedAnalog;
}

bool UMOZ::isButtonPressed(uint8_t buttonPin) {
  if (digitalRead(buttonPin) == HIGH) {
    if (millis() - _previousMillisButton >= 50) {
      _previousMillisButton = millis();
      return true;
    }
  } else {
    _previousMillisButton = millis();
  }
  return false;
}

bool UMOZ::addTask(umoz_task_t func, uint32_t intervalMs, UMOZPriority priority) {
  if (_taskCount < UMOZ_MAX_TASKS) {
    _tasks[_taskCount].taskFunction = func;
    _tasks[_taskCount].interval = intervalMs;
    _tasks[_taskCount].lastRun = millis();
    _tasks[_taskCount].priority = priority;
    _tasks[_taskCount].isActive = true;
    _taskCount++;
    return true;
  }
  return false;
}

void UMOZ::runTasks() {
  uint32_t currentMillis = millis();
  int targetIndex = -1;
  int highestPriority = -1;
  uint32_t minTimeToNextTask = 0xFFFFFFFF;

  for (uint8_t i = 0; i < _taskCount; i++) {
    if (_tasks[i].isActive) {
      uint32_t timeElapsed = currentMillis - _tasks[i].lastRun;

      if (timeElapsed >= _tasks[i].interval) {
        if ((int)_tasks[i].priority > highestPriority) {
          highestPriority = (int)_tasks[i].priority;
          targetIndex = i;
        }
      } else {
        uint32_t timeLeft = _tasks[i].interval - timeElapsed;
        if (timeLeft < minTimeToNextTask) {
          minTimeToNextTask = timeLeft;
        }
      }
    }
  }

  if (targetIndex != -1) {
    _tasks[targetIndex].taskFunction();
    _tasks[targetIndex].lastRun = millis();
  } else {
    if (_smartSleepEnabled && minTimeToNextTask > 5) {
      enterLowPowerSleep();
    }
  }
}

void UMOZ::enterLowPowerSleep() {
  #if defined(__AVR__)
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    power_adc_disable();
    power_spi_disable();

    sleep_cpu();

    sleep_disable();
    power_all_enable();
  #elif defined(ARDUINO_ARCH_ESP32)
  #endif
}
