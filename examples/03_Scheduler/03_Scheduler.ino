#include "MicroTaskX.h"

void taskA(void* arg) {
  Serial.println("Task A running every 500ms");
}

void taskB(void* arg) {
  Serial.println("Task B running every 2s");
}

MTX_START()
Serial.begin(9600);

mtx.addTask(taskA, 500, MTX_MEDIUM);
mtx.addTask(taskB, 2000, MTX_HIGH);
MTX_RUN()
MTX_END
