/*
 * 03_LCD_TypewriterScroll.ino
 * Modul: LiquidCrystal_I2C & IskakINO_ArduFast (Universal)
 *
 * Menunjukkan animasi teks typewriter, marquee scroll, dan progress bar
 * yang berjalan secara asinkron tanpa memblokir siklus program utama.
 * Penjadwalan transisi antar-efek dikelola menggunakan IskakINO_ArduFast scheduler.
 */

#include <IskakINO.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);

enum DemoState : uint8_t {
    STATE_TYPEWRITER_START,
    STATE_TYPEWRITER_WAIT,
    STATE_SCROLL_START,
    STATE_SCROLL_WAIT,
    STATE_PROGRESS_START,
    STATE_PROGRESS_STEP,
    STATE_FINISH
};

DemoState currentState = STATE_TYPEWRITER_START;
uint8_t progressPercent = 0;

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - LCD Non-Blocking Showcase  "));
    fast.log(F("========================================"));

    lcd.begin(); // Auto-scan alamat I2C (0x27 / 0x3F)

    if (!lcd.isConnected()) {
        fast.log(F("[Peringatan] LCD tidak terdeteksi -- periksa koneksi kabel I2C SDA/SCL."));
    } else {
        fast.log(F("[Ready] LCD I2C terdeteksi."));
    }

    lcd.backlight();
    lcd.setBacklightTimeout(30000); // Backlight mati otomatis setelah 30 detik idle
}

void loop() {
    // WAJIB: Panggil lcd.update() di setiap putaran loop untuk memproses animasi teks
    lcd.update();

    // Mesin status demonstrasi non-blocking dikontrol via ArduFast scheduler
    switch (currentState) {
        case STATE_TYPEWRITER_START:
            lcd.clear();
            lcd.typewriterStart("Halo, IskakINO!", 0, 80); // Baris 0, delay 80ms/karakter
            currentState = STATE_TYPEWRITER_WAIT;
            break;

        case STATE_TYPEWRITER_WAIT:
            if (!lcd.isTypewriterActive()) {
                // Jeda 2 detik setelah teks selesai diketik
                if (fast.once(2000, 0)) {
                    fast.reset(0);
                    currentState = STATE_SCROLL_START;
                }
            }
            break;

        case STATE_SCROLL_START:
            lcd.clear();
            lcd.printCenter("Teks Berjalan:", 0);
            lcd.scrollTextStart("IskakINO: ArduFast + Storage + LCD + Voice + Shield + WiFi + NTP", 1, 250);
            fast.reset(1);
            currentState = STATE_SCROLL_WAIT;
            break;

        case STATE_SCROLL_WAIT:
            // Biarkan teks berjalan selama 6 detik
            if (fast.once(6000, 1)) {
                fast.reset(1);
                lcd.scrollTextStop();
                currentState = STATE_PROGRESS_START;
            }
            break;

        case STATE_PROGRESS_START:
            lcd.clear();
            lcd.printCenter("Loading...", 0);
            progressPercent = 0;
            currentState = STATE_PROGRESS_STEP;
            break;

        case STATE_PROGRESS_STEP:
            // Naikkan progress bar setiap 200 ms
            if (fast.every(200, 2)) {
                lcd.drawProgressBar(progressPercent, 1);
                progressPercent += 10;
                if (progressPercent > 100) {
                    currentState = STATE_FINISH;
                }
            }
            break;

        case STATE_FINISH:
            // Tunggu 2 detik lalu ulangi siklus demo dari awal
            if (fast.once(2000, 3)) {
                fast.reset(3);
                currentState = STATE_TYPEWRITER_START;
            }
            break;
    }
}
