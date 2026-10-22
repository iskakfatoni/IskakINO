/*
 * src/cam/IskakINO_CamModule.h
 * Adapter modular supaya IskakINO_Cam bisa didaftarkan ke IskakINO_Kernel.
 */

#ifndef ISKAKINO_CAM_MODULE_H
#define ISKAKINO_CAM_MODULE_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_PLATFORM_ESP32) || defined(ESP32)

#include "../core/IskakINO_Module.h"
#include "IskakINO_Cam.h"

class IskakINO_CamModule : public IskakINO_Module {
  private:
    IskakINO_Cam& _cam;
    IskakCamModel _model;
    framesize_t _frameSize;
    uint8_t _jpegQuality;
    uint8_t _fbCount;

  public:
    explicit IskakINO_CamModule(IskakINO_Cam& cam,
                                IskakCamModel model = CAM_MODEL_AI_THINKER,
                                framesize_t frameSize = FRAMESIZE_VGA,
                                uint8_t jpegQuality = 12,
                                uint8_t fbCount = 1)
        : _cam(cam), _model(model), _frameSize(frameSize), _jpegQuality(jpegQuality), _fbCount(fbCount) {}

    void begin() override {
        _cam.begin(_model, _frameSize, _jpegQuality, _fbCount);
    }

    void update() override {
        _cam.update();
    }

    const char* moduleName() const override {
        return "Camera";
    }
};

#endif // defined(ISKAKINO_PLATFORM_ESP32)

#endif // ISKAKINO_CAM_MODULE_H
