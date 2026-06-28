#include <Arduino.h>
#include <MicroTaskX.h>

const uint8_t LED = 13;

MTX_START()
  pinMode(LED, OUTPUT);
MTX_RUN()
  MTX_EVERY_MS(100) {
    MTXUtils::toggleFast<LED>();
  }
MTX_END
