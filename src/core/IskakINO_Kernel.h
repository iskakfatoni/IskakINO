/*
 * src/core/IskakINO_Kernel.h
 * Kernel/registry framework IskakINO. Modul (lewat kelas adapter, lihat
 * IskakINO_Module.h) didaftarkan sekali di setup(), lalu kernel otomatis
 * memanggil begin()/update() semua modul terdaftar tanpa sketch perlu
 * menulis ulang tiap modul.begin()/modul.tick() secara manual satu-satu.
 *
 * Contoh pemakaian (lihat juga examples/08_Framework_Kernel):
 *   IskakINO_WifiPortalModule portalMod(portal);
 *   IskakINO_FastNTPModule    ntpMod(ntp);
 *   IskakINO_LCDModule        lcdMod(lcd);
 *
 *   void setup() {
 *       IskakINO.registerModule(&portalMod);
 *       IskakINO.registerModule(&ntpMod);
 *       IskakINO.registerModule(&lcdMod);
 *       IskakINO.begin();   // panggil begin() ketiganya, urut pendaftaran
 *   }
 *   void loop() {
 *       IskakINO.update();  // panggil update() ketiganya, urut pendaftaran
 *   }
 *
 * Global instance `IskakINO` (di bawah) dikonfigurasi dengan slot tetap
 * (default 8, bisa di-override lewat makro ISKAKINO_KERNEL_MAX_MODULES
 * SEBELUM #include <IskakINO.h>, pola sama seperti ISKAKINO_LOGF_BUFFER_SIZE
 * di core/IskakINO_Logger.h) -- karena ini singleton global yang
 * dikonstruksi sekali saat static-init, ukurannya tidak bisa diubah lagi
 * setelah compile.
 *
 * registerModule() TIDAK WAJIB dipakai -- kalau Kak Iskak lebih suka pola
 * manual (panggil begin()/tick()/update() tiap modul sendiri-sendiri di
 * sketch, seperti contoh 07_Unified_SmartClock), itu tetap didukung penuh
 * dan tidak lebih "kurang benar". Kernel ini murni kenyamanan opsional.
 */

#ifndef ISKAKINO_KERNEL_H
#define ISKAKINO_KERNEL_H

#include <Arduino.h>
#include "IskakINO_Module.h"
#include "IskakINO_Logger.h"

#ifndef ISKAKINO_KERNEL_MAX_MODULES
#define ISKAKINO_KERNEL_MAX_MODULES 8
#endif

class IskakINO_Kernel {
  private:
    IskakINO_Module* _modules[ISKAKINO_KERNEL_MAX_MODULES];
    uint8_t _count = 0;
    IskakINO_Logger _logger;

    // Non-copyable: menyalin array pointer secara shallow tidak masuk akal
    // untuk sebuah registry (dua kernel akan "berbagi" modul yang sama).
    IskakINO_Kernel(const IskakINO_Kernel&);
    IskakINO_Kernel& operator=(const IskakINO_Kernel&);

  public:
    IskakINO_Kernel() {}

    // Daftarkan modul (lewat adapter, mis. IskakINO_WifiPortalModule).
    // Return false kalau slot penuh (ISKAKINO_KERNEL_MAX_MODULES tercapai)
    // -- modul TIDAK terdaftar, tapi tidak crash. Naikkan
    // ISKAKINO_KERNEL_MAX_MODULES kalau butuh lebih banyak slot.
    bool registerModule(IskakINO_Module* mod) {
        if (!mod || _count >= ISKAKINO_KERNEL_MAX_MODULES) return false;
        _modules[_count++] = mod;
        return true;
    }

    // Panggil begin() SEMUA modul terdaftar, urut sesuai urutan pendaftaran.
    void begin() {
        for (uint8_t i = 0; i < _count; i++) {
            if (_logger.isDebug()) {
                Serial.print(F("[IskakINO] begin(): "));
                Serial.println(_modules[i]->moduleName());
            }
            _modules[i]->begin();
        }
    }

    // Panggil update() SEMUA modul terdaftar, urut sesuai urutan pendaftaran.
    // Idealnya dipanggil sekali per loop().
    inline void update() {
        for (uint8_t i = 0; i < _count; i++) {
            _modules[i]->update();
        }
    }

    uint8_t moduleCount() const { return _count; }

    // Diagnostik opsional (default nonaktif, pola sama seperti modul lain).
    void setDebug(bool debugMode) { _logger.setDebug(debugMode); }
};

// Instance global tunggal -- inilah yang dipakai sketch lewat `IskakINO.`
extern IskakINO_Kernel IskakINO;

#endif
