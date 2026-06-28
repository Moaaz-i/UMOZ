#include <MicroTaskX.h>

void showCPU() {
  int usage = mtx.getCPUUsage();
  if (usage != -1) {
    Serial.print("CPU Load: ");
    Serial.print(usage);
    Serial.println("%");
  }
}

MTX_START()
  Serial.begin(9600);
  mtx.addTask(showCPU, 1000);
MTX_RUN()
MTX_END
