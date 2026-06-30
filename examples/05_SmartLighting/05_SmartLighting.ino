#include "MicroTaskX.h"

const uint8_t LED = 9;
const uint8_t SENSOR = A0;

int smoothed = 0;

void adjustLight(void* arg) {
  int raw = MTXUtils::smoothReadFast(SENSOR, smoothed);
  int brightness = map(raw, 0, 1023, 0, 255);
  analogWrite(LED, brightness);
}

MTX_START()
pinMode(LED, OUTPUT);
mtx.addTask(adjustLight, 50);
MTX_RUN()
MTX_END
