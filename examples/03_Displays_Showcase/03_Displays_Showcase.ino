/*
 * 03_Displays_Showcase.ino
 * Modul: LiquidCrystal_I2C & IskakINO_OLED (Universal)
 *
 * Demonstrasi terpadu penggerak display teks & grafis I2C non-blocking:
 *  1. LiquidCrystal_I2C (16x2 / 20x4 LCD):
 *     - Auto-scan alamat I2C otomatis (0x27 / 0x3F).
 *     - Efek mesin ketik (Typewriter) non-blocking.
 *     - Teks berjalan panjang (Marquee scroll).
 *     - Ikon kustom dinamis & Progress Bar bawaan.
 *  2. IskakINO_OLED (SSD1306 / SH1106 128x64):
 *     - Rendering teks font normal & 2x.
 *     - Teks typewriter & marquee scroll pada OLED.
 *     - Dashboard grafis: WiFi icon bars, battery gauge, progress bar.
 *     - Invert display & pengatur kontras.
 *
 * Catatan: Sketsa ini otomatis mendeteksi hardware I2C yang terpasang.
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);
IskakINO_OLED     oled(128, 64, 0x3C);

bool lcdDetected = false;
bool oledDetected = false;
uint8_t demoStep = 0;
uint8_t progressVal = 0;

void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("     IskakINO - LCD & OLED Dual Displays Showcase        "));
    fast.log(F("========================================================="));

    // 1. Inisialisasi LCD
    lcd.begin();
    if (lcd.isConnected()) {
        lcdDetected = true;
        lcd.backlight();
        lcd.printCenter("IskakINO LCD", 0);
        lcd.printCenter("Display Ready!", 1);
        fast.log(F("[LCD] Terdeteksi dan diinisialisasi."));
    } else {
        fast.log(F("[LCD] Tidak terdeteksi di bus I2C (0x27/0x3F)."));
    }

    // 2. Inisialisasi OLED
    if (oled.begin()) {
        oledDetected = true;
        oled.clear();
        oled.setTextSize(1);
        oled.printCenter("IskakINO OLED", 1);
        oled.printCenter("Display Ready!", 3);
        fast.log(F("[OLED] Terdeteksi pada alamat 0x3C."));
    } else {
        fast.log(F("[OLED] Tidak terdeteksi di bus I2C."));
    }

    fast.log(F("[System] Memulai demo visual non-blocking...\n"));
}

void loop() {
    // WAJIB: Panggil update() pada kedua display di setiap loop
    lcd.update();
    oled.update();

    // Jalankan siklus animasi setiap 100 ms
    if (fast.every(100, 0)) {
        progressVal++;
        if (progressVal > 100) progressVal = 0;

        // Update progress bar LCD jika terpasang (percent, row)
        if (lcdDetected && fast.every(500, 1)) {
            lcd.drawProgressBar(progressVal, 1);
        }

        // Update animasi OLED jika terpasang
        if (oledDetected && fast.every(200, 2)) {
            oled.clear();
            oled.setCursor(0, 0);
            oled.print("IskakINO Dashboard");
            oled.drawHLine(0, 1, 128, 0x01);

            oled.setCursor(0, 2);
            oled.print("Progress: ");
            oled.print(progressVal);
            oled.print("%");

            // Gambar battery icon & WiFi bar (level, col, row)
            oled.drawBatteryIcon((progressVal / 20), 108, 0);
            oled.drawWifiIcon((progressVal % 5), 90, 0);

            // Gambar progress bar grafis (percent, row)
            oled.drawProgressBar(progressVal, 4);
        }
    }

    // Rotasi demo teks setiap 8 detik
    if (fast.every(8000, 3)) {
        demoStep = (demoStep + 1) % 3;

        if (lcdDetected) {
            lcd.clear();
            if (demoStep == 0) {
                lcd.typewriterStart("Halo Sahabat!", 0, 80);
                lcd.setCursor(0, 1);
                lcd.print("IskakINO LCD");
            } else if (demoStep == 1) {
                lcd.setCursor(0, 0);
                lcd.print("Status: Aktif");
                lcd.scrollTextStart("Perpustakaan Cepat & Lengkap untuk Arduino dan ESP", 1, 150);
            } else {
                lcd.setCursor(0, 0);
                lcd.print("Suhu: 28 C");
                lcd.setCursor(0, 1);
                lcd.print("Kelembapan: 65%");
            }
        }

        if (oledDetected) {
            fast.logf(F("[Display Demo] Step: %u | Progress: %u%%"), demoStep, progressVal);
        }
    }
}
