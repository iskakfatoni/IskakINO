/*
 * src/buzzer/IskakINO_Buzzer.h
 * Modul driver Buzzer Non-Blocking untuk ekosistem IskakINO.
 *
 * Fitur:
 *  - 100% Non-Blocking (dijalankan via update() di loop tanpa delay).
 *  - Kompatibel lintas platform: Arduino AVR, ESP8266, & ESP32.
 *  - Mendukung Passive Buzzer (nada frekuensi PWM) & Active Buzzer (digital ON/OFF).
 *  - Nada status terintegrasi (Success, Error, Warning, Notification, Alarm).
 *  - Pemutar format RTTTL (Ring Tone Text Transfer Language - Nokia Ringtone)
 *    langsung dari RAM atau PROGMEM/Flash tanpa alokasi memori dinamis.
 *  - Pemutar array nada kustom (Custom Tone Sequence).
 *  - Kontrol mute, stop, dan pemantauan status isPlaying().
 */

#ifndef ISKAKINO_BUZZER_H
#define ISKAKINO_BUZZER_H

#include <Arduino.h>
#include "../core/IskakINO_Platform.h"
#include "IskakINO_Pitches.h"

// Tipe hardware buzzer
enum IskakBuzzerType : uint8_t {
    ISKAK_BUZZER_PASSIVE = 0, // Menggunakan sinyal frekuensi (tone / PWM)
    ISKAK_BUZZER_ACTIVE  = 1  // Menggunakan sinyal digital HIGH / LOW
};

class IskakINO_Buzzer {
public:
    // Konstruktor
    // pin: pin GPIO terhubung ke buzzer
    // type: ISKAK_BUZZER_PASSIVE (default) atau ISKAK_BUZZER_ACTIVE
    // activeHigh: true (default) jika active buzzer menyala saat HIGH, false jika LOW
    explicit IskakINO_Buzzer(uint8_t pin = 255,
                            IskakBuzzerType type = ISKAK_BUZZER_PASSIVE,
                            bool activeHigh = true);

    // Inisialisasi pin
    void begin();
    void begin(uint8_t pin, IskakBuzzerType type = ISKAK_BUZZER_PASSIVE, bool activeHigh = true);

    // Wajib dipanggil di dalam loop() untuk memproses transisi nada non-blocking
    void update();

    // Menghentikan suara seketika
    void stop();

    // Status apakah buzzer sedang aktif berbunyi
    bool isPlaying() const;

    // Mode hening (mute)
    void setMute(bool mute);
    bool isMuted() const;

    // --- 1. Generator Beep Tunggal ---
    void beep(uint16_t durationMs = 80, uint16_t frequency = 2000);

    // --- 2. Preset Nada Status Instan ---
    void playSuccess();       // Nada ceria akor naik (C5-E5-G5-C6)
    void playError();         // Nada rendah 3x beruntun (G3-E3-C3)
    void playWarning();       // Nada peringatan 2x (A4)
    void playNotification();  // Nada notifikasi 2 nada (E5-G5)
    void playAlarm(uint8_t repeatCount = 3); // Sirine peringatan bergantian (A5-E5)

    // --- 3. Custom Sequence Player ---
    // notes: array frekuensi (Hz)
    // durationsMs: array durasi tiap nada (ms)
    // count: jumlah nada dalam array
    void playSequence(const uint16_t* notes, const uint16_t* durationsMs, size_t count);

    // --- 4. RTTTL Melody Player (Nokia Ringtone Format) ---
    // Mendukung string RTTTL dari RAM maupun PROGMEM (F("..."))
    void playRTTTL(const char* rtttl);
    void playRTTTL(const __FlashStringHelper* rtttl);

private:
    enum SourceType : uint8_t {
        SRC_NONE = 0,
        SRC_BEEP,
        SRC_PRESET,
        SRC_SEQUENCE,
        SRC_RTTTL_RAM,
        SRC_RTTTL_PROGMEM
    };

    enum State : uint8_t {
        STATE_IDLE = 0,
        STATE_PLAYING_NOTE,
        STATE_GAP
    };

    uint8_t _pin;
    IskakBuzzerType _type;
    bool _activeHigh;
    bool _muted;
    bool _initialized;

    State _state;
    SourceType _source;
    uint32_t _timerMark;
    uint16_t _currentDuration;
    uint16_t _gapDuration;

    // Pointer data sequence
    const uint16_t* _seqNotes;
    const uint16_t* _seqDurations;
    size_t _seqCount;
    size_t _seqIndex;

    // Data preset internal
    uint8_t _presetType;
    uint8_t _presetStep;
    uint8_t _presetRepeat;

    // RTTTL parser state
    const char* _rtttlPtr;
    uint8_t _rtttlDefaultDuration;
    uint8_t _rtttlDefaultOctave;
    uint16_t _rtttlBpm;
    uint32_t _rtttlWholenoteMs;

    // Helper internal
    void _hardwareTone(uint16_t frequency);
    void _hardwareNoTone();
    char _readRtttlChar();
    void _advanceRtttl();
    void _startPresetStep();
    void _parseNextRtttlNote();
    uint16_t _getNoteFrequency(char note, bool sharp, uint8_t octave);
};

#endif // ISKAKINO_BUZZER_H
