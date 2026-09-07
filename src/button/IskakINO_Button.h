/*
 * src/button/IskakINO_Button.h
 * Modul driver tombol pintar non-blocking dengan deteksi gesture multi-aksi untuk IskakINO.
 *
 * Fitur:
 *  - Software debounce non-blocking otomatis.
 *  - Deteksi Single Click, Double Click, Multi-Click, dan Long Press (Hold).
 *  - Mendukung mode Active LOW (INPUT_PULLUP) dan Active HIGH (INPUT).
 *  - Gaya Polling (isClicked(), isDoubleClicked()) & Event Callback (onClick()).
 *  - Zero dynamic heap memory allocation.
 *  - Kompatibel lintas platform: AVR, ESP8266, & ESP32.
 */

#ifndef ISKAKINO_BUTTON_H
#define ISKAKINO_BUTTON_H

#include <Arduino.h>
#include "../core/IskakINO_Platform.h"

typedef void (*ButtonCallback)();

class IskakINO_Button {
public:
    // Konstruktor
    // pin: pin GPIO terhubung ke tombol
    // activeLow: true (default) jika tombol ditekan bernilai LOW
    // pullup: true (default) mengaktifkan resistor internal INPUT_PULLUP
    explicit IskakINO_Button(uint8_t pin = 255, bool activeLow = true, bool pullup = true);

    // Inisialisasi pin
    void begin();
    void begin(uint8_t pin, bool activeLow = true, bool pullup = true);

    // Wajib dipanggil di dalam loop() untuk memproses state-machine tombol
    void update();

    // --- Konfigurasi Parameter Timing (ms) ---
    void setDebounceMs(uint16_t ms);
    void setClickWindowMs(uint16_t ms);
    void setLongPressMs(uint16_t ms);

    // --- Polling Status Real-Time ---
    bool isPressed() const;       // Sedang dalam kondisi fisik tertekan
    bool isReleased() const;      // Sedang dalam kondisi fisik tidak tertekan
    bool stateChanged() const;    // Terjadi perubahan status pada update() terakhir
    bool isHolding() const;       // Sedang ditahan melebihi durasi long press
    uint32_t getHoldDuration() const; // Durasi (ms) penekanan saat ini

    // --- Polling Event (Event dikonsumsi sekali / auto-clear saat dibaca) ---
    bool isClicked();             // Single Click terdeteksi
    bool isDoubleClicked();       // Double Click terdeteksi
    bool isLongPressed();         // Long Press terpicu (sekali saat threshold tercapai)
    uint8_t getClickCount();      // Mengambil jumlah klik berturut-turut (1, 2, 3, ...)

    // --- Registrasi Event Callback (Opsional) ---
    void onClick(ButtonCallback cb);
    void onDoubleClick(ButtonCallback cb);
    void onLongPressStart(ButtonCallback cb);
    void onLongPressEnd(ButtonCallback cb);

private:
    uint8_t _pin;
    bool _activeLow;
    bool _pullup;
    bool _initialized;

    // Timing parameters
    uint16_t _debounceMs;
    uint16_t _clickWindowMs;
    uint16_t _longPressMs;

    // State machine
    bool _lastPhysicalState;
    bool _debouncedState;
    bool _stateChanged;
    uint32_t _lastDebounceTime;
    uint32_t _pressStartTime;

    // Click & Hold tracking
    uint8_t _clickCounter;
    uint8_t _reportedClicks;
    uint32_t _lastReleaseTime;
    bool _singleClicked;
    bool _doubleClicked;
    bool _longPressed;
    bool _longPressHandled;

    // Callbacks
    ButtonCallback _cbClick;
    ButtonCallback _cbDoubleClick;
    ButtonCallback _cbLongPressStart;
    ButtonCallback _cbLongPressEnd;

    bool _readRawPressed() const;
};

#endif // ISKAKINO_BUTTON_H
