/*
 * src/prayertimes/IskakINO_Hijri.h
 *
 * Algoritma Konversi Kalender Hijriah (Masehi ke Hijriah) dan
 * Kalkulator Arah Kiblat (Qibla Azimuth) berpresisi tinggi.
 *
 * Universal untuk semua platform (AVR, ESP32, ESP8266).
 */

#ifndef ISKAKINO_HIJRI_H
#define ISKAKINO_HIJRI_H

#include <Arduino.h>
#include <math.h>

struct IskakHijriDate {
    int day;
    int month;
    int year;

    const char* getMonthName() const {
        static const char* const monthNames[] = {
            "Muharram", "Safar", "Rabi'ul Awwal", "Rabi'ul Akhir",
            "Jumadil Ula", "Jumadil Akhir", "Rajab", "Sya'ban",
            "Ramadhan", "Syawwal", "Dzulqa'dah", "Dzulhijjah"
        };
        if (month >= 1 && month <= 12) {
            return monthNames[month - 1];
        }
        return "Unknown";
    }

    String toString() const {
        return String(day) + " " + getMonthName() + " " + String(year) + " H";
    }
};

class IskakINO_Hijri {
public:
    // Konversi tanggal Masehi ke Hijriah (Algoritma Tabular / Arithmatic Hijri)
    static IskakHijriDate toHijri(int year, int month, int day, int adjustmentDays = 0) {
        // 1. Hitung Julian Day
        long jd = gregorianToJD(year, month, day) + adjustmentDays;

        // 2. Hitung Tanggal Hijriah dari Julian Day (Epoch Hijriah: JD 1948439.5 = 16 Juli 622 M)
        long l = jd - 1948440 + 10632;
        long n = (l - 1) / 10631;
        l = l - 10631 * n + 354;
        long j = ((10985 - l) / 5316) * ((50 * l) / 17719) + (l / 5670) * ((43 * l) / 15238);
        l = l - ((30 - j) / 15) * ((17719 * j) / 50) - (j / 16) * ((15238 * j) / 43) + 29;
        
        int hMonth = (int)((24 * l) / 709);
        int hDay = (int)(l - ((709 * hMonth) / 24));
        int hYear = (int)(30 * n + j - 30);

        IskakHijriDate result;
        result.day = hDay;
        result.month = hMonth;
        result.year = hYear;
        return result;
    }

    // Hitung Julian Day Number
    static long gregorianToJD(int year, int month, int day) {
        if (month <= 2) {
            year -= 1;
            month += 12;
        }
        long a = year / 100;
        long b = 2 - a + (a / 4);
        return (long)(365.25 * (year + 4716)) + (long)(30.6001 * (month + 1)) + day + b - 1524;
    }

    // Hitung Arah Kiblat (Derajat Azimuth dari Utara Sejati searah jarum jam)
    // Koordinat Ka'bah: Lintang 21.422487° N, Bujur 39.826206° E
    static float calculateQibla(float latitude, float longitude) {
        const float kaabaLat = 21.422487f * (float)(M_PI / 180.0);
        const float kaabaLng = 39.826206f * (float)(M_PI / 180.0);

        float userLat = latitude * (float)(M_PI / 180.0);
        float userLng = longitude * (float)(M_PI / 180.0);

        float dLng = kaabaLng - userLng;

        float y = sinf(dLng);
        float x = cosf(userLat) * tanf(kaabaLat) - sinf(userLat) * cosf(dLng);

        float qiblaRad = atan2f(y, x);
        float qiblaDeg = qiblaRad * (float)(180.0 / M_PI);

        if (qiblaDeg < 0.0f) {
            qiblaDeg += 360.0f;
        }
        return qiblaDeg;
    }
};

#endif // ISKAKINO_HIJRI_H
