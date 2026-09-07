/*
 * src/relay/IskakINO_Relay.h
 * Modul driver Relay & Aktuator Pintar non-blocking untuk ekosistem IskakINO.
 *
 * Fitur:
 *  - Mendukung modul relay Active HIGH & Active LOW.
 *  - Status booting aman (dijamin OFF saat startup).
 *  - Pulse Timer Non-Blocking: pulse(3000) menyala 3 detik lalu mati sendiri.
 *  - Blink Cadence Non-Blocking: blink(onMs, offMs, repeatCount).
 *  - Proteksi Switching Chatter: setMinSwitchInterval().
 *  - Kompatibel lintas platform: AVR, ESP8266, & ESP32.
 */

#ifndef ISKAKINO_RELAY_H
#define ISKAKINO_RELAY_H

#include <Arduino.h>
#include "../core/IskakINO_Platform.h"

enum IskakRelayMode : uint8_t {
    ISKAK_RELAY_ACTIVE_HIGH = 0, // Relay ON saat pin bernilai HIGH
    ISKAK_RELAY_ACTIVE_LOW  = 1  // Relay ON saat pin bernilai LOW (standar modul relay optocoupler)
};

class IskakINO_Relay {
public:
    // Konstruktor
    // pin: pin GPIO terhubung ke relay
    // mode: ISKAK_RELAY_ACTIVE_LOW (default) atau ISKAK_RELAY_ACTIVE_HIGH
    // initialOn: status awal saat inisialisasi (default false / OFF)
    explicit IskakINO_Relay(uint8_t pin = 255,
                            IskakRelayMode mode = ISKAK_RELAY_ACTIVE_LOW,
                            bool initialOn = false);

    // Inisialisasi pin
    void begin();
    void begin(uint8_t pin, IskakRelayMode mode = ISKAK_RELAY_ACTIVE_LOW, bool initialOn = false);

    // Wajib dipanggil di dalam loop() untuk memproses timer pulse & blink
    void update();

    // --- Kontrol Dasar ---
    void on();
    void off();
    void toggle();
    void set(bool state);

    // --- Pulse & Blink Non-Blocking ---
    // pulse(): Menyalakan relay selama durationMs lalu otomatis mati sendiri
    void pulse(uint32_t durationMs);

    // blink(): Menyalakan dan mematikan relay secara berkala
    // onMs: durasi ON (ms)
    // offMs: durasi OFF (ms)
    // repeatCount: jumlah siklus blink (0 = berulang terus tanpa henti)
    void blink(uint32_t onMs, uint32_t offMs, uint8_t repeatCount = 0);

    // Menghentikan efek pulse / blink dan mematikan relay
    void stop();

    // --- Konfigurasi Keamanan ---
    // Mencegah relay berganti status terlalu cepat untuk melindungi kontak mekanis
    void setMinSwitchInterval(uint32_t intervalMs);

    // --- Status Query ---
    bool isOn() const;
    bool isOff() const;
    bool isPulsing() const;
    bool isBlinking() const;

private:
    uint8_t _pin;
    IskakRelayMode _mode;
    bool _state;        // true = ON, false = OFF
    bool _initialOn;
    bool _initialized;

    // Safety switch throttle
    uint32_t _minSwitchIntervalMs;
    uint32_t _lastSwitchTime;

    // Pulse state
    bool _isPulsing;
    uint32_t _pulseStartTime;
    uint32_t _pulseDurationMs;

    // Blink state
    bool _isBlinking;
    uint32_t _blinkTimerMark;
    uint32_t _blinkOnMs;
    uint32_t _blinkOffMs;
    uint8_t _blinkTargetCount;
    uint8_t _blinkCurrentCount;

    void _hardwareWrite(bool state);
};

#endif // ISKAKINO_RELAY_H
