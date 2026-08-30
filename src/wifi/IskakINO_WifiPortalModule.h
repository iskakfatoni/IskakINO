/*
 * src/wifi/IskakINO_WifiPortalModule.h
 * Adapter tipis supaya IskakINO_WifiPortal bisa didaftarkan ke
 * IskakINO_Kernel. update() meneruskan ke portal.tick() -- inilah yang
 * menjalankan seluruh state machine WiFi tanpa sketch perlu memanggilnya
 * manual tiap loop().
 *
 * Sama seperti IskakINO_WifiPortal.h sendiri, SELURUH isi file ini
 * dibungkus #if defined(ISKAKINO_HAS_WIFI) -- otomatis kosong (bukan
 * error) di board non-WiFi. Lihat komentar di IskakINO_WifiPortal.h untuk
 * alasan lengkapnya.
 */

#ifndef ISKAKINO_WIFIPORTAL_MODULE_H
#define ISKAKINO_WIFIPORTAL_MODULE_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_HAS_WIFI)

#include "../core/IskakINO_Module.h"
#include "IskakINO_WifiPortal.h"

class IskakINO_WifiPortalModule : public IskakINO_Module {
  private:
    IskakINO_WifiPortal& _portal;
    const char* _apName;
    const char* _apPass;

  public:
    explicit IskakINO_WifiPortalModule(IskakINO_WifiPortal& portal,
                                        const char* apName, const char* apPass = NULL)
        : _portal(portal), _apName(apName), _apPass(apPass) {}

    void begin() override { _portal.beginAsync(_apName, _apPass); }
    void update() override { _portal.tick(); }
    const char* moduleName() const override { return "WifiPortal"; }
};

#endif // defined(ISKAKINO_HAS_WIFI)

#endif
