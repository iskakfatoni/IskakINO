/*
 * src/button/IskakINO_Button.cpp
 * Implementasi driver tombol pintar non-blocking untuk IskakINO.
 */

#include "IskakINO_Button.h"

IskakINO_Button::IskakINO_Button(uint8_t pin, bool activeLow, bool pullup)
    : _pin(pin), _activeLow(activeLow), _pullup(pullup), _initialized(false),
      _debounceMs(40), _clickWindowMs(250), _longPressMs(600),
      _lastPhysicalState(false), _debouncedState(false), _stateChanged(false),
      _lastDebounceTime(0), _pressStartTime(0),
      _clickCounter(0), _reportedClicks(0), _lastReleaseTime(0),
      _singleClicked(false), _doubleClicked(false), _longPressed(false), _longPressHandled(false),
      _cbClick(nullptr), _cbDoubleClick(nullptr), _cbLongPressStart(nullptr), _cbLongPressEnd(nullptr) {
}

void IskakINO_Button::begin() {
    if (_pin != 255) {
        if (_pullup) {
            pinMode(_pin, INPUT_PULLUP);
        } else {
            pinMode(_pin, INPUT);
        }
        _lastPhysicalState = _readRawPressed();
        _debouncedState = _lastPhysicalState;
        _initialized = true;
    }
}

void IskakINO_Button::begin(uint8_t pin, bool activeLow, bool pullup) {
    _pin = pin;
    _activeLow = activeLow;
    _pullup = pullup;
    begin();
}

void IskakINO_Button::setDebounceMs(uint16_t ms) {
    _debounceMs = ms;
}

void IskakINO_Button::setClickWindowMs(uint16_t ms) {
    _clickWindowMs = ms;
}

void IskakINO_Button::setLongPressMs(uint16_t ms) {
    _longPressMs = ms;
}

bool IskakINO_Button::_readRawPressed() const {
    if (_pin == 255) return false;
    int raw = digitalRead(_pin);
    return _activeLow ? (raw == LOW) : (raw == HIGH);
}

bool IskakINO_Button::isPressed() const {
    return _debouncedState;
}

bool IskakINO_Button::isReleased() const {
    return !_debouncedState;
}

bool IskakINO_Button::stateChanged() const {
    return _stateChanged;
}

bool IskakINO_Button::isHolding() const {
    return _debouncedState && _longPressHandled;
}

uint32_t IskakINO_Button::getHoldDuration() const {
    if (!_debouncedState) return 0;
    return millis() - _pressStartTime;
}

bool IskakINO_Button::isClicked() {
    bool res = _singleClicked;
    _singleClicked = false;
    return res;
}

bool IskakINO_Button::isDoubleClicked() {
    bool res = _doubleClicked;
    _doubleClicked = false;
    return res;
}

bool IskakINO_Button::isLongPressed() {
    bool res = _longPressed;
    _longPressed = false;
    return res;
}

uint8_t IskakINO_Button::getClickCount() {
    uint8_t count = _reportedClicks;
    _reportedClicks = 0;
    return count;
}

void IskakINO_Button::onClick(ButtonCallback cb) {
    _cbClick = cb;
}

void IskakINO_Button::onDoubleClick(ButtonCallback cb) {
    _cbDoubleClick = cb;
}

void IskakINO_Button::onLongPressStart(ButtonCallback cb) {
    _cbLongPressStart = cb;
}

void IskakINO_Button::onLongPressEnd(ButtonCallback cb) {
    _cbLongPressEnd = cb;
}

void IskakINO_Button::update() {
    if (!_initialized || _pin == 255) return;

    uint32_t now = millis();
    bool rawPressed = _readRawPressed();
    _stateChanged = false;

    // 1. Software Debouncing
    if (rawPressed != _lastPhysicalState) {
        _lastDebounceTime = now;
        _lastPhysicalState = rawPressed;
    }

    if ((now - _lastDebounceTime) >= _debounceMs) {
        if (rawPressed != _debouncedState) {
            _debouncedState = rawPressed;
            _stateChanged = true;

            if (_debouncedState) {
                // Transisi dari RELEASED ke PRESSED
                _pressStartTime = now;
                _longPressHandled = false;
            } else {
                // Transisi dari PRESSED ke RELEASED
                if (!_longPressHandled) {
                    _clickCounter++;
                    _lastReleaseTime = now;
                } else {
                    if (_cbLongPressEnd) {
                        _cbLongPressEnd();
                    }
                }
            }
        }
    }

    // 2. Deteksi Long Press (Saat tombol masih ditahan)
    if (_debouncedState) {
        if (!_longPressHandled && (now - _pressStartTime >= _longPressMs)) {
            _longPressed = true;
            _longPressHandled = true;
            _clickCounter = 0; // Batalkan akumulasi multi-click jika terjadi hold
            if (_cbLongPressStart) {
                _cbLongPressStart();
            }
        }
    }

    // 3. Evaluasi Jendela Multi-Click (Saat tombol sudah dilepas)
    if (!_debouncedState && _clickCounter > 0) {
        if (now - _lastReleaseTime >= _clickWindowMs) {
            _reportedClicks = _clickCounter;

            if (_clickCounter == 1) {
                _singleClicked = true;
                if (_cbClick) _cbClick();
            } else if (_clickCounter == 2) {
                _doubleClicked = true;
                if (_cbDoubleClick) _cbDoubleClick();
            }

            _clickCounter = 0;
        }
    }
}
