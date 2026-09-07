/*
 * src/ardufast/IskakINO_ArduFastModule.h
 * Adapter tipis supaya IskakINO_ArduFast bisa didaftarkan ke IskakINO_Kernel.
 *
 * ArduFast sendiri tidak punya konsep "tick per loop" (task manager-nya
 * dipakai lewat fast.every()/once() langsung oleh kode sketch, bukan lewat
 * satu method update() tunggal) -- jadi update() di sini sengaja kosong.
 * Adapter ini murni supaya fast.begin(baud) ikut terpanggil otomatis lewat
 * IskakINO.begin(), tanpa mengubah apa pun di IskakINO_ArduFast sendiri.
 */

#ifndef ISKAKINO_ARDUFAST_MODULE_H
#define ISKAKINO_ARDUFAST_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_ArduFast.h"

class IskakINO_ArduFastModule : public IskakINO_Module {
  private:
    IskakINO_ArduFast& _fast;
    unsigned long _baud;

  public:
    explicit IskakINO_ArduFastModule(IskakINO_ArduFast& fast, unsigned long baud = 115200)
        : _fast(fast), _baud(baud) {}

    void begin() override { _fast.begin(_baud); }
    // update() sengaja tidak di-override (default no-op dari IskakINO_Module)
    const char* moduleName() const override { return "ArduFast"; }
};

#endif
