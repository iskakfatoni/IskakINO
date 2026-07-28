// Verifikasi FastPin<P> tetap bisa dipakai lewat #include <IskakINO_ArduFast.h>
// walau definisinya sudah pindah ke core/IskakINO_Platform.h (compile-only,
// fallback generic path karena tidak ada __AVR__/ESP32/ESP8266 di mock).
#include "Arduino.h"
#include "../../src/ardufast/IskakINO_ArduFast.h"
#include <cstdio>

int main() {
    FastPin<13> led;   // harus resolve dari IskakINO_ArduFast.h -> Platform.h
    led.mode(OUTPUT);
    led.high();
    led.low();
    led.toggle();
    bool v = led.read();
    (void)v;
    IskakINO_ArduFast fast;
    fast.setDebug(false); // method baru, opsional
    printf("OK: FastPin<P> transitif via IskakINO_ArduFast.h berhasil compile\n");
    return 0;
}
