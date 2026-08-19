/*
 * src/cam/IskakINO_Cam.cpp
 * Implementasi driver modul kamera ESP32 (ESP32-CAM) untuk IskakINO.
 */

#include "IskakINO_Cam.h"

#if defined(ISKAKINO_PLATFORM_ESP32) || defined(ESP32)

IskakINO_Cam::IskakINO_Cam(int8_t flashPin)
    : _initialized(false), _hasPSRAM(false), _flashPin(flashPin),
      _flashState(false), _flashPulsing(false), _flashPulseStart(0), _flashPulseDuration(0) {
}

bool IskakINO_Cam::begin(IskakCamModel model, framesize_t frameSize, uint8_t jpegQuality, uint8_t fbCount) {
    _hasPSRAM = psramFound();

    _config.ledc_channel = LEDC_CHANNEL_0;
    _config.ledc_timer   = LEDC_TIMER_0;
    _config.pixel_format = PIXFORMAT_JPEG;
    _config.xclk_freq_hz = 20000000;
    _config.frame_size   = frameSize;
    _config.jpeg_quality = jpegQuality;
    _config.fb_count     = _hasPSRAM ? fbCount : 1;
    _config.fb_location  = _hasPSRAM ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;
    _config.grab_mode    = CAMERA_GRAB_LATEST;

    // Terapkan pinout sesuai model board
    iskakApplyCamPinout(_config, model);

    // Inisialisasi driver kamera ESP-IDF
    esp_err_t err = esp_camera_init(&_config);
    if (err != ESP_OK) {
        _initialized = false;
        return false;
    }

    _initialized = true;

    // Inisialisasi pin flash jika ditentukan
    if (_flashPin >= 0) {
        pinMode(_flashPin, OUTPUT);
        digitalWrite(_flashPin, LOW);
    }

    return true;
}

camera_fb_t* IskakINO_Cam::capture() {
    if (!_initialized) return nullptr;
    return esp_camera_fb_get();
}

void IskakINO_Cam::release(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

bool IskakINO_Cam::isInitialized() const {
    return _initialized;
}

bool IskakINO_Cam::hasPSRAM() const {
    return _hasPSRAM;
}

void IskakINO_Cam::setFlashPin(int8_t pin) {
    _flashPin = pin;
    if (_flashPin >= 0) {
        pinMode(_flashPin, OUTPUT);
        digitalWrite(_flashPin, LOW);
    }
}

void IskakINO_Cam::flashOn() {
    if (_flashPin >= 0) {
        _flashPulsing = false;
        _flashState = true;
        digitalWrite(_flashPin, HIGH);
    }
}

void IskakINO_Cam::flashOff() {
    if (_flashPin >= 0) {
        _flashPulsing = false;
        _flashState = false;
        digitalWrite(_flashPin, LOW);
    }
}

void IskakINO_Cam::flashPulse(uint16_t durationMs) {
    if (_flashPin >= 0 && durationMs > 0) {
        digitalWrite(_flashPin, HIGH);
        _flashState = true;
        _flashPulsing = true;
        _flashPulseStart = millis();
        _flashPulseDuration = durationMs;
    }
}

void IskakINO_Cam::update() {
    if (_flashPulsing && _flashPin >= 0) {
        if (millis() - _flashPulseStart >= _flashPulseDuration) {
            digitalWrite(_flashPin, LOW);
            _flashState = false;
            _flashPulsing = false;
        }
    }
}

sensor_t* IskakINO_Cam::getSensor() {
    if (!_initialized) return nullptr;
    return esp_camera_sensor_get();
}

bool IskakINO_Cam::setBrightness(int level) {
    sensor_t* s = getSensor();
    if (!s) return false;
    return (s->set_brightness(s, level) == 0);
}

bool IskakINO_Cam::setContrast(int level) {
    sensor_t* s = getSensor();
    if (!s) return false;
    return (s->set_contrast(s, level) == 0);
}

bool IskakINO_Cam::setSaturation(int level) {
    sensor_t* s = getSensor();
    if (!s) return false;
    return (s->set_saturation(s, level) == 0);
}

bool IskakINO_Cam::setVFlip(bool flip) {
    sensor_t* s = getSensor();
    if (!s) return false;
    return (s->set_vflip(s, flip ? 1 : 0) == 0);
}

bool IskakINO_Cam::setHMirror(bool mirror) {
    sensor_t* s = getSensor();
    if (!s) return false;
    return (s->set_hmirror(s, mirror ? 1 : 0) == 0);
}

bool IskakINO_Cam::setResolution(framesize_t size) {
    sensor_t* s = getSensor();
    if (!s) return false;
    return (s->set_framesize(s, size) == 0);
}

#endif // defined(ISKAKINO_PLATFORM_ESP32)
