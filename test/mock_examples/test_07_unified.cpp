#include "ArduinoExtra.h"
#include "Wire.h"
#include "../../examples/07_Unified_SmartClock/07_Unified_SmartClock.ino"
extern unsigned long _mock_millis_value;
int main() {
    Wire._ackAddress[0x27] = true;
    setup();
    for (int i = 0; i < 3000; i++) { _mock_millis_value += 50; loop(); }
    printf("OK: 07_Unified_SmartClock jalan tanpa crash (semua modul dipakai bersamaan)\n");
    return 0;
}
