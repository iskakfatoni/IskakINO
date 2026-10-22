/*
 * src/core/IskakINO_Module.h
 * Interface dasar untuk semua "modul" yang ingin dikelola otomatis oleh
 * IskakINO_Kernel (lihat IskakINO_Kernel.h).
 *
 * DESAIN PENTING: class modul asli (IskakINO_WifiPortal, IskakINO_FastNTP,
 * LiquidCrystal_I2C, IskakINO_Storage, IskakINO_SmartVoice, IskakINO_ArduFast)
 * TIDAK mewarisi IskakINO_Module secara langsung — supaya API publik yang
 * sudah ada dan sudah diverifikasi lintas 6 pilot migrasi TIDAK berubah
 * sama sekali. Sebagai gantinya, tiap modul punya kelas "adapter" tipis
 * (mis. IskakINO_WifiPortalModule) yang MEWARISI IskakINO_Module dan
 * meneruskan begin()/update() ke method asli modul tsb (tick()/update()/
 * begin() dengan parameter yang sesuai). Lihat mis. src/wifi/
 * IskakINO_WifiPortalModule.h untuk contoh konkretnya.
 *
 * begin() dan update() sengaja BUKAN pure virtual (ada implementasi default
 * kosong) — supaya modul yang memang tidak butuh salah satunya (mis.
 * IskakINO_Storage tidak butuh update() per-loop) tidak dipaksa
 * mengimplementasikan method yang tidak relevan.
 */

#ifndef ISKAKINO_MODULE_H
#define ISKAKINO_MODULE_H

#include <Arduino.h>

class IskakINO_Module {
  public:
    virtual ~IskakINO_Module() {}

    // Dipanggil SEKALI oleh IskakINO_Kernel::begin(), urut sesuai urutan
    // registerModule() dipanggil.
    virtual void begin() {}

    // Dipanggil setiap kali IskakINO_Kernel::update() dipanggil (idealnya
    // sekali per loop()), urut sesuai urutan pendaftaran.
    virtual void update() {}

    // Nama modul untuk keperluan logging/debug kernel, mis. "WifiPortal".
    virtual const char* moduleName() const { return "Module"; }
};

#endif
