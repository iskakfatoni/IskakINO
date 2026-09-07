/*
 * src/cam/IskakINO_CamPins.h
 * Definisi pemetaan pin hardware modul kamera ESP32 (AI-Thinker, M5Stack, WROVER-KIT, ESP-EYE).
 */

#ifndef ISKAKINO_CAMPINS_H
#define ISKAKINO_CAMPINS_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_PLATFORM_ESP32) || defined(ESP32)

#include "esp_camera.h"

enum IskakCamModel : uint8_t {
    CAM_MODEL_AI_THINKER = 0, // Standar modul ESP32-CAM AI-Thinker (paling umum)
    CAM_MODEL_M5STACK_PSRAM,  // M5Stack Unit Cam / M5Camera
    CAM_MODEL_WROVER_KIT,     // ESP-WROVER-KIT
    CAM_MODEL_ESP_EYE         // ESP-EYE
};

// Helper untuk mengisi konfigurasi pin kamera esp_camera_config_t
inline void iskakApplyCamPinout(camera_config_t &config, IskakCamModel model) {
    switch (model) {
        case CAM_MODEL_AI_THINKER:
        default:
            config.pin_pwdn     = 32;
            config.pin_reset    = -1;
            config.pin_xclk     = 0;
            config.pin_sccb_sda = 26;
            config.pin_sccb_scl = 27;
            config.pin_d7       = 35;
            config.pin_d6       = 34;
            config.pin_d5       = 39;
            config.pin_d4       = 36;
            config.pin_d3       = 21;
            config.pin_d2       = 19;
            config.pin_d1       = 18;
            config.pin_d0       = 5;
            config.pin_vsync    = 25;
            config.pin_href     = 23;
            config.pin_pclk     = 22;
            break;

        case CAM_MODEL_M5STACK_PSRAM:
            config.pin_pwdn     = -1;
            config.pin_reset    = 15;
            config.pin_xclk     = 27;
            config.pin_sccb_sda = 25;
            config.pin_sccb_scl = 23;
            config.pin_d7       = 19;
            config.pin_d6       = 36;
            config.pin_d5       = 18;
            config.pin_d4       = 39;
            config.pin_d3       = 5;
            config.pin_d2       = 34;
            config.pin_d1       = 35;
            config.pin_d0       = 32;
            config.pin_vsync    = 22;
            config.pin_href     = 26;
            config.pin_pclk     = 21;
            break;

        case CAM_MODEL_WROVER_KIT:
            config.pin_pwdn     = -1;
            config.pin_reset    = -1;
            config.pin_xclk     = 21;
            config.pin_sccb_sda = 26;
            config.pin_sccb_scl = 27;
            config.pin_d7       = 35;
            config.pin_d6       = 34;
            config.pin_d5       = 39;
            config.pin_d4       = 36;
            config.pin_d3       = 19;
            config.pin_d2       = 18;
            config.pin_d1       = 5;
            config.pin_d0       = 4;
            config.pin_vsync    = 25;
            config.pin_href     = 23;
            config.pin_pclk     = 22;
            break;

        case CAM_MODEL_ESP_EYE:
            config.pin_pwdn     = -1;
            config.pin_reset    = -1;
            config.pin_xclk     = 4;
            config.pin_sccb_sda = 18;
            config.pin_sccb_scl = 23;
            config.pin_d7       = 36;
            config.pin_d6       = 37;
            config.pin_d5       = 38;
            config.pin_d4       = 39;
            config.pin_d3       = 35;
            config.pin_d2       = 14;
            config.pin_d1       = 13;
            config.pin_d0       = 34;
            config.pin_vsync    = 5;
            config.pin_href     = 27;
            config.pin_pclk     = 25;
            break;
    }
}

#endif // defined(ISKAKINO_PLATFORM_ESP32)

#endif // ISKAKINO_CAMPINS_H
