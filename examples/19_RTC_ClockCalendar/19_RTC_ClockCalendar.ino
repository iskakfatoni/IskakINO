/*
 * 19_RTC_ClockCalendar.ino
 * Modul: IskakINO_RTC & IskakINO_ArduFast (UNIVERSAL: AVR, ESP32, ESP8266)
 *
 * Menunjukkan:
 *   1. Auto-deteksi chip RTC pada I2C (DS3231, DS1307, atau PCF8563)
 *   2. Pembacaan waktu dan kalender terformat (IskakDateTime)
 *   3. Pembacaan sensor suhu presisi internal (khusus DS3231)
 *   4. Pengaturan tanggal/jam manual
 */

#include <IskakINO.h>

IskakINO_ArduFast fast;
IskakINO_RTC      rtc;

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - RTC Clock & Calendar Demo  "));
    fast.log(F("========================================"));

    // Inisialisasi RTC (I2C default pada SDA/SCL)
    if (!rtc.begin()) {
        fast.log(F("[Error] Modul RTC tidak terdeteksi pada bus I2C!"));
        while (1) { delay(1000); }
    }

    fast.logf(F("[RTC] Chip aktif: %s"), rtc.chipName());

    // Cek jika baterai RTC habis / power lost
    if (rtc.lostPower()) {
        fast.log(F("[RTC] Peringatan: Daya RTC sempat hilang! Menyetel waktu awal..."));
        // Set ke waktu awal contoh: 2026-08-28 19:00:00
        rtc.setDateTime(2026, 8, 28, 19, 0, 0);
    }
}

void loop() {
    // Tampilkan waktu setiap 1 detik non-blocking
    if (fast.every(1000, 0)) {
        IskakDateTime dt = rtc.now();

        // Tanggal & Hari format Bahasa Indonesia
        String dateStr = dt.getDateString(true);
        // Jam format HH:MM:SS
        String timeStr = dt.getTimeString(true);

        if (rtc.chipType() == IskakRTCType::DS3231) {
            float temp = rtc.getTemperature();
            fast.logf(F("[Clock] %s | %s | Suhu: %.2f °C"), dateStr.c_str(), timeStr.c_str(), temp);
        } else {
            fast.logf(F("[Clock] %s | %s"), dateStr.c_str(), timeStr.c_str());
        }
    }
}
