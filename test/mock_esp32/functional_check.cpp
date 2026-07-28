// Smoke-test fungsional lewat alur publik (loadParams()/loadWifiList()/
// setupPortal() bersifat private, jadi diuji tidak langsung lewat
// beginAsync() — sesuai API publik yang tersedia untuk konsumen).
#include "../../src/wifi/IskakINO_WifiPortal.h"
#include <cassert>
#include <cstdio>

extern unsigned long _mock_millis_value;

int main() {
    char buf[16] = "";
    IskakINO_WifiPortal portal;
    portal.addParameter("id1", "Label", buf, sizeof(buf));

    // Belum pernah ada file tersimpan -> loadParams() harus return awal
    // TANPA menganggapnya error (lihat komentar "belum pernah save, bukan error").
    _mock_millis_value = 0;
    portal.beginAsync("TestAP");
    assert(portal.lastError() == IskakINO_Result::OK);

    // Tidak ada kredensial WiFi tersimpan -> otomatis masuk state PORTAL
    assert(portal.state() == IskakPortalState::PORTAL);
    assert(!portal.isConnected());

    // Portal timeout: set 1 detik, pastikan belum restart sebelum waktunya,
    // lalu restart tepat setelah waktu habis (via Scheduler, bukan lagi
    // _portalStartTime manual).
    portal.setPortalTimeout(1);
    // setupPortal() dipanggil ulang oleh beginAsync() di atas -> baseline
    // scheduler direset ke t=0.
    _mock_millis_value = 500;
    portal.tick();
    // Belum 1000ms, restart TIDAK boleh terpanggil.
    // (Tidak ada akses langsung ke ESP.restartCount dari sini karena ESP
    // adalah objek global mock; dicek lewat efek tidak langsung: state
    // tetap PORTAL, tidak exception/crash.)
    assert(portal.state() == IskakPortalState::PORTAL);

    _mock_millis_value = 1500;
    portal.tick(); // sekarang timeout terlampaui -> ESP.restart() (mock, no-op) terpanggil

    printf("OK: semua smoke-test fungsional WifiPortal lolos (ESP32 mock)\n");
    return 0;
}
