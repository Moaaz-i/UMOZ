/*
 * UMOZ Library Example - Smart Auto-Lighting System
 * ----------------------------------------------------------------------------
 * Description: Demonstrates how to run an asynchronous background task to
 * sample an Ultrasonic Distance Sensor and dynamically control an LED based
 * on proximity—all without using blocking delay() functions.
 * ----------------------------------------------------------------------------
 */

#include <UMOZ.h>

const uint8_t TRIGGER_PIN = 3;
const uint8_t ECHO_PIN = 4;
const uint8_t LIGHT_PIN = 5;

// --- Background Task: Non-blocking Proximity Sensing ---
void distanceSensorTask() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Set a timeout of 20000 microseconds to prevent the sensor from freezing the CPU
  long duration = pulseIn(ECHO_PIN, HIGH, 20000);
  int distance = duration * 0.034 / 2;

  if (distance > 0 && distance < 20) {
    digitalWrite(LIGHT_PIN, HIGH); // Turn on the light if an object is closer than 20cm
  } else {
    digitalWrite(LIGHT_PIN, LOW);
  }
}

UMOZ_START()
  Serial.begin(9600);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LIGHT_PIN, OUTPUT);

  // Register the distance task to run in the background every 100ms
  tool.addTask(distanceSensorTask, 100);

UMOZ_RUN()
  // Print system status every 2000ms using the built-in independent macro timer
  UMOZ_EVERY_MS(2000) {
    Serial.println(F("[System] Proximity tracking is active and running fluidly..."));
  }
UMOZ_END
