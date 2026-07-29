/*
 * 03_LCD_TypewriterScroll.ino
 * Modul: LiquidCrystal_I2C (universal -- semua platform yang punya Wire/I2C)
 *
 * Menunjukkan efek typewriter, teks berjalan (scroll), dan progress bar.
 * Ketiga fitur INTERNAL library-nya non-blocking (jalan lewat lcd.update()),
 * tapi contoh ini membungkusnya dengan while()+delay() per demo supaya
 * mudah dibaca alurnya -- lihat catatan di bawah untuk pola pemakaian
 * non-blocking yang sesungguhnya di aplikasi nyata.
 *
 * Rangkaian: LCD 16x2 I2C (PCF8574), SDA/SCL sesuai board.
 * CATATAN: contoh ini sengaja pakai while()+delay() di tiap demo supaya
 * mudah dibaca alurnya. Di aplikasi nyata yang perlu tetap responsif
 * (baca sensor lain, layani WiFi, dst.), ganti pola while() dengan
 * pengecekan lcd.isTypewriterActive()/isScrollActive() di dalam loop()
 * utama -- lihat 07_Unified_SmartClock untuk contoh pola non-blocking
 * penuh yang menggabungkan LCD dengan modul lain.
 */

#include <IskakINO.h>

LiquidCrystal_I2C lcd(16, 2); // 16 kolom, 2 baris

void demoTypewriter() {
    lcd.clear();
    lcd.typewriterStart("Halo, IskakINO!", 0, 80); // baris 0, 80ms per karakter
    while (lcd.isTypewriterActive()) {
        lcd.update(); // WAJIB dipanggil berulang -- inilah yang menggerakkan efeknya
    }
    delay(1500); // jeda sejenak biar sempat dibaca sebelum demo berikutnya
}

void demoScroll() {
    lcd.clear();
    lcd.printCenter("Teks Berjalan:", 0);
    lcd.scrollTextStart("Library gabungan ArduFast + Storage + LCD + WiFi + NTP + Voice", 1, 300);
    unsigned long start = millis();
    while (millis() - start < 6000) { // scroll selama 6 detik lalu lanjut demo berikutnya
        lcd.update();
    }
    lcd.scrollTextStop();
}

void demoProgressBar() {
    lcd.clear();
    lcd.printCenter("Loading...", 0);
    for (uint8_t percent = 0; percent <= 100; percent += 20) {
        lcd.drawProgressBar(percent, 1);
        delay(400);
    }
    delay(1000);
}

void setup() {
    Serial.begin(115200);
    lcd.begin(); // auto-scan alamat I2C (0x27 atau 0x3F umumnya)

    if (!lcd.isConnected()) {
        Serial.println(F("LCD tidak terdeteksi -- cek wiring SDA/SCL."));
    }

    lcd.setBacklightTimeout(15000); // backlight mati otomatis setelah 15 detik idle
}

void loop() {
    // Tiga demo berjalan bergantian, masing-masing membersihkan layar dulu
    // supaya tidak saling tumpang tindih di layar 16x2.
    demoTypewriter();
    demoScroll();
    demoProgressBar();
}
