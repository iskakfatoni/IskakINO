#ifndef ARDUINO_ARCH_ESP32
#define ARDUINO_ARCH_ESP32 1
#endif
#ifndef ISKAKINO_PLATFORM_ESP32
#define ISKAKINO_PLATFORM_ESP32 1
#endif

#include "../mock_esp32/ArduinoExtra.h"
#include "../mock_lcd/Wire.h"
#include "../mock_voice/Stream.h"
#include "../../examples/09_SmartSchoolBell/09_SmartSchoolBell.ino"

extern unsigned long _mock_millis_value;
int main() {
    Wire._ackAddress[0x27] = true;
    setup();
    for (int i = 0; i < 3000; i++) { _mock_millis_value += 50; loop(); }
    printf("OK: 09_SmartSchoolBell jalan tanpa crash (semua 6 modul terintegrasi)\n");
    return 0;
}
