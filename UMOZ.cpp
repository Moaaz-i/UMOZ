/*
 * UMOZ Library Implementation File
 * ----------------------------------------------------------------------------
 * Implements the Singleton pattern core, real-time CPU benchmark calibration,
 * EMA filtering arithmetic, and the background task dispatcher loop.
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
  for(uint8_t i = 0; i < UMOZ_MAX_TASKS; i++) {
     _tasks[i].active = false;
  }
}

void UMOZ::begin(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
}

void UMOZ::initLibrary() {
  #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADCSRA = (ADCSRA & 0xf8) | 0x04;
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
    if (usage < 0) usage = 0;
    if (usage > 100) usage = 100;

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

bool UMOZ::addTask(umoz_task_t func, uint32_t intervalMs) {
  if (_taskCount < UMOZ_MAX_TASKS) {
    _tasks[_taskCount].func = func;
    _tasks[_taskCount].interval = intervalMs;
    _tasks[_taskCount].lastRun = millis();
    _tasks[_taskCount].active = true;
    _taskCount++;
    return true;
  }
  return false;
}

void UMOZ::runTasks() {
  uint32_t currentMillis = millis();
  for (uint8_t i = 0; i < _taskCount; i++) {
    if (_tasks[i].active && (currentMillis - _tasks[i].lastRun >= _tasks[i].interval)) {
      _tasks[i].lastRun = currentMillis;
      _tasks[i].func();
    }
  }
}
