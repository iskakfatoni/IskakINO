#include "ArduinoExtra.h"
#include "../../examples/05_WifiPortal_CaptivePortal/05_WifiPortal_CaptivePortal.ino"
extern unsigned long _mock_millis_value;
int main() {
    setup();
    for (int i = 0; i < 20; i++) { _mock_millis_value += 100; loop(); }
    printf("OK: 05_WifiPortal_CaptivePortal jalan tanpa crash\n");
    return 0;
}
