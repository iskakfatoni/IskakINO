/*
 * src/ntp/IskakINO_FastNTPModule.h
 * Adapter tipis supaya IskakINO_FastNTP bisa didaftarkan ke IskakINO_Kernel.
 * update() meneruskan ke ntp.update() -- inilah yang menjalankan state
 * machine sinkronisasi NTP tanpa sketch perlu memanggilnya manual tiap
 * loop().
 *
 * Sama seperti IskakINO_FastNTP.h sendiri, SELURUH isi file ini dibungkus
 * #if defined(ISKAKINO_HAS_WIFI) -- otomatis kosong (bukan error) di board
 * non-WiFi.
 */

#ifndef ISKAKINO_FASTNTP_MODULE_H
#define ISKAKINO_FASTNTP_MODULE_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_HAS_WIFI)

#include "../core/IskakINO_Module.h"
#include "IskakINO_FastNTP.h"

class IskakINO_FastNTPModule : public IskakINO_Module {
  private:
    IskakINO_FastNTP& _ntp;
    long _gmtOffsetSec;
    int _daylightOffsetSec;

  public:
    explicit IskakINO_FastNTPModule(IskakINO_FastNTP& ntp,
                                     long gmtOffsetSec = 25200, int daylightOffsetSec = 0)
        : _ntp(ntp), _gmtOffsetSec(gmtOffsetSec), _daylightOffsetSec(daylightOffsetSec) {}

    void begin() override { _ntp.begin(_gmtOffsetSec, _daylightOffsetSec); }
    void update() override { _ntp.update(); }
    const char* moduleName() const override { return "FastNTP"; }
};

#endif // defined(ISKAKINO_HAS_WIFI)

#endif
