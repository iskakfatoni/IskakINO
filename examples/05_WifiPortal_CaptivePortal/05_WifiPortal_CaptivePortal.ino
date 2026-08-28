/*
 * 05_WifiPortal_CaptivePortal.ino
 * Modul: IskakINO_WifiPortal & IskakINO_ArduFast (HANYA ESP32 & ESP8266)
 *
 * Menunjukkan:
 *   1. Captive Portal otomatis saat ESP belum terhubung ke router WiFi
 *   2. Parameter kustom web (API Key) yang tersimpan di NVS/LittleFS
 *   3. Perlindungan PIN Admin dan fitur OTA Web Firmware Update
 *   4. Integrasi task scheduler dan logging via ArduFast
 */

#include <IskakINO.h>

#if !defined(ISKAKINO_HAS_WIFI)
  #error "Sketsa ini hanya mendukung board ESP32 atau ESP8266."
#endif

IskakINO_ArduFast   fast;
IskakINO_WifiPortal portal;

// Buffer string untuk parameter kustom Web UI
char apiKey[33] = "";

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - WiFi Captive Portal Demo   "));
    fast.log(F("========================================"));

    portal.setDebug(true);

    // Tambahkan parameter input kustom ke formulir Web UI
    portal.addParameter("apikey", "API Key / Token", apiKey, sizeof(apiKey));

    portal.setPortalTimeout(180); // Portal otomatis restart jika 3 menit idle
    portal.setAdminPin("1234");   // Kunci menu restart/reset dengan PIN
    portal.enableOTA(true);       // Buka menu update firmware via Web browser

    // Opsional: Daftarkan profil WiFi cadangan (Multi-SSID Fallback) via kode
    // portal.addWifi("Kantor-WiFi", "password123");
    // portal.addWifi("Hotspot-HP", "hotspot123");

    // Mulai mode koneksi atau Captive Portal secara asinkron
    portal.beginAsync("IskakINO-Setup");
    fast.logf(F("[Info] Memulai koneksi WiFi... Profil tersimpan: %d"), portal.getWifiCount());
}

void loop() {
    // WAJIB: Panggil portal.tick() di setiap loop untuk menjalankan Web Server & DNS
    portal.tick();

    // Pantau perubahan status koneksi WiFi
    static IskakPortalState lastState = IskakPortalState::IDLE;
    IskakPortalState currentState = portal.state();

    if (currentState != lastState) {
        if (currentState == IskakPortalState::CONNECTED) {
            fast.logf(F("[Success] WiFi tersambung ke: %s"), portal.getCurrentSSID().c_str());
            if (strlen(apiKey) > 0) {
                fast.logf(F("[Config] API Key tersimpan: %s"), apiKey);
            }
        } else if (currentState == IskakPortalState::PORTAL) {
            fast.log(F("[Portal] Gagal tersambung -> Access Point aktif: 'IskakINO-Setup'"));
            fast.log(F("[Portal] Konek ke AP tersebut dari HP/PC untuk mengisi SSID & Password."));
        }
        lastState = currentState;
    }

    // Task berkala setiap 5 detik saat sudah online
    if (portal.isConnected() && fast.every(5000, 0)) {
        fast.logf(F("[Online] SSID: %s | Uptime: %lu ms | IP: %s"), 
                   portal.getCurrentSSID().c_str(), millis(), WiFi.localIP().toString().c_str());
    }
}
