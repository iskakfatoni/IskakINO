/*
 * src/filter/IskakINO_Filter.h
 * Modul pemrosesan sinyal & filter data sensor analog/jarak untuk ekosistem IskakINO.
 *
 * Berisi 4 alat filter performa tinggi:
 *  1. IskakINO_Kalman1D        : 1D Kalman Filter (Optimal untuk sensor bising seperti Ultrasonik HC-SR04, ToF, & Load Cell).
 *  2. IskakINO_MedianFilter<N> : Moving Median Filter (Menghilangkan outlier/lonjakan spike ekstrem).
 *  3. IskakINO_EMAFilter       : Exponential Moving Average Filter (Perataan data cepat hemat memori).
 *  4. IskakINO_LinearCalibrator: Multi-Point Piecewise Linear Calibration (Konversi nilai ADC ke unit fisik).
 *
 * Zero-Heap Dynamic Allocation & Kompatibel Universal (AVR, ESP8266, ESP32).
 */

#ifndef ISKAKINO_FILTER_H
#define ISKAKINO_FILTER_H

#include <Arduino.h>
#include <math.h>

// ============================================================================
// 1. 1D Kalman Filter (IskakINO_Kalman1D)
// ============================================================================
class IskakINO_Kalman1D {
private:
    float _q; // Process noise covariance
    float _r; // Measurement noise covariance
    float _x; // Estimated value
    float _p; // Estimation error covariance
    float _k; // Kalman gain
    bool _initialized;

public:
    // Konstruktor
    // q: process noise (default 0.125f)
    // r: measurement noise (default 4.0f)
    // p: initial estimation error (default 1.0f)
    explicit IskakINO_Kalman1D(float q = 0.125f, float r = 4.0f, float p = 1.0f)
        : _q(q), _r(r), _x(0.0f), _p(p), _k(0.0f), _initialized(false) {}

    void setParameters(float q, float r) {
        _q = q;
        _r = r;
    }

    void reset(float initialValue = 0.0f) {
        _x = initialValue;
        _p = 1.0f;
        _initialized = true;
    }

    float update(float measurement) {
        if (!_initialized) {
            _x = measurement;
            _initialized = true;
            return _x;
        }

        // 1. Prediction update
        _p = _p + _q;

        // 2. Measurement update (Kalman Gain)
        _k = _p / (_p + _r);
        _x = _x + _k * (measurement - _x);
        _p = (1.0f - _k) * _p;

        return _x;
    }

    float getEstimate() const { return _x; }
};

// ============================================================================
// 2. Moving Median Filter (IskakINO_MedianFilter<N>)
// N: Jumlah sampel window (harus ganjil: 3, 5, 7, atau 9)
// ============================================================================
template <uint8_t N = 5>
class IskakINO_MedianFilter {
private:
    float _buffer[N];
    uint8_t _index;
    uint8_t _count;

public:
    IskakINO_MedianFilter() : _index(0), _count(0) {
        for (uint8_t i = 0; i < N; i++) _buffer[i] = 0.0f;
    }

    void reset() {
        _index = 0;
        _count = 0;
    }

    float update(float val) {
        _buffer[_index] = val;
        _index = (_index + 1) % N;
        if (_count < N) _count++;

        // Salin ke buffer stack lokal untuk diurutkan (sorting)
        float sorted[N];
        for (uint8_t i = 0; i < _count; i++) sorted[i] = _buffer[i];

        // Simple Insertion Sort (sangat cepat untuk N kecil <= 9)
        for (uint8_t i = 1; i < _count; i++) {
            float key = sorted[i];
            int8_t j = i - 1;
            while (j >= 0 && sorted[j] > key) {
                sorted[j + 1] = sorted[j];
                j--;
            }
            sorted[j + 1] = key;
        }

        return sorted[_count / 2];
    }
};

// ============================================================================
// 3. Exponential Moving Average Filter (IskakINO_EMAFilter)
// ============================================================================
class IskakINO_EMAFilter {
private:
    float _alpha;
    float _filtered;
    bool _initialized;

public:
    // alpha: faktor penghalusan 0.0f (sangat halus) s/d 1.0f (tanpa filter)
    explicit IskakINO_EMAFilter(float alpha = 0.15f)
        : _alpha(alpha), _filtered(0.0f), _initialized(false) {}

    void setAlpha(float alpha) {
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;
        _alpha = alpha;
    }

    void reset(float initialValue = 0.0f) {
        _filtered = initialValue;
        _initialized = true;
    }

    float update(float val) {
        if (!_initialized) {
            _filtered = val;
            _initialized = true;
            return _filtered;
        }
        _filtered = (_alpha * val) + ((1.0f - _alpha) * _filtered);
        return _filtered;
    }

    float get() const { return _filtered; }
};

// ============================================================================
// 4. Multi-Point Piecewise Linear Calibrator (IskakINO_LinearCalibrator)
// Mengonversi input mentah ke nilai terkalibrasi berdasarkan titik referensi.
// ============================================================================
class IskakINO_LinearCalibrator {
public:
    static const uint8_t MAX_CAL_POINTS = 8;

private:
    struct Point {
        float raw;
        float calibrated;
    };

    Point _points[MAX_CAL_POINTS];
    uint8_t _count;

public:
    IskakINO_LinearCalibrator() : _count(0) {}

    void clear() {
        _count = 0;
    }

    bool addPoint(float rawInput, float calibratedOutput) {
        if (_count >= MAX_CAL_POINTS) return false;

        // Sisipkan titik secara terurut berdasarkan rawInput
        uint8_t insertIdx = _count;
        for (uint8_t i = 0; i < _count; i++) {
            if (rawInput < _points[i].raw) {
                insertIdx = i;
                break;
            }
        }

        for (uint8_t i = _count; i > insertIdx; i--) {
            _points[i] = _points[i - 1];
        }

        _points[insertIdx].raw = rawInput;
        _points[insertIdx].calibrated = calibratedOutput;
        _count++;
        return true;
    }

    float calibrate(float raw) const {
        if (_count == 0) return raw;
        if (_count == 1) return _points[0].calibrated;

        // Jika lebih kecil dari titik kalibrasi pertama (ekstrapolasi kiri)
        if (raw <= _points[0].raw) {
            float slope = (_points[1].calibrated - _points[0].calibrated) / (_points[1].raw - _points[0].raw);
            return _points[0].calibrated + slope * (raw - _points[0].raw);
        }

        // Jika lebih besar dari titik kalibrasi terakhir (ekstrapolasi kanan)
        if (raw >= _points[_count - 1].raw) {
            uint8_t last = _count - 1;
            float slope = (_points[last].calibrated - _points[last - 1].calibrated) / (_points[last].raw - _points[last - 1].raw);
            return _points[last].calibrated + slope * (raw - _points[last].raw);
        }

        // Interpolasi linear di antara dua titik acuan
        for (uint8_t i = 0; i < _count - 1; i++) {
            if (raw >= _points[i].raw && raw <= _points[i + 1].raw) {
                float t = (raw - _points[i].raw) / (_points[i + 1].raw - _points[i].raw);
                return _points[i].calibrated + t * (_points[i + 1].calibrated - _points[i].calibrated);
            }
        }

        return raw;
    }

    size_t getPointCount() const { return _count; }
};

#endif // ISKAKINO_FILTER_H
