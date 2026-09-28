/*
 * src/shield/IskakINO_BasicIOShieldModule.h
 * Adapter tipis supaya IskakINO_BasicIOShield bisa didaftarkan ke IskakINO_Kernel.
 * update() meneruskan ke shield.update() -- inilah yang me-refresh multiplexer
 * 7-segment non-blocking secara otomatis tanpa sketch perlu memanggilnya
 * manual di tiap loop().
 */

#ifndef ISKAKINO_BASIC_IO_SHIELD_MODULE_H
#define ISKAKINO_BASIC_IO_SHIELD_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_BasicIOShield.h"

#if defined(ISKAKINO_PLATFORM_AVR)

class IskakINO_BasicIOShieldModule : public IskakINO_Module {
  private:
    IskakINO_BasicIOShield& _shield;

  public:
    explicit IskakINO_BasicIOShieldModule(IskakINO_BasicIOShield& shield) : _shield(shield) {}

    void begin() override { _shield.begin(); }
    void update() override { _shield.update(); }
    const char* moduleName() const override { return "BasicIOShield"; }
};

#endif // ISKAKINO_PLATFORM_AVR

#endif // ISKAKINO_BASIC_IO_SHIELD_MODULE_H
