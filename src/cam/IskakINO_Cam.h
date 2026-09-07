/*
 * src/cam/IskakINO_Cam.h
 * Modul driver Kamera ESP32 (ESP32-CAM, OV2640 / OV3660) untuk ekosistem IskakINO.
 *
 * Fitur:
 *  - Konfigurasi instan untuk modul ESP32-CAM AI-Thinker, M5Stack, WROVER, dll.
 *  - Auto-deteksi PSRAM untuk alokasi buffer frame resolusi tinggi (UXGA / HD / SVGA).
 *  - Kontrol lampu Flash LED onboard terintegrasi (GPIO 4) dengan pulse timer non-blocking.
 *  - Penyesuaian sensor mudah (kecerahan, kontras, saturasi, flip vertikal, mirror horizontal).
 *  - Manajemen frame buffer aman (capture & release).
 *  - Penjagaan platform (otomatis no-op di AVR & ESP8266 tanpa error kompilasi).
 */

#ifndef ISKAKINO_CAM_H
#define ISKAKINO_CAM_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_PLATFORM_ESP32) || defined(ESP32)

#include <Arduino.h>
#include "esp_camera.h"
#include "IskakINO_CamPins.h"

class IskakINO_Cam {
public:
    explicit IskakINO_Cam(int8_t flashPin = 4);

    // Inisialisasi modul kamera
    // model: CAM_MODEL_AI_THINKER (default)
    // frameSize: FRAMESIZE_QVGA, FRAMESIZE_VGA (default), FRAMESIZE_SVGA, FRAMESIZE_HD, FRAMESIZE_UXGA
    // jpegQuality: 10 s/d 63 (semakin kecil = kualitas gambar semakin tinggi)
    // fbCount: jumlah frame buffer (1 atau 2 jika PSRAM aktif)
    bool begin(IskakCamModel model = CAM_MODEL_AI_THINKER,
               framesize_t frameSize = FRAMESIZE_VGA,
               uint8_t jpegQuality = 12,
               uint8_t fbCount = 1);

    // Wajib dipanggil di loop() jika menggunakan flash pulse timer non-blocking
    void update();

    // --- Pengambilan Gambar (Capture & Free Buffer) ---
    camera_fb_t* capture();
    void release(camera_fb_t* fb);

    // --- Status & Informasi ---
    bool isInitialized() const;
    bool hasPSRAM() const;

    // --- Kontrol Lampu Flash LED Onboard ---
    void setFlashPin(int8_t pin);
    void flashOn();
    void flashOff();
    void flashPulse(uint16_t durationMs);

    // --- Penyesuaian Sensor Gambar (OV2640) ---
    bool setBrightness(int level); // -2 s/d 2
    bool setContrast(int level);   // -2 s/d 2
    bool setSaturation(int level); // -2 s/d 2
    bool setVFlip(bool flip);      // Balik gambar vertikal (180 derajat)
    bool setHMirror(bool mirror);  // Cermin horizontal
    bool setResolution(framesize_t size);

    sensor_t* getSensor();

private:
    bool _initialized;
    bool _hasPSRAM;
    int8_t _flashPin;
    bool _flashState;
    bool _flashPulsing;
    uint32_t _flashPulseStart;
    uint16_t _flashPulseDuration;
    camera_config_t _config;
};

#endif // defined(ISKAKINO_PLATFORM_ESP32)

#endif // ISKAKINO_CAM_H
