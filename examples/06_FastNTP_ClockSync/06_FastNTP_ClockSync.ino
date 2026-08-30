/*
 * 06_FastNTP_ClockSync.ino
 * Modul: IskakINO_FastNTP & IskakINO_ArduFast (HANYA ESP32 & ESP8266)
 *
 * Menunjukkan:
 *   1. Sinkronisasi waktu internet NTP secara asinkron (non-blocking)
 *   2. Konversi zona waktu lokal (WIB GMT+7) & lokalisasi nama hari Bahasa Indonesia
 *   3. Penjadwalan pencetakan jam per detik via ArduFast scheduler
 */

#include <IskakINO.h>
#include <WiFiUdp.h>

#if !defined(ISKAKINO_HAS_WIFI)
  #error "Sketsa ini hanya mendukung board ESP32 atau ESP8266."
#endif

const char* WIFI_SSID = "Nama_WiFi_Anda";
const char* WIFI_PASS = "Password_WiFi_Anda";

IskakINO_ArduFast fast;
WiFiUDP           ntpUdp;
IskakINO_FastNTP  ntp(ntpUdp, "pool.ntp.org");

void onSyncSukses(uint32_t utcEpoch) {
    (void)utcEpoch;
    fast.log(F("[NTP] Sinkronisasi waktu berhasil!"));
}

void onSyncGagal(uint8_t consecutiveFails) {
    fast.logf(F("[NTP] Gagal sinkronisasi, percobaan ke-%u"), consecutiveFails);
}

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - FastNTP Clock Sync Demo    "));
    fast.log(F("========================================"));

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    fast.log(F("Menyambungkan ke WiFi..."));

    while (WiFi.status() != WL_CONNECTED && millis() < 10000) {
        delay(300);
    }

    if (WiFi.status() == WL_CONNECTED) {
        fast.logf(F("[WiFi] Tersambung! IP: %s"), WiFi.localIP().toString().c_str());
    } else {
        fast.log(F("[WiFi] Gagal tersambung dalam 10 detik. Periksa SSID/Password."));
    }

    ntp.setDebug(true);
    ntp.onSync(onSyncSukses);
    ntp.onSyncFail(onSyncGagal);
    ntp.setSyncInterval(3600000UL); // Sync ulang setiap 1 jam

    // Atur GMT+7 (25200 detik), tanpa DST (0)
    ntp.begin(25200, 0);
}

void loop() {
    // WAJIB: Panggil ntp.update() di setiap loop untuk sinkronisasi waktu berkala
    ntp.update();

    // Cetak waktu setiap 1000 ms menggunakan ArduFast scheduler
    if (ntp.isTimeSet() && fast.every(1000, 0)) {
        fast.logf(F("[Waktu] %s, %s %s"),
                  ntp.getDayName(LANG_ID).c_str(),
                  ntp.getFormattedDate().c_str(),
                  ntp.getFormattedTime().c_str());
    }
}
