/*
 * 03_LCD_TypewriterScroll.ino
 * Modul: LiquidCrystal_I2C & IskakINO_ArduFast (Universal)
 *
 * Menunjukkan fitur lengkap LCD I2C IskakINO:
 * 1. Efek mesin ketik (Typewriter) non-blocking.
 * 2. Teks berjalan panjang (Marquee scroll) non-blocking.
 * 3. Indikator grafik Progress Bar (0-100%).
 * 4. Generator Ikon Kustom Dinamis (Baterai, Sinyal WiFi bar / RSSI, Termometer).
 * 5. Dynamic Banner / Page Flipper otomatis multi-halaman tanpa memblokir siklus program.
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
    STATE_ICONS_DEMO,
    STATE_ICONS_WAIT,
    STATE_BANNER_START,
    STATE_BANNER_WAIT,
    STATE_FINISH
};

DemoState currentState = STATE_TYPEWRITER_START;
uint8_t progressPercent = 0;

// Definisi 3 halaman banner statis untuk demo Dynamic Banner
const LCDPage demoPages[] = {
    LCDPage("IskakINO Core", "Smart Library"),
    LCDPage("Status: Online", "WiFi: 4 Bars"),
    LCDPage("Suhu: 28 C", "Baterai: 95%")
};

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
    lcd.setBacklightTimeout(60000); // Backlight mati otomatis setelah 60 detik idle
}

void loop() {
    // WAJIB: Panggil lcd.update() di setiap putaran loop untuk memproses animasi & banner
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
            // Biarkan teks berjalan selama 5 detik
            if (fast.once(5000, 1)) {
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
            // Naikkan progress bar setiap 150 ms
            if (fast.every(150, 2)) {
                lcd.drawProgressBar(progressPercent, 1);
                progressPercent += 10;
                if (progressPercent > 100) {
                    currentState = STATE_ICONS_DEMO;
                }
            }
            break;

        case STATE_ICONS_DEMO:
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Icons: ");

            // Gambar custom icons otomatis tanpa atur byte manual:
            lcd.drawBattery(8, 0, 85);          // Baterai 85% di col 8, row 0 (slot 0)
            lcd.drawWifiSignalRssi(11, 0, -62); // WiFi 3 bar (-62 dBm) di col 11, row 0 (slot 1)
            lcd.drawThermometer(14, 0, 2);      // Termometer med (level 2) di col 14, row 0 (slot 2)

            lcd.setCursor(0, 1);
            lcd.print("Bat85% Wf3 Th2");

            fast.reset(3);
            currentState = STATE_ICONS_WAIT;
            break;

        case STATE_ICONS_WAIT:
            // Tampilkan ikon selama 3 detik lalu masuk ke demo Banner
            if (fast.once(3000, 3)) {
                fast.reset(3);
                currentState = STATE_BANNER_START;
            }
            break;

        case STATE_BANNER_START:
            // Mulai Dynamic Banner (3 halaman berganti otomatis tiap 2 detik)
            lcd.bannerStart(demoPages, 3, 2000);
            fast.reset(4);
            currentState = STATE_BANNER_WAIT;
            break;

        case STATE_BANNER_WAIT:
            // Biarkan banner berganti halaman selama 6.5 detik
            if (fast.once(6500, 4)) {
                fast.reset(4);
                lcd.bannerStop();
                currentState = STATE_FINISH;
            }
            break;

        case STATE_FINISH:
            // Tunggu 1.5 detik lalu ulangi siklus demo dari awal
            if (fast.once(1500, 5)) {
                fast.reset(5);
                currentState = STATE_TYPEWRITER_START;
            }
            break;
    }
}
