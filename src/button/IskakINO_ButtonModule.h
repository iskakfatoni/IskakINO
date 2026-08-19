/*
 * src/button/IskakINO_ButtonModule.h
 * Adapter modular supaya IskakINO_Button bisa didaftarkan ke IskakINO_Kernel.
 */

#ifndef ISKAKINO_BUTTON_MODULE_H
#define ISKAKINO_BUTTON_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_Button.h"

class IskakINO_ButtonModule : public IskakINO_Module {
  private:
    IskakINO_Button& _button;

  public:
    explicit IskakINO_ButtonModule(IskakINO_Button& button)
        : _button(button) {}

    void begin() override {
        _button.begin();
    }

    void update() override {
        _button.update();
    }

    const char* moduleName() const override {
        return "Button";
    }
};

#endif // ISKAKINO_BUTTON_MODULE_H
