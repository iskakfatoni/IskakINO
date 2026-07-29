#include "ArduinoExtra.h"
#include "../../examples/01_ArduFast_TaskManager/01_ArduFast_TaskManager.ino"
int main() {
    setup();
    for (int i = 0; i < 5000; i++) { _mock_millis_value += 1; loop(); }
    printf("OK: 01_ArduFast_TaskManager jalan tanpa crash\n");
    return 0;
}
