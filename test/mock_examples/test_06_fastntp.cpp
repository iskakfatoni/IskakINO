#include "ArduinoExtra.h"
#include "../../examples/06_FastNTP_ClockSync/06_FastNTP_ClockSync.ino"
extern unsigned long _mock_millis_value;
int main() {
    setup(); // WiFi.status() mock selalu WL_CONNECTED -> while() langsung lolos
    for (int i = 0; i < 5; i++) { _mock_millis_value += 200; loop(); }
    printf("OK: 06_FastNTP_ClockSync jalan tanpa crash\n");
    return 0;
}
