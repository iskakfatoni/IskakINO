/*
 * 05_WifiPortal_CaptivePortal.ino
 * Modul: IskakINO_WifiPortal (HANYA ESP32/ESP8266)
 *
 * Menunjukkan captive portal dasar dengan satu custom parameter (API Key),
 * timeout portal, dan cara membaca status koneksi. Kalau belum ada WiFi
 * tersimpan, board akan membuka Access Point "IskakINO-Setup" -- konek ke
 * situ dari HP, portal akan otomatis muncul untuk isi SSID/password.
 *
 * CATATAN: sketch ini HANYA bisa di-compile untuk board ESP32/ESP8266.
 * Kalau di-compile untuk AVR, class IskakINO_WifiPortal tidak akan ada
 * (lihat catatan guard platform di src/wifi/IskakINO_WifiPortal.h).
 */

#include <IskakINO.h>

IskakINO_WifiPortal portal;
char apiKey[33] = ""; // buffer utk custom parameter, HARUS bertahan selama portal aktif

void setup() {
    Serial.begin(115200);
    portal.setDebug(true);

    // Custom parameter muncul otomatis di halaman portal sebagai input teks
    portal.addParameter("apikey", "API Key", apiKey, sizeof(apiKey));

    // Opsional: tambah kredensial WiFi hardcode sebagai fallback (selain
    // yang nanti disimpan lewat portal)
    // portal.addWifi("SSID_Cadangan", "password_cadangan");

    portal.setPortalTimeout(180); // portal otomatis restart kalau 3 menit tidak diisi
    portal.setAdminPin("1234");   // lindungi tombol reboot/reset dgn PIN

    portal.beginAsync("IskakINO-Setup"); // non-blocking, langsung lanjut ke loop()
}

void loop() {
    portal.tick(); // WAJIB dipanggil tiap loop() -- ini yang menjalankan seluruh state machine

    static IskakPortalState lastState = IskakPortalState::IDLE;
    IskakPortalState state = portal.state();
    if (state != lastState) {
        if (state == IskakPortalState::CONNECTED) {
            Serial.println(F("WiFi tersambung!"));
            if (strlen(apiKey) > 0) {
                Serial.print(F("API Key tersimpan: "));
                Serial.println(apiKey);
            }
        } else if (state == IskakPortalState::PORTAL) {
            Serial.println(F("Membuka portal konfigurasi -- konek ke AP 'IskakINO-Setup'"));
        }
        lastState = state;
    }

    // Kode aplikasi lain bisa jalan di sini, cek portal.isConnected() dulu
    // sebelum melakukan hal yang butuh internet (mis. kirim data ke server).
}
