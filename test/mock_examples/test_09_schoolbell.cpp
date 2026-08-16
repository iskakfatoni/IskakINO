#define ARDUINO_ARCH_ESP32 1
#include "ArduinoExtra.h"
#include "Wire.h"
#include "Stream.h"
#include "../../examples/09_SmartSchoolBell/09_SmartSchoolBell.ino"
extern unsigned long _mock_millis_value;
int main() {
    Wire._ackAddress[0x27] = true;
    setup();
    for (int i = 0; i < 3000; i++) { _mock_millis_value += 50; loop(); }
    printf("OK: 09_SmartSchoolBell jalan tanpa crash (semua 6 modul terintegrasi)\n");
    return 0;
}
