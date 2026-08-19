/*
 * 15_Filter_SensorSmoothing.ino
 * Modul: IskakINO_Filter & IskakINO_ArduFast (Universal)
 *
 * Demonstrasi perbandingan algoritma pemrosesan sinyal sensor & kalibrasi:
 *  1. Raw Noisy Signal    : Sinyal mentah dengan derau bising dan spike lonjakan liar.
 *  2. Moving Median Filter: Mengeliminasi data spike outlier ekstrem.
 *  3. 1D Kalman Filter    : Estimasi optimal dari pembacaan sensor bising kontinu.
 *  4. EMA Filter          : Exponential Moving Average super cepat & ringan.
 *  5. Linear Calibrator   : Mengonversi sinyal ADC ke besaran fisik terkalibrasi (misal Suhu °C).
 *
 * Format output kompatibel dengan Arduino IDE Serial Monitor & Serial Plotter.
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

IskakINO_ArduFast fast;

// Inisialisasi Filter
IskakINO_Kalman1D kalmanFilter(0.125f, 4.0f);   // Process noise Q=0.125, Measurement noise R=4.0
IskakINO_MedianFilter<5> medianFilter;          // Window size 5
IskakINO_EMAFilter emaFilter(0.15f);            // Alpha = 0.15
IskakINO_LinearCalibrator tempCalibrator;       // Kalibrator Suhu (°C)

float simAngle = 0.0f;
uint16_t sampleIndex = 0;

void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("       IskakINO - Advanced Signal Filters Showcase       "));
    fast.log(F("========================================================="));
    fast.log(F("[Tips] Buka Serial Plotter (Ctrl+Shift+L) untuk grafik visual."));

    // Konfigurasi Titik Kalibrasi: addPoint(raw_adc, suhu_celsius)
    // 0 ADC -> 0.0 C, 512 ADC -> 50.0 C, 1023 ADC -> 100.0 C
    tempCalibrator.addPoint(0.0f, 0.0f);
    tempCalibrator.addPoint(250.0f, 25.0f);
    tempCalibrator.addPoint(500.0f, 50.0f);
    tempCalibrator.addPoint(750.0f, 75.0f);
    tempCalibrator.addPoint(1023.0f, 100.0f);

    fast.log(F("Index\tRaw\tMedian\tKalman\tEMA\tCalibrated_Temp(C)"));
    fast.reset(0);
}

void loop() {
    // Jalankan sampling filter setiap 50 ms secara non-blocking
    if (fast.every(50, 0)) {
        sampleIndex++;

        // 1. Buat sinyal simulasi: Gelombang dasar 500 + sin(angle) * 200
        simAngle += 0.05f;
        if (simAngle > 6.28318f) simAngle -= 6.28318f;
        float baseSignal = 500.0f + sin(simAngle) * 200.0f;

        // 2. Tambahkan noise acak (-15 s/d +15)
        float noise = (float)(random(-150, 150)) / 10.0f;
        float rawValue = baseSignal + noise;

        // 3. Simulasikan lonjakan spike liar sesekali (setiap 40 sampel)
        if (sampleIndex % 40 == 0) {
            rawValue += 250.0f; // Spike outlier ekstrem
        }

        // 4. Proses melalui masing-masing filter
        float medianVal = medianFilter.update(rawValue);
        float kalmanVal = kalmanFilter.update(medianVal); // Cascaded Filter (Median -> Kalman)
        float emaVal    = emaFilter.update(rawValue);
        float tempC     = tempCalibrator.calibrate(kalmanVal);

        // 5. Cetak data (Format ramah Serial Plotter)
        fast.logf(F("%u\tRaw:%.1f\tMedian:%.1f\tKalman:%.1f\tEMA:%.1f\tTemp:%.2f\n"),
                  sampleIndex,
                  (double)rawValue,
                  (double)medianVal,
                  (double)kalmanVal,
                  (double)emaVal,
                  (double)tempC);
    }
}
