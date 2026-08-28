/*
 * 23_Sensors_Ultrasonic_Filtered.ino
 * Modul: IskakINO_Ultrasonic & IskakINO_ArduFast (UNIVERSAL: AVR, ESP32, ESP8266)
 *
 * Menunjukkan:
 *   1. Pengukuran jarak akurat dengan sensor ultrasonik HC-SR04
 *   2. Perbandingan nilai mentah (Raw) vs nilai tersaring (Filtered) dengan Moving Median Filter
 *   3. Pencegahan lonjakan nilai acak akibat pantulan gema akustik
 */

#include <IskakINO.h>

#define TRIG_PIN 5
#define ECHO_PIN 18 // Sesuaikan dengan pin board yang digunakan

IskakINO_ArduFast   fast;
IskakINO_Ultrasonic sonar;

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - Ultrasonic Filtered Demo   "));
    fast.log(F("========================================"));

    // Inisialisasi HC-SR04 (Maksimal jarak 400 cm)
    sonar.begin(TRIG_PIN, ECHO_PIN, 400);
}

void loop() {
    // Ukur jarak setiap 200 ms non-blocking
    if (fast.every(200, 0)) {
        float rawCm = sonar.getDistanceCm(false);     // Nilai mentah tanpa filter
        float filteredCm = sonar.getDistanceCm(true); // Nilai stabil tersaring Median Filter

        fast.logf(F("[Jarak] Raw: %.1f cm | Filtered: %.1f cm (%.0f mm)"),
                   rawCm, filteredCm, sonar.getDistanceMm(true));
    }
}
