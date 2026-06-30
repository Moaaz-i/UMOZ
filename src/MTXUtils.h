#ifndef MTX_UTILS_H
#define MTX_UTILS_H

#include "Arduino.h"

struct MTXButton {
  uint32_t lastDebounceTime;
  bool lastState;
  bool isLongPressed;
};

struct MTXPID {
  float kp, ki, kd;
  float integral;
  float lastError;
  float minOutput;
  float maxOutput;
};

struct MTXEdge {
  bool lastState;
};

struct MTXTypewriter {
  uint32_t lastCharTime;
  size_t currentIndex;
};

// Feature 7: Kalman Filter
struct MTX_Kalman {
  float q;  // process noise covariance
  float r;  // measurement noise covariance
  float x;  // value
  float p;  // estimation error covariance
  float k;  // kalman gain
  MTX_Kalman() : q(0.125f), r(4.0f), x(0.0f), p(1.0f), k(0.0f) {}
};

// Feature 8: Software PWM
struct MTX_PWMState {
  uint8_t pin;
  uint8_t dutyCycle;
  uint32_t periodMicros;
  uint32_t lastFlip;
  bool pinState;
};

// Feature 9: Rotary Encoder
struct MTX_Encoder {
  uint8_t pinA;
  uint8_t pinB;
  int32_t position;
  uint8_t lastStateA;
};

class MTXUtils {
 public:
  static MTXUtils& getInstance() {
    static MTXUtils instance;
    return instance;
  }

  // Feature 6: RAM Diagnostics
  static uint32_t getFreeMemory();

  // Feature 7: Kalman Filter
  static float kalmanFilter(float measurement, MTX_Kalman& kf);

  // Feature 8: Software PWM
  static void handlePWM(MTX_PWMState& pwm);

  // Feature 9: Rotary Encoder
  static void initEncoder(MTX_Encoder& enc, uint8_t pA, uint8_t pB);
  static bool updateEncoder(MTX_Encoder& enc);

  template <uint8_t PIN>
  static inline void toggleFast() {
#if defined(__AVR__)
    *portInputRegister(digitalPinToPort(PIN)) = digitalPinToBitMask(PIN);
#else
    digitalWrite(PIN, !digitalRead(PIN));
#endif
  }

  static inline void toggle(uint8_t pin) { digitalWrite(pin, !digitalRead(pin)); }

  static inline int smoothReadFast(uint8_t analogPin, int& smoothedReference) {
    int newRead = analogRead(analogPin);
    smoothedReference = (smoothedReference * 7 + newRead) >> 3;
    return smoothedReference;
  }

  static int smoothRead(uint8_t analogPin, int& smoothedReference, uint8_t alpha = 15);

  template <uint8_t WINDOW_SIZE = 10>
  static int movingAverage(int newRead, int* buffer, uint8_t& index, long& sum) {
    sum -= buffer[index];
    buffer[index] = newRead;
    sum += buffer[index];
    index = (index + 1) % WINDOW_SIZE;
    return sum / WINDOW_SIZE;
  }

  static bool isButtonPressed(uint8_t buttonPin, MTXButton& btnState, uint32_t debounceDelay = 50);
  static bool isButtonLongPressed(uint8_t buttonPin, MTXButton& btnState, uint32_t longPressThreshold = 1000);

  static inline bool isRisingEdge(bool currentState, MTXEdge& edgeState) {
    bool result = (currentState && !edgeState.lastState);
    edgeState.lastState = currentState;
    return result;
  }
  static inline bool isFallingEdge(bool currentState, MTXEdge& edgeState) {
    bool result = (!currentState && edgeState.lastState);
    edgeState.lastState = currentState;
    return result;
  }

  static float computePID(float setpoint, float currentInput, MTXPID& pid, float dt);
  static bool applyHysteresis(float value, float threshold, float margin, bool currentOutputState);

  static inline int toPercentage(int rawValue, int minRaw, int maxRaw) {
    return constrain(map(rawValue, minRaw, maxRaw, 0, 100), 0, 100);
  }

  static inline float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    if (in_max == in_min)
      return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  }

  static uint8_t calculateBatteryPercentage(uint8_t analogPin, float r1, float r2, float minVolts, float maxVolts,
                                            float vRef = 5.0, int adcResolution = 1023);

  static void formatMillisToTime(uint32_t ms, char* outputBuffer);
  static void bytesToHexString(const uint8_t* bytes, size_t length, char* outputBuffer);
  static uint8_t computeCRC8(const uint8_t* data, size_t length);

  static inline bool hasTimeout(uint32_t startTime, uint32_t interval) { return (millis() - startTime >= interval); }

  static inline void setBit(uint32_t& variable, uint8_t bit) { variable |= (1UL << bit); }
  static inline void clearBit(uint32_t& variable, uint8_t bit) { variable &= ~(1UL << bit); }
  static inline void toggleBit(uint32_t& variable, uint8_t bit) { variable ^= (1UL << bit); }
  static inline bool readBit(uint32_t variable, uint8_t bit) { return (variable >> bit) & 1U; }

  static inline uint8_t getCenterOffset(uint8_t lcdWidth, const char* text) {
    size_t len = strlen(text);
    if (len >= lcdWidth)
      return 0;
    return (lcdWidth - len) / 2;
  }

  template <typename T>
  static bool printTypewriter(T& lcd, const char* text, uint8_t col, uint8_t row, uint32_t speedMs,
                              MTXTypewriter& state) {
    size_t len = strlen(text);
    if (state.currentIndex >= len)
      return true;

    if (millis() - state.lastCharTime >= speedMs) {
      lcd.setCursor(col + state.currentIndex, row);
      lcd.print(text[state.currentIndex]);
      state.currentIndex++;
      state.lastCharTime = millis();
    }
    return false;
  }

  template <uint8_t COLS, uint8_t ROWS, typename T>
  static void updateScreenSmart(T& lcd, const char newBuf[ROWS][COLS + 1], char oldBuf[ROWS][COLS + 1]) {
    for (uint8_t r = 0; r < ROWS; r++) {
      for (uint8_t c = 0; c < COLS; c++) {
        if (newBuf[r][c] != oldBuf[r][c]) {
          lcd.setCursor(c, r);
          lcd.print(newBuf[r][c]);
          oldBuf[r][c] = newBuf[r][c];
        }
      }
    }
  }

  static void getProgressBarString(uint8_t width, uint8_t percentage, char* outputBuffer);
};

template <typename T, uint16_t SIZE>
class MTXCircularQueue {
 private:
  T _buffer[SIZE];
  uint16_t _head = 0;
  uint16_t _tail = 0;
  uint16_t _count = 0;

 public:
  bool enqueue(T item) {
    if (_count >= SIZE)
      return false;
    _buffer[_tail] = item;
    _tail = (_tail + 1) % SIZE;
    _count++;
    return true;
  }
  bool dequeue(T& item) {
    if (_count == 0)
      return false;
    item = _buffer[_head];
    _head = (_head + 1) % SIZE;
    _count--;
    return true;
  }
  uint16_t available() const { return _count; }
  void clear() {
    _head = 0;
    _tail = 0;
    _count = 0;
  }
};

#endif
