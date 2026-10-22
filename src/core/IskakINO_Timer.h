/*
 * src/core/IskakINO_Timer.h
 * Timer stopwatch & countdown non-blocking mandiri (ad-hoc / standalone).
 *
 * Berbeda dengan IskakINO_Scheduler yang mengelola tabel task berbasis ID,
 * IskakINO_Timer adalah objek timer per-instance yang sangat ringan (8 byte),
 * ideal untuk timeout komunikasi, debounce, durasi jeda, dan stopwatch.
 */

#ifndef ISKAKINO_TIMER_H
#define ISKAKINO_TIMER_H

#include <Arduino.h>

class IskakINO_Timer {
  public:
    // durationMs: Durasi timer dalam milidetik (0 jika murni sebagai stopwatch)
    explicit IskakINO_Timer(uint32_t durationMs = 0)
        : _duration(durationMs), _startTime(0), _running(false) {}

    // Memulai timer dari titik sekarang
    void start() {
        _startTime = millis();
        _running = true;
    }

    // Memulai timer dengan menetapkan durasi baru sekaligus
    void start(uint32_t durationMs) {
        _duration = durationMs;
        start();
    }

    // Menghentikan timer
    void stop() {
        _running = false;
    }

    // Memulai ulang timer dari titik sekarang
    void restart() {
        start();
    }

    // Menghentikan dan mereset waktu timer
    void reset() {
        _startTime = millis();
        _running = false;
    }

    // Memeriksa apakah durasi hitung mundur telah tercapai/habis.
    // Jika timer habis, mengembalikan true.
    bool hasExpired() const {
        if (!_running) return false;
        return (millis() - _startTime >= _duration);
    }

    // Operator boolean: return true jika timer masih berjalan dan belum habis.
    explicit operator bool() const {
        return (_running && !hasExpired());
    }

    // Durasi waktu yang telah berjalan sejak start() (milidetik).
    uint32_t elapsed() const {
        if (!_running) return 0;
        return (millis() - _startTime);
    }

    // Sisa waktu hitung mundur menuju nol (milidetik).
    uint32_t remaining() const {
        if (!_running) return 0;
        uint32_t el = elapsed();
        return (el >= _duration) ? 0 : (_duration - el);
    }

    bool isRunning() const { return _running; }
    void setDuration(uint32_t durationMs) { _duration = durationMs; }
    uint32_t duration() const { return _duration; }

  private:
    uint32_t _duration;
    uint32_t _startTime;
    bool     _running;
};

/**
 * @brief Varian timer presisi tinggi berbasis mikrodetik (micros()).
 * Cocok untuk timing kritis sensor pulsa ultrasonik, bit-bang sinyal, dsb.
 */
class IskakINO_TimerMicros {
  public:
    explicit IskakINO_TimerMicros(uint32_t durationUs = 0)
        : _duration(durationUs), _startTime(0), _running(false) {}

    void start() {
        _startTime = micros();
        _running = true;
    }

    void start(uint32_t durationUs) {
        _duration = durationUs;
        start();
    }

    void stop() { _running = false; }
    void restart() { start(); }
    void reset() { _startTime = micros(); _running = false; }

    bool hasExpired() const {
        if (!_running) return false;
        return (micros() - _startTime >= _duration);
    }

    uint32_t elapsed() const {
        if (!_running) return 0;
        return (micros() - _startTime);
    }

    uint32_t remaining() const {
        if (!_running) return 0;
        uint32_t el = elapsed();
        return (el >= _duration) ? 0 : (_duration - el);
    }

    bool isRunning() const { return _running; }
    void setDuration(uint32_t durationUs) { _duration = durationUs; }
    uint32_t duration() const { return _duration; }

  private:
    uint32_t _duration;
    uint32_t _startTime;
    bool     _running;
};

#endif // ISKAKINO_TIMER_H
