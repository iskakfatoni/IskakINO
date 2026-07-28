#include "ArduinoExtra.h"
#include "Wire.h"
#include "../../examples/08_Framework_Kernel/08_Framework_Kernel.ino"
extern unsigned long _mock_millis_value;
int main() {
    Wire._ackAddress[0x27] = true;
    setup();
    for (int i = 0; i < 3000; i++) { _mock_millis_value += 50; loop(); }
    printf("OK: 08_Framework_Kernel jalan tanpa crash (pola framework)\n");
    return 0;
}
