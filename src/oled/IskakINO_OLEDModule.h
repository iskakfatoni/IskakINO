/*
 * src/oled/IskakINO_OLEDModule.h
 * Adapter modular supaya IskakINO_OLED bisa didaftarkan ke IskakINO_Kernel.
 *
 * Mengelola pemanggilan otomatis oled.begin() di setup() dan oled.update()
 * di setiap putaran loop() via IskakINO.begin() & IskakINO.update().
 */

#ifndef ISKAKINO_OLED_MODULE_H
#define ISKAKINO_OLED_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_OLED.h"

class IskakINO_OLEDModule : public IskakINO_Module {
  private:
    IskakINO_OLED& _oled;
    TwoWire* _wire;

  public:
    explicit IskakINO_OLEDModule(IskakINO_OLED& oled, TwoWire& wire = Wire)
        : _oled(oled), _wire(&wire) {}

    void begin() override {
        _oled.begin(*_wire);
    }

    void update() override {
        _oled.update();
    }

    const char* moduleName() const override {
        return "OLED";
    }
};

#endif // ISKAKINO_OLED_MODULE_H
