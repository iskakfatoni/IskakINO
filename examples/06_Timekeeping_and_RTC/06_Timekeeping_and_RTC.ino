/*
 * 06_Timekeeping_and_RTC.ino
 * Modul: IskakINO_RTC, IskakINO_FastNTP, & IskakINO_PrayerTimes (Universal)
 *
 * Demonstrasi terpadu manajemen waktu, kalender, dan hisab sholat:
 *  1. Hardware RTC        : Auto-deteksi multi-chip (DS3231, DS1307, PCF8563), baca jam, & kalender.
 *  2. FastNTP (WiFi)      : Auto-sinkronisasi jam internet presisi tinggi (ESP8266 & ESP32).
 *  3. Hybrid Time Sync    : Jam fisik RTC otomatis disinkronkan dari NTP; fallback offline instan.
 *  4. Prayer Times Engine : Perhitungan jadwal sholat Kemenag RI, kalender Hijriah & arah kiblat.
 *
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

IskakINO_ArduFast    fast;
IskakINO_RTC         rtc;
IskakINO_PrayerTimes prayerEngine;

#if defined(ISKAKINO_HAS_WIFI)
  WiFiUDP          ntpUdp;
  IskakINO_FastNTP ntp(ntpUdp, "pool.ntp.org");
  const char* WIFI_SSID = "YOUR_WIFI_SSID";
  const char* WIFI_PASS = "YOUR_WIFI_PASS";
#endif

// Koordinat Lokasi: Jakarta, Indonesia
const float LATITUDE   = -6.2088f;
const float LONGITUDE  = 106.8456f;
const float TIMEZONE   = 7.0f; // WIB (UTC+7)
const float ELEVATION  = 25.0f;

void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("    IskakINO - Timekeeping, RTC & Prayer Times Suite     "));
    fast.log(F("========================================================="));

    // 1. Inisialisasi Hardware RTC
    if (rtc.begin()) {
        fast.logf(F("[RTC] Hardware RTC terdeteksi: %s"), rtc.chipName());
        if (rtc.lostPower()) {
            fast.log(F("[RTC] Daya sempat hilang! Mengatur waktu awal default 2026-09-01 12:00:00"));
            rtc.setDateTime(2026, 9, 1, 12, 0, 0);
        }
    } else {
        fast.log(F("[RTC] Tidak ada hardware RTC di bus I2C. Berjalan dalam mode simulasi."));
    }

    // 2. Inisialisasi Mesin Jadwal Sholat Kemenag RI
    prayerEngine.setLocation(LATITUDE, LONGITUDE, TIMEZONE, ELEVATION);
    prayerEngine.setMethod(IskakPrayerMethod::KEMENAG);
    prayerEngine.setIkhtiyat(2); // +2 menit pengaman

    fast.logf(F("[Qibla] Arah Kiblat: %.1f° dari Utara"), prayerEngine.getQiblaDirection());

#if defined(ISKAKINO_HAS_WIFI)
    // 3. Hubungkan WiFi & FastNTP pada ESP32 / ESP8266
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    ntp.begin(25200); // UTC+7 = 25200 detik
    rtc.syncWithNTP(ntp, 3600000UL); // Auto-sync jam RTC dari NTP tiap 1 jam
    fast.log(F("[WiFi] Menghubungkan ke WiFi untuk sinkronisasi FastNTP..."));
#endif
}

void loop() {
#if defined(ISKAKINO_HAS_WIFI)
    ntp.update();
    rtc.tick();
#endif

    // Update dan tampilkan waktu setiap 2 detik
    if (fast.every(2000, 0)) {
        IskakDateTime dt = rtc.now();

        // Hitung jadwal sholat untuk hari ini
        prayerEngine.compute(dt.year, dt.month, dt.day);
        IskakHijriDate hijri = prayerEngine.getHijriDate(dt.year, dt.month, dt.day);

        fast.logf(F("[Waktu] %s %s | %s Hijriah"),
                  dt.getDateString(true).c_str(),
                  dt.getTimeString(true).c_str(),
                  hijri.toString().c_str());

        fast.logf(F("[Jadwal Sholat] Subuh: %s | Dzuhur: %s | Ashar: %s | Maghrib: %s | Isya: %s"),
                  prayerEngine.getFormattedTime(IskakPrayer::FAJR).c_str(),
                  prayerEngine.getFormattedTime(IskakPrayer::DHUHR).c_str(),
                  prayerEngine.getFormattedTime(IskakPrayer::ASR).c_str(),
                  prayerEngine.getFormattedTime(IskakPrayer::MAGHRIB).c_str(),
                  prayerEngine.getFormattedTime(IskakPrayer::ISHA).c_str());
    }
}
