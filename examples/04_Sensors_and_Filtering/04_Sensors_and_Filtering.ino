/*
 * 04_Sensors_and_Filtering.ino
 * Modul: IskakINO_Sensors (DHT, DS18B20, Ultrasonic) & IskakINO_Filter (Universal)
 *
 * Demonstrasi membaca sensor lingkungan dan memproses datanya dengan filter:
 *  1. IskakINO_DHT         : Suhu, kelembapan, & Heat Index (DHT11 / DHT22).
 *  2. IskakINO_DS18B20     : Sensor suhu presisi tinggi waterproof 1-Wire.
 *  3. IskakINO_Ultrasonic  : Pengukuran jarak akustik HC-SR04 dengan filter internal.
 *  4. IskakINO_Filter Suite:
 *     - IskakINO_MedianFilter : Menghilangkan lonjakan data outlier/noise spike ekstrem.
 *     - IskakINO_Kalman1D     : Estimasi optimal dari pembacaan sensor bising.
 *     - IskakINO_EMAFilter    : Exponential Moving Average untuk kehalusan sinyal.
 *     - IskakINO_LinearCalibrator: Kalibrasi titik bertingkat (Piecewise Linear).
 *
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

// Definisi Pin Sensor
#if defined(ESP32)
  const uint8_t DHT_PIN      = 4;
  const uint8_t ONE_WIRE_PIN = 16;
  const uint8_t TRIG_PIN     = 5;
  const uint8_t ECHO_PIN     = 18;
#elif defined(ESP8266)
  const uint8_t DHT_PIN      = D4; // GPIO2
  const uint8_t ONE_WIRE_PIN = D2; // GPIO4
  const uint8_t TRIG_PIN     = D5; // GPIO14
  const uint8_t ECHO_PIN     = D6; // GPIO12
#else
  const uint8_t DHT_PIN      = 4;  // Arduino Uno D4
  const uint8_t ONE_WIRE_PIN = 2;  // Arduino Uno D2
  const uint8_t TRIG_PIN     = 5;  // Arduino Uno D5
  const uint8_t ECHO_PIN     = 6;  // Arduino Uno D6
#endif

IskakINO_ArduFast   fast;
IskakINO_DHT        dht;
IskakINO_DS18B20    temp1Wire;
IskakINO_Ultrasonic sonar;

// Objek Filter
IskakINO_MedianFilter<5> medianFilter;
IskakINO_Kalman1D        kalmanFilter(0.125f, 4.0f);
IskakINO_EMAFilter       emaFilter(0.2f);

void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("    IskakINO - Sensors & Signal Filtering Suite          "));
    fast.log(F("========================================================="));

    // 1. Inisialisasi DHT (DHT11 standar)
    dht.begin(DHT_PIN, IskakDHTType::DHT11);

    // 2. Inisialisasi DS18B20 1-Wire
    temp1Wire.begin(ONE_WIRE_PIN);

    // 3. Inisialisasi HC-SR04 Ultrasonic (Max range 400 cm)
    sonar.begin(TRIG_PIN, ECHO_PIN, 400);

    fast.log(F("[Sensors] Inisialisasi selesai. Memulai pembacaan...\n"));
}

void loop() {
    // Task 1: Baca Ultrasonic & uji filter setiap 200 ms
    if (fast.every(200, 0)) {
        float rawDist = sonar.getDistanceCm(false); // Mentah
        float medDist = medianFilter.update(rawDist);
        float kalDist = kalmanFilter.update(rawDist);

        fast.logf(F("[Sonar HC-SR04] Raw: %.1f cm | Median: %.1f cm | Kalman: %.1f cm"),
                  rawDist, medDist, kalDist);
    }

    // Task 2: Baca DHT11 & DS18B20 setiap 2000 ms
    if (fast.every(2000, 1)) {
        // Baca DHT
        if (dht.read()) {
            float temp = dht.getTemperature();
            float hum  = dht.getHumidity();
            float hi   = dht.getHeatIndex();
            float smoothTemp = emaFilter.update(temp);

            fast.logf(F("[DHT11] Suhu: %.1f °C (Smooth: %.1f °C) | Kelembapan: %.1f %% | Heat Index: %.1f °C"),
                      temp, smoothTemp, hum, hi);
        } else {
            fast.log(F("[DHT11] Menunggu sinyal sensor atau periksa pin..."));
        }

        // Baca DS18B20
        if (temp1Wire.read()) {
            fast.logf(F("[DS18B20] Suhu 1-Wire: %.2f °C (%.2f °F)"),
                      temp1Wire.getTemperatureC(), temp1Wire.getTemperatureF());
        }
    }
}
