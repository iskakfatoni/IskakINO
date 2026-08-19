/*
 * src/relay/IskakINO_Relay.cpp
 * Implementasi driver Relay & Aktuator Pintar untuk IskakINO.
 */

#include "IskakINO_Relay.h"

IskakINO_Relay::IskakINO_Relay(uint8_t pin, IskakRelayMode mode, bool initialOn)
    : _pin(pin), _mode(mode), _state(false), _initialOn(initialOn), _initialized(false),
      _minSwitchIntervalMs(0), _lastSwitchTime(0),
      _isPulsing(false), _pulseStartTime(0), _pulseDurationMs(0),
      _isBlinking(false), _blinkTimerMark(0), _blinkOnMs(0), _blinkOffMs(0),
      _blinkTargetCount(0), _blinkCurrentCount(0) {
}

void IskakINO_Relay::begin() {
    if (_pin != 255) {
        // Set ke status aman awal sebelum pinMode untuk mencegah glitch saat booting
        _hardwareWrite(_initialOn);
        pinMode(_pin, OUTPUT);
        _state = _initialOn;
        _initialized = true;
    }
}

void IskakINO_Relay::begin(uint8_t pin, IskakRelayMode mode, bool initialOn) {
    _pin = pin;
    _mode = mode;
    _initialOn = initialOn;
    begin();
}

void IskakINO_Relay::_hardwareWrite(bool state) {
    if (_pin == 255) return;
    if (_mode == ISKAK_RELAY_ACTIVE_LOW) {
        digitalWrite(_pin, state ? LOW : HIGH);
    } else {
        digitalWrite(_pin, state ? HIGH : LOW);
    }
}

void IskakINO_Relay::set(bool state) {
    if (!_initialized || _pin == 255) return;

    // Cek proteksi interval perpindahan minimum
    uint32_t now = millis();
    if (_minSwitchIntervalMs > 0 && (now - _lastSwitchTime < _minSwitchIntervalMs)) {
        return;
    }

    _state = state;
    _hardwareWrite(_state);
    _lastSwitchTime = now;
}

void IskakINO_Relay::on() {
    _isPulsing = false;
    _isBlinking = false;
    set(true);
}

void IskakINO_Relay::off() {
    _isPulsing = false;
    _isBlinking = false;
    set(false);
}

void IskakINO_Relay::toggle() {
    _isPulsing = false;
    _isBlinking = false;
    set(!_state);
}

void IskakINO_Relay::pulse(uint32_t durationMs) {
    if (durationMs == 0) return;

    _isBlinking = false;
    set(true);
    _isPulsing = true;
    _pulseStartTime = millis();
    _pulseDurationMs = durationMs;
}

void IskakINO_Relay::blink(uint32_t onMs, uint32_t offMs, uint8_t repeatCount) {
    if (onMs == 0 || offMs == 0) return;

    _isPulsing = false;
    _blinkOnMs = onMs;
    _blinkOffMs = offMs;
    _blinkTargetCount = repeatCount;
    _blinkCurrentCount = 0;
    _isBlinking = true;
    _blinkTimerMark = millis();

    set(true);
}

void IskakINO_Relay::stop() {
    _isPulsing = false;
    _isBlinking = false;
    set(false);
}

void IskakINO_Relay::setMinSwitchInterval(uint32_t intervalMs) {
    _minSwitchIntervalMs = intervalMs;
}

bool IskakINO_Relay::isOn() const {
    return _state;
}

bool IskakINO_Relay::isOff() const {
    return !_state;
}

bool IskakINO_Relay::isPulsing() const {
    return _isPulsing;
}

bool IskakINO_Relay::isBlinking() const {
    return _isBlinking;
}

void IskakINO_Relay::update() {
    if (!_initialized || _pin == 255) return;

    uint32_t now = millis();

    // 1. Proses Pulse Timer
    if (_isPulsing) {
        if (now - _pulseStartTime >= _pulseDurationMs) {
            _isPulsing = false;
            set(false);
        }
    }

    // 2. Proses Blink Cadence
    if (_isBlinking) {
        if (_state) {
            // Sedang fase ON, tunggu durasi ON selesai
            if (now - _blinkTimerMark >= _blinkOnMs) {
                set(false);
                _blinkTimerMark = now;
            }
        } else {
            // Sedang fase OFF, tunggu durasi OFF selesai
            if (now - _blinkTimerMark >= _blinkOffMs) {
                _blinkCurrentCount++;
                if (_blinkTargetCount > 0 && (_blinkCurrentCount >= _blinkTargetCount)) {
                    _isBlinking = false;
                } else {
                    set(true);
                    _blinkTimerMark = now;
                }
            }
        }
    }
}
