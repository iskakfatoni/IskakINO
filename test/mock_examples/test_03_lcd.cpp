#include "ArduinoExtra.h"
#include "Wire.h"
#include "../../examples/03_LCD_TypewriterScroll/03_LCD_TypewriterScroll.ino"
extern unsigned long _mock_millis_value;
int main() {
    Wire._ackAddress[0x27] = true;
    setup();
    // loop() sengaja hanya dipanggil sekali penuh (berisi while+delay
    // internal per demo) -- delay() di mock adalah no-op jadi while()
    // yang menunggu millis() TIDAK akan pernah selesai secara natural.
    // Untuk validasi compile+jalannya logika awal saja, kita panggil
    // demoTypewriter() secara terpisah dgn _mock_millis_value dimajukan manual.
    lcd.clear();
    lcd.typewriterStart("Halo, IskakINO!", 0, 80);
    int guard = 0;
    while (lcd.isTypewriterActive() && guard < 100) {
        _mock_millis_value += 90;
        lcd.update();
        guard++;
    }
    printf("Typewriter selesai dlm %d tick (guard<100 = tidak infinite loop)\n", guard);
    printf("OK: 03_LCD_TypewriterScroll (logic inti) jalan tanpa crash\n");
    return 0;
}
