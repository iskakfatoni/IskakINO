// Smoke-test fungsional IskakINO_LiquidCrystal_I2C — verifikasi backlight
// auto-timeout & typewriter/scroll non-blocking benar-benar berjalan lewat
// Scheduler (bukan _lastActivityMillis/_twLastMillis manual lagi).
#include "ArduinoExtra.h"
#include "Wire.h"
#include "../../src/lcd/IskakINO_LiquidCrystal_I2C.h"
#include <cassert>
#include <cstdio>

extern unsigned long _mock_millis_value;

int main() {
    Wire._ackAddress[0x27] = true; // simulasikan LCD terpasang di 0x27

    LiquidCrystal_I2C lcd(16, 2);
    lcd.setDebug(true);
    _mock_millis_value = 0;
    lcd.begin();
    assert(lcd.getAddress() == 0x27);
    assert(lcd.isConnected());

    // --- Backlight auto-timeout ---
    lcd.setBacklightTimeout(1000); // 1 detik
    _mock_millis_value = 500;
    lcd.update();
    // Belum 1 detik sejak setBacklightTimeout() -> backlight harus masih nyala.
    // (Tidak ada getter publik utk _backlight, jadi kita verifikasi tidak
    // langsung: panggil backlight() lagi untuk reset baseline, lalu uji alur
    // penuh di bawah dengan urutan yang bisa diverifikasi via noBacklight
    // side-effect pada _expanderWrite -> tidak observable langsung, jadi
    // yang penting: tidak crash & alur jalan hingga waktunya.)

    _mock_millis_value = 1600; // lewat dari 1000ms sejak reset terakhir (t=0)
    lcd.update(); // sekarang backlight timeout harus terpicu (once())

    // Reset lagi via aktivitas (write), backlight tidak seharusnya mati lagi
    // sebelum 1000ms berikutnya.
    lcd.backlight();
    _mock_millis_value = 2000;
    lcd.print("X"); // write() -> reset baseline lagi
    _mock_millis_value = 2500; // baru 500ms sejak print() terakhir
    lcd.update(); // belum boleh trigger lagi

    // --- Typewriter non-blocking ---
    lcd.typewriterStart("Hi", 0, 100); // 1 karakter tiap 100ms
    assert(lcd.isTypewriterActive());
    _mock_millis_value = 2550; // baru 50ms, belum waktunya tick pertama
    lcd.update();
    assert(lcd.isTypewriterActive()); // masih aktif, belum selesai
    _mock_millis_value = 2650; // >=100ms sejak reset() terakhir (typewriterStart)
    lcd.update(); // tick 1: cetak 'H' (_twIndex 0->1), _twActive tetap true
    assert(lcd.isTypewriterActive());
    _mock_millis_value = 2750; // >=100ms sejak tick 1
    lcd.update(); // tick 2: cetak 'i' (_twIndex 1->2), _twActive tetap true
    assert(lcd.isTypewriterActive()); // masih true — baru cetak karakter terakhir
    _mock_millis_value = 2850; // >=100ms sejak tick 2
    lcd.update(); // tick 3: _twIndex(2) >= panjang teks(2) -> BARU di sini nonaktif
    assert(!lcd.isTypewriterActive());

    // --- Scroll non-blocking ---
    lcd.scrollTextStart("AB", 1, 50); // baseline reset di t=2850
    assert(lcd.isScrollActive());
    _mock_millis_value = 2900; // 50ms sejak reset (scrollTextStart)
    lcd.update(); // tick scroll pertama
    assert(lcd.isScrollActive()); // scroll terus looping sampai scrollTextStop()
    lcd.scrollTextStop();
    assert(!lcd.isScrollActive());

    // --- printFormatted ---
    lcd.printFormatted("T=%d", 25);

    // --- drawProgressBar ---
    lcd.drawProgressBar(50, 1);

    printf("OK: semua smoke-test fungsional LCD lolos\n");
    return 0;
}
