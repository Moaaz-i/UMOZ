#include "MicroTaskX.h"

MTX_START()
Serial.begin(9600);
MTX_RUN()
MTX_EVERY_MS(1000) {
  Serial.println("Timer 1 → every 1s");
}

MTX_EVERY_MS(2500) {
  Serial.println("Timer 2 → every 2.5s");
}

MTX_EVERY_HZ(2) {
  Serial.println("Timer 3 → 2 Hz");
}
MTX_END
