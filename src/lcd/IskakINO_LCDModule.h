/*
 * src/lcd/IskakINO_LCDModule.h
 * Adapter tipis supaya LiquidCrystal_I2C bisa didaftarkan ke IskakINO_Kernel.
 * update() meneruskan ke lcd.update() -- inilah yang menggerakkan efek
 * non-blocking (typewriter/scroll/backlight timeout) tanpa sketch perlu
 * memanggilnya manual tiap loop().
 */

#ifndef ISKAKINO_LCD_MODULE_H
#define ISKAKINO_LCD_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_LiquidCrystal_I2C.h"

class IskakINO_LCDModule : public IskakINO_Module {
  private:
    LiquidCrystal_I2C& _lcd;

  public:
    explicit IskakINO_LCDModule(LiquidCrystal_I2C& lcd) : _lcd(lcd) {}

    void begin() override { _lcd.begin(); }
    void update() override { _lcd.update(); }
    const char* moduleName() const override { return "LCD"; }
};

#endif
