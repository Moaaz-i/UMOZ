#include "MTXUtils.h"

/*
 * =================================================================
 * ================= MicroTaskX Utility Features ===================
 * =================================================================
 *
 * - Analog Read Smoothing (Exponential Moving Average)
 * - Button Debouncing (Press & Long Press)
 * - PID Controller Logic
 * - Hysteresis Function for thresholds
 * - Battery Percentage Calculation
 * - Time Formatting (ms to HH:MM:SS)
 * - Data Conversion (Bytes to Hex String)
 * - CRC8 Checksum Calculation
 * - ASCII Progress Bar Generation
 */

int MTXUtils::smoothRead(uint8_t analogPin, int& smoothedReference, uint8_t alpha) {
  int newRead = analogRead(analogPin);
  if (alpha > 100)
    alpha = 100;
  if (alpha == 0)
    alpha = 1;
  smoothedReference = ((100 - alpha) * smoothedReference + alpha * newRead) / 100;
  return smoothedReference;
}

bool MTXUtils::isButtonPressed(uint8_t buttonPin, MTXButton& btnState, uint32_t debounceDelay) {
  bool reading = digitalRead(buttonPin);
  bool triggered = false;

  if (reading != btnState.lastState) {
    btnState.lastDebounceTime = millis();
  }

  if ((millis() - btnState.lastDebounceTime) >= debounceDelay) {
    if (reading == LOW) {
      if (!btnState.lastState) {
        triggered = true;
      }
    }
  }
  btnState.lastState = reading;
  return triggered;
}

bool MTXUtils::isButtonLongPressed(uint8_t buttonPin, MTXButton& btnState, uint32_t longPressThreshold) {
  bool reading = digitalRead(buttonPin);
  if (reading == LOW) {
    if (!btnState.isLongPressed && (millis() - btnState.lastDebounceTime >= longPressThreshold)) {
      btnState.isLongPressed = true;
      return true;
    }
  } else {
    btnState.isLongPressed = false;
  }
  return false;
}

float MTXUtils::computePID(float setpoint, float currentInput, MTXPID& pid, float dt) {
  if (dt <= 0.0f)
    dt = 0.01f;
  float error = setpoint - currentInput;
  pid.integral += error * dt;
  float derivative = (error - pid.lastError) / dt;
  float output = (pid.kp * error) + (pid.ki * pid.integral) + (pid.kd * derivative);

  if (output > pid.maxOutput) {
    output = pid.maxOutput;
    pid.integral -= error * dt;
  } else if (output < pid.minOutput) {
    output = pid.minOutput;
    pid.integral -= error * dt;
  }

  pid.lastError = error;
  return output;
}

bool MTXUtils::applyHysteresis(float value, float threshold, float margin, bool currentOutputState) {
  if (currentOutputState) {
    if (value < (threshold - margin))
      return false;
  } else {
    if (value > (threshold + margin))
      return true;
  }
  return currentOutputState;
}

uint8_t MTXUtils::calculateBatteryPercentage(uint8_t analogPin, float r1, float r2, float minVolts, float maxVolts,
                                             float vRef, int adcResolution) {
  int rawADC = analogRead(analogPin);
  float vADC = (rawADC * vRef) / adcResolution;
  float vBattery = vADC * ((r1 + r2) / r2);

  float percentage = ((vBattery - minVolts) / (maxVolts - minVolts)) * 100.0;
  return (uint8_t)constrain(percentage, 0, 100);
}

void MTXUtils::formatMillisToTime(uint32_t ms, char* outputBuffer) {
  uint32_t totalSeconds = ms / 1000;
  uint8_t seconds = totalSeconds % 60;
  uint32_t totalMinutes = totalSeconds / 60;
  uint8_t minutes = totalMinutes % 60;
  uint32_t hours = totalMinutes / 60;

  sprintf(outputBuffer, "%02lu:%02u:%02u", hours, minutes, seconds);
}

void MTXUtils::bytesToHexString(const uint8_t* bytes, size_t length, char* outputBuffer) {
  for (size_t i = 0; i < length; i++) {
    sprintf(&outputBuffer[i * 2], "%02X", bytes[i]);
  }
}

uint8_t MTXUtils::computeCRC8(const uint8_t* data, size_t length) {
  uint8_t crc = 0x00;
  for (size_t i = 0; i < length; i++) {
    uint8_t extract = data[i];
    for (uint8_t tempI = 8; tempI; tempI--) {
      uint8_t sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}

void MTXUtils::getProgressBarString(uint8_t width, uint8_t percentage, char* outputBuffer) {
  if (width < 3)
    return;

  uint8_t barWidth = width - 2;
  uint8_t filledChars = (percentage * barWidth) / 100;

  outputBuffer[0] = '[';
  for (uint8_t i = 1; i <= barWidth; i++) {
    if (i <= filledChars) {
      outputBuffer[i] = '=';
    } else {
      outputBuffer[i] = ' ';
    }
  }
  outputBuffer[width - 1] = ']';
  outputBuffer[width] = '\0';
}
