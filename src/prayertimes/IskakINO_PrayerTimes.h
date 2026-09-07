/*
 * src/prayertimes/IskakINO_PrayerTimes.h
 *
 * Mesin kalkulasi waktu sholat astronomis presisi tinggi, zero-dependency,
 * ultra-hemat RAM, dan universal (AVR, ESP32, ESP8266).
 *
 * Mendukung standar resmi Kemenag RI, MWL, Makkah, Egypt, ISNA, Karachi,
 * koreksi elevasi, waktu pengaman (ikhtiyat), kalender Hijriah, dan arah kiblat.
 */

#ifndef ISKAKINO_PRAYERTIMES_H
#define ISKAKINO_PRAYERTIMES_H

#include "../core/IskakINO_Platform.h"
#include "IskakINO_Hijri.h"
#include <Arduino.h>

enum class IskakPrayer : uint8_t {
    IMSAK    = 0,
    FAJR     = 1, // Subuh
    SUNRISE  = 2, // Syuruq
    DHUHA    = 3, // Dhuha
    DHUHR    = 4, // Dzuhur
    ASR      = 5, // Ashar
    SUNSET   = 6, // Terbenam
    MAGHRIB  = 7, // Maghrib
    ISHA     = 8, // Isya
    MIDNIGHT = 9, // Tengah Malam
    COUNT    = 10
};

enum class IskakPrayerMethod : uint8_t {
    KEMENAG = 0, // Kementerian Agama RI (Fajr -20.0°, Isha -18.0°) - Default Indonesia
    MWL     = 1, // Muslim World League (Fajr -18.0°, Isha -17.0°)
    EGYPT   = 2, // Egyptian General Authority (Fajr -19.5°, Isha -17.5°)
    MAKKAH  = 3, // Umm al-Qura University, Makkah (Fajr -18.5°, Isha +90 min)
    ISNA    = 4, // Islamic Society of North America (Fajr -15.0°, Isha -15.0°)
    KARACHI = 5, // University of Islamic Sciences, Karachi (Fajr -18.0°, Isha -18.0°)
    CUSTOM  = 6  // Kustom pengguna
};

enum class IskakAsrJuristic : uint8_t {
    SHAFI  = 1, // Syafi'i, Maliki, Hambali (Panjang bayangan = 1x panjang benda)
    HANAFI = 2  // Hanafi (Panjang bayangan = 2x panjang benda)
};

struct IskakPrayerTime {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    float toDecimalHours() const {
        return (float)hour + ((float)minute / 60.0f) + ((float)second / 3600.0f);
    }

    String toString() const {
        char buf[9];
        snprintf(buf, sizeof(buf), "%02u:%02u", hour, minute);
        return String(buf);
    }

    String toStringWithSeconds() const {
        char buf[9];
        snprintf(buf, sizeof(buf), "%02u:%02u:%02u", hour, minute, second);
        return String(buf);
    }
};

class IskakINO_PrayerTimes {
private:
    float _latitude;
    float _longitude;
    float _timezone;
    float _elevation; // Meter di atas permukaan laut (mdpl)

    IskakPrayerMethod _method;
    IskakAsrJuristic _asrJuristic;

    float _fajrAngle;
    float _ishaAngle;
    bool  _ishaIsMinutes;
    float _ishaMinutes;

    int   _ikhtiyatMinutes;    // Waktu pengaman (default +2 menit)
    int   _imsakMinutesBefore; // Menit imsak sebelum subuh (default 10 menit)
    int   _dhuhaMinutesAfter;  // Menit dhuha setelah syuruq (default 20 menit)
    int   _hijriAdjustment;    // Koreksi hari Hijriah (default 0)

    // Nilai hasil komputasi (dalam jam desimal 0.0 - 24.0)
    float _computedTimes[(size_t)IskakPrayer::COUNT];
    int   _lastYear;
    int   _lastMonth;
    int   _lastDay;

    // Helper kalkulasi astronomis internal
    static float fixAngle(float a);
    static float fixHour(float a);
    static float dsin(float d);
    static float dcos(float d);
    static float dtan(float d);
    static float dasin(float x);
    static float dacos(float x);
    static float datan2(float y, float x);
    static float dacot(float x);

    void calculateSunPosition(float jd, float &declination, float &equationOfTime);
    float computeTime(float angle, float dec, float eqt);
    float computeAsr(int step, float dec, float eqt);

public:
    IskakINO_PrayerTimes();

    // Konfigurasi Lokasi & Geografis
    void setLocation(float latitude, float longitude, float timezone = 7.0f, float elevation = 0.0f);
    void setElevation(float elevationMeters) { _elevation = elevationMeters; }
    void setTimezone(float timezoneHours)   { _timezone = timezoneHours; }

    // Konfigurasi Metode Perhitungan
    void setMethod(IskakPrayerMethod method);
    void setCustomMethod(float fajrAngle, float ishaAngle, bool ishaIsMinutes = false, float ishaMinutes = 90.0f);
    void setAsrJuristic(IskakAsrJuristic juristic) { _asrJuristic = juristic; }

    // Koreksi & Pengaman
    void setIkhtiyat(int minutes) { _ikhtiyatMinutes = minutes; }
    void setImsakMinutesBefore(int minutes) { _imsakMinutesBefore = minutes; }
    void setDhuhaMinutesAfter(int minutes)  { _dhuhaMinutesAfter = minutes; }
    void setHijriAdjustment(int days)       { _hijriAdjustment = days; }

    // Komputasi Jadwal Sholat untuk Tanggal Tertentu
    void compute(int year, int month, int day);

    // Dapatkan Waktu Sholat
    IskakPrayerTime getTime(IskakPrayer prayer) const;
    float getDecimalTime(IskakPrayer prayer) const;
    String getFormattedTime(IskakPrayer prayer) const;
    const char* getPrayerName(IskakPrayer prayer) const;

    // Helper Jam Masjid (JWS)
    IskakPrayer getNextPrayer(uint8_t currentHour, uint8_t currentMinute, uint8_t currentSecond = 0) const;
    long getTimeRemaining(IskakPrayer prayer, uint8_t currentHour, uint8_t currentMinute, uint8_t currentSecond = 0) const;
    bool isPrayerTimeNow(uint8_t currentHour, uint8_t currentMinute, uint8_t thresholdSeconds = 59) const;
    IskakPrayer getCurrentPrayer() const;

    // Arah Kiblat & Kalender Hijriah
    float getQiblaDirection() const {
        return IskakINO_Hijri::calculateQibla(_latitude, _longitude);
    }

    IskakHijriDate getHijriDate(int year, int month, int day) const {
        return IskakINO_Hijri::toHijri(year, month, day, _hijriAdjustment);
    }
};

#endif // ISKAKINO_PRAYERTIMES_H
