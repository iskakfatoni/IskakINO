/*
 * examples/28_PrayerTimes_SmartMosqueClock/28_PrayerTimes_SmartMosqueClock.ino
 *
 * Contoh Demonstrasi Modul IskakINO_PrayerTimes:
 * Mesin Kalkulator Jadwal Sholat Presisi, Kalender Hijriah & Arah Kiblat Universal.
 *
 * Kompatibel dengan:
 * - Arduino AVR (Uno, Nano, Mega)
 * - ESP8266 (NodeMCU, Wemos)
 * - ESP32 (Dev Module, S2, S3, C3)
 */

#include <IskakINO.h>

IskakINO_PrayerTimes prayerTimes;

// Koordinat Contoh: Jakarta, Indonesia
const float LATITUDE   = -6.2088f;
const float LONGITUDE  = 106.8456f;
const float TIMEZONE   = 7.0f;  // WIB (UTC+7)
const float ELEVATION  = 25.0f; // 25 mdpl

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000);

    Serial.println(F("\n========================================================"));
    Serial.println(F("   🕌 IskakINO_PrayerTimes - Smart Mosque Clock Engine   "));
    Serial.println(F("========================================================"));

    // 1. Konfigurasi Lokasi & Metode Perhitungan
    prayerTimes.setLocation(LATITUDE, LONGITUDE, TIMEZONE, ELEVATION);
    prayerTimes.setMethod(IskakPrayerMethod::KEMENAG); // Standar resmi Kemenag RI (Subuh -20°, Isya -18°)
    prayerTimes.setIkhtiyat(2);                        // +2 Menit Waktu Pengaman (Ihtiyath)

    // 2. Hitung Waktu Sholat untuk Tanggal Tertentu (Contoh: 30 Agustus 2026)
    int year = 2026;
    int month = 8;
    int day = 30;
    prayerTimes.compute(year, month, day);

    // 3. Tampilkan Kalender Hijriah & Arah Kiblat
    IskakHijriDate hijri = prayerTimes.getHijriDate(year, month, day);
    Serial.print(F("📅 Tanggal Masehi  : "));
    Serial.print(day); Serial.print(F("-"));
    Serial.print(month); Serial.print(F("-"));
    Serial.println(year);

    Serial.print(F("🌙 Tanggal Hijriah : "));
    Serial.println(hijri.toString());

    Serial.print(F("🧭 Arah Kiblat     : "));
    Serial.print(prayerTimes.getQiblaDirection(), 1);
    Serial.println(F("° dari Utara (Searah Jarum Jam)"));

    Serial.println(F("\n--------------------------------------------------------"));
    Serial.println(F("   ⏰ Jadwal Waktu Sholat Hari Ini (Standar Kemenag RI)  "));
    Serial.println(F("--------------------------------------------------------"));

    const IskakPrayer prayers[] = {
        IskakPrayer::IMSAK,
        IskakPrayer::FAJR,
        IskakPrayer::SUNRISE,
        IskakPrayer::DHUHA,
        IskakPrayer::DHUHR,
        IskakPrayer::ASR,
        IskakPrayer::MAGHRIB,
        IskakPrayer::ISHA,
        IskakPrayer::MIDNIGHT
    };

    for (size_t i = 0; i < 9; ++i) {
        IskakPrayer p = prayers[i];
        Serial.print(F(" • "));
        Serial.print(prayerTimes.getPrayerName(p));
        // Spasi perapian
        int pad = 15 - strlen(prayerTimes.getPrayerName(p));
        for (int s = 0; s < pad; ++s) Serial.print(F(" "));
        Serial.print(F(": "));
        Serial.println(prayerTimes.getFormattedTime(p));
    }
    Serial.println(F("--------------------------------------------------------\n"));

    // 4. Simulasi Cek Jadwal Sholat Berikutnya (Contoh jam 11:30:00)
    uint8_t simHour = 11;
    uint8_t simMin  = 30;
    uint8_t simSec  = 0;

    IskakPrayer nextP = prayerTimes.getNextPrayer(simHour, simMin, simSec);
    long remainingSec = prayerTimes.getTimeRemaining(nextP, simHour, simMin, simSec);

    long remHours = remainingSec / 3600;
    long remMins  = (remainingSec % 3600) / 60;
    long remSecs  = remainingSec % 60;

    Serial.print(F("Waktu Simulasi : 11:30:00\n"));
    Serial.print(F("Sholat Berikutnya : "));
    Serial.print(prayerTimes.getPrayerName(nextP));
    Serial.print(F(" ("));
    Serial.print(prayerTimes.getFormattedTime(nextP));
    Serial.println(F(")"));

    Serial.print(F("Hitung Mundur     : "));
    Serial.print(remHours); Serial.print(F(" Jam "));
    Serial.print(remMins);  Serial.print(F(" Menit "));
    Serial.print(remSecs);  Serial.println(F(" Detik"));
    Serial.println(F("========================================================\n"));
}

void loop() {
    // Pada aplikasi nyata, loop() membaca jam dari IskakINO_RTC atau IskakINO_FastNTP
    // dan memeriksa if (prayerTimes.isPrayerTimeNow(hour, min)) untuk memicu Buzzer / Adzan.
    delay(10000);
}
