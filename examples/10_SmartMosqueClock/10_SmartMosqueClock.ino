/*
 * 10_SmartMosqueClock.ino
 * Modul: IskakINO_PrayerTimes & IskakINO_ArduFast (Universal)
 *
 * Demonstrasi mesin hisab jadwal sholat presisi, kalender Hijriah & arah kiblat:
 *  1. Standar Resmi Kemenag RI (Subuh -20°, Isya -18°) dengan waktu pengaman (Ihtiyath).
 *  2. Konversi Kalender Masehi ke Penanggalan Hijriah otomatis.
 *  3. Perhitungan Arah Kiblat astronomis dari koordinat lintang & bujur.
 *  4. Deteksi Waktu Sholat Berikutnya & Hitung Mundur (Countdown Adzan / Iqomah).
 *
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

IskakINO_ArduFast    fast;
IskakINO_PrayerTimes prayerTimes;

// Koordinat Lokasi: Jakarta, Indonesia
const float LATITUDE   = -6.2088f;
const float LONGITUDE  = 106.8456f;
const float TIMEZONE   = 7.0f;  // WIB (UTC+7)
const float ELEVATION  = 25.0f; // 25 mdpl

void setup() {
    fast.begin(115200);

    fast.log(F("========================================================"));
    fast.log(F("   IskakINO - Smart Mosque Clock (JWS Engine)           "));
    fast.log(F("========================================================"));

    // 1. Konfigurasi Lokasi & Metode Perhitungan
    prayerTimes.setLocation(LATITUDE, LONGITUDE, TIMEZONE, ELEVATION);
    prayerTimes.setMethod(IskakPrayerMethod::KEMENAG); // Kemenag RI
    prayerTimes.setIkhtiyat(2);                        // +2 Menit Waktu Pengaman

    // 2. Hitung Waktu Sholat untuk Tanggal Tertentu (Contoh: 30 Agustus 2026)
    int year = 2026;
    int month = 8;
    int day = 30;
    prayerTimes.compute(year, month, day);

    // 3. Tampilkan Kalender Hijriah & Arah Kiblat
    IskakHijriDate hijri = prayerTimes.getHijriDate(year, month, day);
    fast.logf(F("[Kalender] Masehi: %d-%d-%d | Hijriah: %s"), day, month, year, hijri.toString().c_str());
    fast.logf(F("[Kiblat] Arah: %.1f° dari Utara (Searah Jarum Jam)"), prayerTimes.getQiblaDirection());

    fast.log(F("\n--- Jadwal Waktu Sholat Hari Ini (Standar Kemenag RI) ---"));
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
        fast.logf(F(" • %-12s : %s"), prayerTimes.getPrayerName(p), prayerTimes.getFormattedTime(p).c_str());
    }
    fast.log(F("--------------------------------------------------------\n"));

    // 4. Simulasi Hitung Mundur Sholat Berikutnya (Contoh jam 11:30:00)
    uint8_t simHour = 11;
    uint8_t simMin  = 30;
    uint8_t simSec  = 0;

    IskakPrayer nextP = prayerTimes.getNextPrayer(simHour, simMin, simSec);
    long remainingSec = prayerTimes.getTimeRemaining(nextP, simHour, simMin, simSec);

    long remHours = remainingSec / 3600;
    long remMins  = (remainingSec % 3600) / 60;
    long remSecs  = remainingSec % 60;

    fast.logf(F("[Next Prayer] Sholat: %s (%s) | Sisa Waktu: %02ld:%02ld:%02ld"),
              prayerTimes.getPrayerName(nextP),
              prayerTimes.getFormattedTime(nextP).c_str(),
              remHours, remMins, remSecs);
}

void loop() {
    // Pada implementasi JWS nyata, periksa jadwal sholat setiap detik
    // dengan jam yang dibaca dari IskakINO_RTC atau IskakINO_FastNTP.
    if (fast.every(10000, 0)) {
        fast.log(F("[JWS Monitor] Mesin hisab sholat aktif dan siap diintegrasikan dengan display & audio."));
    }
}
