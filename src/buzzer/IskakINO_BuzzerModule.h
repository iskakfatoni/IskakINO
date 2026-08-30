/*
 * src/buzzer/IskakINO_BuzzerModule.h
 * Adapter modular supaya IskakINO_Buzzer bisa didaftarkan ke IskakINO_Kernel.
 *
 * Mengelola pemanggilan otomatis buzzer.begin() di setup() dan buzzer.update()
 * di setiap putaran loop() via IskakINO.begin() & IskakINO.update().
 */

#ifndef ISKAKINO_BUZZER_MODULE_H
#define ISKAKINO_BUZZER_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_Buzzer.h"

class IskakINO_BuzzerModule : public IskakINO_Module {
  private:
    IskakINO_Buzzer& _buzzer;

  public:
    explicit IskakINO_BuzzerModule(IskakINO_Buzzer& buzzer)
        : _buzzer(buzzer) {}

    void begin() override {
        _buzzer.begin();
    }

    void update() override {
        _buzzer.update();
    }

    const char* moduleName() const override {
        return "Buzzer";
    }
};

#endif // ISKAKINO_BUZZER_MODULE_H
