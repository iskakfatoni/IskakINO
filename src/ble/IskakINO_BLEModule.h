#ifndef ISKAKINO_BLE_MODULE_H
#define ISKAKINO_BLE_MODULE_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_PLATFORM_ESP32)

#include "../core/IskakINO_Module.h"
#include "IskakINO_BLE.h"

class IskakINO_BLEModule : public IskakINO_Module {
  private:
    IskakINO_BLE& _ble;
    const char* _deviceName;

  public:
    explicit IskakINO_BLEModule(IskakINO_BLE& ble, const char* deviceName = "IskakINO-BLE")
        : _ble(ble), _deviceName(deviceName) {}

    void begin() override {
        _ble.begin(_deviceName);
    }

    void update() override {
        _ble.tick();
    }

    const char* moduleName() const override { return "BLE"; }
};

#endif // defined(ISKAKINO_PLATFORM_ESP32)

#endif // ISKAKINO_BLE_MODULE_H
