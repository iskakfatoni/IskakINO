/*
 * src/buzzer/IskakINO_Buzzer.cpp
 * Implementasi driver Buzzer Non-Blocking untuk ekosistem IskakINO.
 */

#include "IskakINO_Buzzer.h"
#include <ctype.h>

#define PRESET_SUCCESS      1
#define PRESET_ERROR        2
#define PRESET_WARNING      3
#define PRESET_NOTIFICATION 4
#define PRESET_ALARM        5

IskakINO_Buzzer::IskakINO_Buzzer(uint8_t pin, IskakBuzzerType type, bool activeHigh)
    : _pin(pin), _type(type), _activeHigh(activeHigh), _muted(false), _initialized(false),
      _state(STATE_IDLE), _source(SRC_NONE), _timerMark(0), _currentDuration(0), _gapDuration(15),
      _seqNotes(nullptr), _seqDurations(nullptr), _seqCount(0), _seqIndex(0),
      _presetType(0), _presetStep(0), _presetRepeat(0),
      _rtttlPtr(nullptr), _rtttlDefaultDuration(4), _rtttlDefaultOctave(6), _rtttlBpm(63), _rtttlWholenoteMs(3809) {
}

void IskakINO_Buzzer::begin() {
    if (_pin != 255) {
        pinMode(_pin, OUTPUT);
        _hardwareNoTone();
        _initialized = true;
    }
}

void IskakINO_Buzzer::begin(uint8_t pin, IskakBuzzerType type, bool activeHigh) {
    _pin = pin;
    _type = type;
    _activeHigh = activeHigh;
    begin();
}

void IskakINO_Buzzer::stop() {
    _state = STATE_IDLE;
    _source = SRC_NONE;
    _hardwareNoTone();
    _rtttlPtr = nullptr;
    _seqNotes = nullptr;
    _seqDurations = nullptr;
}

bool IskakINO_Buzzer::isPlaying() const {
    return (_state != STATE_IDLE);
}

void IskakINO_Buzzer::setMute(bool mute) {
    _muted = mute;
    if (_muted) {
        _hardwareNoTone();
    }
}

bool IskakINO_Buzzer::isMuted() const {
    return _muted;
}

void IskakINO_Buzzer::_hardwareTone(uint16_t frequency) {
    if (!_initialized || _muted || _pin == 255) return;

    if (_type == ISKAK_BUZZER_ACTIVE) {
        if (frequency > 0) {
            digitalWrite(_pin, _activeHigh ? HIGH : LOW);
        } else {
            digitalWrite(_pin, _activeHigh ? LOW : HIGH);
        }
    } else {
        if (frequency > 0) {
            tone(_pin, frequency);
        } else {
            noTone(_pin);
        }
    }
}

void IskakINO_Buzzer::_hardwareNoTone() {
    if (!_initialized || _pin == 255) return;

    if (_type == ISKAK_BUZZER_ACTIVE) {
        digitalWrite(_pin, _activeHigh ? LOW : HIGH);
    } else {
        noTone(_pin);
    }
}

void IskakINO_Buzzer::beep(uint16_t durationMs, uint16_t frequency) {
    stop();
    _source = SRC_BEEP;
    _state = STATE_PLAYING_NOTE;
    _currentDuration = durationMs;
    _timerMark = millis();
    _hardwareTone(frequency);
}

void IskakINO_Buzzer::playSuccess() {
    stop();
    _source = SRC_PRESET;
    _presetType = PRESET_SUCCESS;
    _presetStep = 0;
    _presetRepeat = 1;
    _startPresetStep();
}

void IskakINO_Buzzer::playError() {
    stop();
    _source = SRC_PRESET;
    _presetType = PRESET_ERROR;
    _presetStep = 0;
    _presetRepeat = 1;
    _startPresetStep();
}

void IskakINO_Buzzer::playWarning() {
    stop();
    _source = SRC_PRESET;
    _presetType = PRESET_WARNING;
    _presetStep = 0;
    _presetRepeat = 1;
    _startPresetStep();
}

void IskakINO_Buzzer::playNotification() {
    stop();
    _source = SRC_PRESET;
    _presetType = PRESET_NOTIFICATION;
    _presetStep = 0;
    _presetRepeat = 1;
    _startPresetStep();
}

void IskakINO_Buzzer::playAlarm(uint8_t repeatCount) {
    stop();
    _source = SRC_PRESET;
    _presetType = PRESET_ALARM;
    _presetStep = 0;
    _presetRepeat = (repeatCount > 0) ? repeatCount : 1;
    _startPresetStep();
}

void IskakINO_Buzzer::_startPresetStep() {
    uint16_t freq = 0;
    uint16_t dur = 0;

    switch (_presetType) {
        case PRESET_SUCCESS:
            if (_presetStep == 0) { freq = NOTE_C5; dur = 80; }
            else if (_presetStep == 1) { freq = NOTE_E5; dur = 80; }
            else if (_presetStep == 2) { freq = NOTE_G5; dur = 80; }
            else if (_presetStep == 3) { freq = NOTE_C6; dur = 180; }
            else { stop(); return; }
            break;

        case PRESET_ERROR:
            if (_presetStep == 0) { freq = NOTE_G3; dur = 100; }
            else if (_presetStep == 1) { freq = NOTE_E3; dur = 100; }
            else if (_presetStep == 2) { freq = NOTE_C3; dur = 220; }
            else { stop(); return; }
            break;

        case PRESET_WARNING:
            if (_presetStep == 0) { freq = NOTE_A4; dur = 120; }
            else if (_presetStep == 1) { freq = 0;       dur = 60; }
            else if (_presetStep == 2) { freq = NOTE_A4; dur = 120; }
            else { stop(); return; }
            break;

        case PRESET_NOTIFICATION:
            if (_presetStep == 0) { freq = NOTE_E5; dur = 70; }
            else if (_presetStep == 1) { freq = NOTE_G5; dur = 140; }
            else { stop(); return; }
            break;

        case PRESET_ALARM:
            if (_presetStep % 2 == 0) {
                freq = NOTE_A5; dur = 140;
            } else {
                freq = NOTE_E5; dur = 140;
            }
            if (_presetStep >= (_presetRepeat * 2)) {
                stop();
                return;
            }
            break;

        default:
            stop();
            return;
    }

    _state = STATE_PLAYING_NOTE;
    _currentDuration = dur;
    _gapDuration = 15;
    _timerMark = millis();
    _hardwareTone(freq);
}

void IskakINO_Buzzer::playSequence(const uint16_t* notes, const uint16_t* durationsMs, size_t count) {
    if (!notes || !durationsMs || count == 0) return;

    stop();
    _source = SRC_SEQUENCE;
    _seqNotes = notes;
    _seqDurations = durationsMs;
    _seqCount = count;
    _seqIndex = 0;

    _state = STATE_PLAYING_NOTE;
    _currentDuration = _seqDurations[0];
    _gapDuration = 15;
    _timerMark = millis();
    _hardwareTone(_seqNotes[0]);
}

char IskakINO_Buzzer::_readRtttlChar() {
    if (!_rtttlPtr) return '\0';
    if (_source == SRC_RTTTL_PROGMEM) {
        return (char)pgm_read_byte(_rtttlPtr);
    }
    return *_rtttlPtr;
}

void IskakINO_Buzzer::_advanceRtttl() {
    if (_rtttlPtr) {
        _rtttlPtr++;
    }
}

void IskakINO_Buzzer::playRTTTL(const char* rtttl) {
    if (!rtttl) return;
    stop();
    _source = SRC_RTTTL_RAM;
    _rtttlPtr = rtttl;

    // 1. Lewati nama melodi hingga ':' pertama
    while (_readRtttlChar() != '\0' && _readRtttlChar() != ':') {
        _advanceRtttl();
    }
    if (_readRtttlChar() == ':') _advanceRtttl();

    // 2. Baca default values (d=4,o=6,b=63)
    _rtttlDefaultDuration = 4;
    _rtttlDefaultOctave = 6;
    _rtttlBpm = 63;

    while (_readRtttlChar() != '\0' && _readRtttlChar() != ':') {
        char c = tolower(_readRtttlChar());
        if (c == 'd') {
            _advanceRtttl();
            while (_readRtttlChar() == ' ' || _readRtttlChar() == '=') _advanceRtttl();
            uint8_t num = 0;
            while (isdigit(_readRtttlChar())) {
                num = (num * 10) + (_readRtttlChar() - '0');
                _advanceRtttl();
            }
            if (num > 0) _rtttlDefaultDuration = num;
        } else if (c == 'o') {
            _advanceRtttl();
            while (_readRtttlChar() == ' ' || _readRtttlChar() == '=') _advanceRtttl();
            uint8_t num = 0;
            while (isdigit(_readRtttlChar())) {
                num = (num * 10) + (_readRtttlChar() - '0');
                _advanceRtttl();
            }
            if (num > 0) _rtttlDefaultOctave = num;
        } else if (c == 'b') {
            _advanceRtttl();
            while (_readRtttlChar() == ' ' || _readRtttlChar() == '=') _advanceRtttl();
            uint16_t num = 0;
            while (isdigit(_readRtttlChar())) {
                num = (num * 10) + (_readRtttlChar() - '0');
                _advanceRtttl();
            }
            if (num > 0) _rtttlBpm = num;
        } else {
            _advanceRtttl();
        }
    }
    if (_readRtttlChar() == ':') _advanceRtttl();

    // Kalkulasi durasi 1 not penuh (wholenote) dalam milidetik
    // 1 bar = 4 ketukan. Waktu 1 ketukan = (60000 / BPM) ms.
    _rtttlWholenoteMs = (60000UL * 4UL) / _rtttlBpm;

    // Parse not pertama
    _parseNextRtttlNote();
}

void IskakINO_Buzzer::playRTTTL(const __FlashStringHelper* rtttl) {
    if (!rtttl) return;
    stop();
    _source = SRC_RTTTL_PROGMEM;
    _rtttlPtr = (const char*)rtttl;

    while (_readRtttlChar() != '\0' && _readRtttlChar() != ':') {
        _advanceRtttl();
    }
    if (_readRtttlChar() == ':') _advanceRtttl();

    _rtttlDefaultDuration = 4;
    _rtttlDefaultOctave = 6;
    _rtttlBpm = 63;

    while (_readRtttlChar() != '\0' && _readRtttlChar() != ':') {
        char c = tolower(_readRtttlChar());
        if (c == 'd') {
            _advanceRtttl();
            while (_readRtttlChar() == ' ' || _readRtttlChar() == '=') _advanceRtttl();
            uint8_t num = 0;
            while (isdigit(_readRtttlChar())) {
                num = (num * 10) + (_readRtttlChar() - '0');
                _advanceRtttl();
            }
            if (num > 0) _rtttlDefaultDuration = num;
        } else if (c == 'o') {
            _advanceRtttl();
            while (_readRtttlChar() == ' ' || _readRtttlChar() == '=') _advanceRtttl();
            uint8_t num = 0;
            while (isdigit(_readRtttlChar())) {
                num = (num * 10) + (_readRtttlChar() - '0');
                _advanceRtttl();
            }
            if (num > 0) _rtttlDefaultOctave = num;
        } else if (c == 'b') {
            _advanceRtttl();
            while (_readRtttlChar() == ' ' || _readRtttlChar() == '=') _advanceRtttl();
            uint16_t num = 0;
            while (isdigit(_readRtttlChar())) {
                num = (num * 10) + (_readRtttlChar() - '0');
                _advanceRtttl();
            }
            if (num > 0) _rtttlBpm = num;
        } else {
            _advanceRtttl();
        }
    }
    if (_readRtttlChar() == ':') _advanceRtttl();

    _rtttlWholenoteMs = (60000UL * 4UL) / _rtttlBpm;

    _parseNextRtttlNote();
}

uint16_t IskakINO_Buzzer::_getNoteFrequency(char note, bool sharp, uint8_t octave) {
    if (note == 'p' || note == 'P') return 0;

    // Frekuensi dasar pada Oktaf 4 (C4 - B4)
    uint16_t baseFreq = 0;
    switch (tolower(note)) {
        case 'c': baseFreq = sharp ? NOTE_CS4 : NOTE_C4; break;
        case 'd': baseFreq = sharp ? NOTE_DS4 : NOTE_D4; break;
        case 'e': baseFreq = NOTE_E4; break; // Tidak ada E# di standar tangga nada umum
        case 'f': baseFreq = sharp ? NOTE_FS4 : NOTE_F4; break;
        case 'g': baseFreq = sharp ? NOTE_GS4 : NOTE_G4; break;
        case 'a': baseFreq = sharp ? NOTE_AS4 : NOTE_A4; break;
        case 'b': baseFreq = NOTE_B4; break;
        default: return 0;
    }

    if (octave >= 4) {
        return baseFreq << (octave - 4);
    } else {
        return baseFreq >> (4 - octave);
    }
}

void IskakINO_Buzzer::_parseNextRtttlNote() {
    // Lewati spasi atau koma pemisah
    while (_readRtttlChar() == ' ' || _readRtttlChar() == ',') {
        _advanceRtttl();
    }

    if (_readRtttlChar() == '\0') {
        stop();
        return;
    }

    // 1. Baca durasi spesifik (jika ada)
    uint8_t durVal = 0;
    while (isdigit(_readRtttlChar())) {
        durVal = (durVal * 10) + (_readRtttlChar() - '0');
        _advanceRtttl();
    }
    if (durVal == 0) durVal = _rtttlDefaultDuration;

    // 2. Baca nama not nada (a, b, c, d, e, f, g, p)
    char noteChar = _readRtttlChar();
    _advanceRtttl();

    // 3. Cek tanda kres (#)
    bool isSharp = false;
    if (_readRtttlChar() == '#') {
        isSharp = true;
        _advanceRtttl();
    }

    // 4. Cek tanda titik/dotted (.) sebelum oktaf
    bool isDotted = false;
    if (_readRtttlChar() == '.') {
        isDotted = true;
        _advanceRtttl();
    }

    // 5. Baca oktaf spesifik (jika ada)
    uint8_t octVal = 0;
    if (isdigit(_readRtttlChar())) {
        octVal = _readRtttlChar() - '0';
        _advanceRtttl();
    }
    if (octVal == 0) octVal = _rtttlDefaultOctave;

    // 6. Cek tanda titik/dotted (.) setelah oktaf
    if (_readRtttlChar() == '.') {
        isDotted = true;
        _advanceRtttl();
    }

    // Lewati karakter sisa sampai koma / akhir string
    while (_readRtttlChar() != '\0' && _readRtttlChar() != ',') {
        _advanceRtttl();
    }

    // Kalkulasi durasi not dalam milidetik
    uint32_t noteDuration = _rtttlWholenoteMs / durVal;
    if (isDotted) {
        noteDuration += (noteDuration / 2);
    }

    uint16_t freq = _getNoteFrequency(noteChar, isSharp, octVal);

    _state = STATE_PLAYING_NOTE;
    _currentDuration = (uint16_t)noteDuration;
    _gapDuration = (noteDuration > 50) ? 15 : 5;
    _timerMark = millis();
    _hardwareTone(freq);
}

void IskakINO_Buzzer::update() {
    if (_state == STATE_IDLE) return;

    uint32_t now = millis();

    if (_state == STATE_PLAYING_NOTE) {
        if (now - _timerMark >= _currentDuration) {
            _hardwareNoTone();
            _state = STATE_GAP;
            _timerMark = now;
        }
    } else if (_state == STATE_GAP) {
        if (now - _timerMark >= _gapDuration) {
            if (_source == SRC_BEEP) {
                stop();
            } else if (_source == SRC_PRESET) {
                _presetStep++;
                _startPresetStep();
            } else if (_source == SRC_SEQUENCE) {
                _seqIndex++;
                if (_seqIndex < _seqCount) {
                    _state = STATE_PLAYING_NOTE;
                    _currentDuration = _seqDurations[_seqIndex];
                    _gapDuration = 15;
                    _timerMark = millis();
                    _hardwareTone(_seqNotes[_seqIndex]);
                } else {
                    stop();
                }
            } else if (_source == SRC_RTTTL_RAM || _source == SRC_RTTTL_PROGMEM) {
                _parseNextRtttlNote();
            } else {
                stop();
            }
        }
    }
}
