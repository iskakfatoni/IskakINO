/*
 * 20_RTC_HybridNTP.ino
 * Modul: IskakINO_RTC, IskakINO_FastNTP, & IskakINO_ArduFast (HANYA ESP32 & ESP8266)
 *
 * Menunjukkan:
 *   1. Sinergi Hybrid Clock antara FastNTP (Internet) dan RTC Fisik (Offline)
 *   2. Saat online: FastNTP otomatis menyinkronkan jam fisik RTC
 *   3. Saat offline / WiFi putus: Sistem mulus membaca jam dari RTC tanpa jeda
 */

#include <IskakINO.h>

#if !defined(ISKAKINO_HAS_WIFI)
  #error "Sketsa ini hanya mendukung board ESP32 atau ESP8266."
#endif

// Konfigurasi WiFi
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASS";

WiFiUDP           ntpUdp;
IskakINO_ArduFast fast;
IskakINO_FastNTP  ntp(ntpUdp, "pool.ntp.org");
IskakINO_RTC      rtc;

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("   IskakINO - Hybrid NTP + RTC Demo     "));
    fast.log(F("========================================"));

    // 1. Inisialisasi RTC
    if (rtc.begin()) {
        fast.logf(F("[RTC] Hardware RTC terhubung: %s"), rtc.chipName());
    } else {
        fast.log(F("[RTC] Tidak ada RTC hardware yang terdeteksi."));
    }

    // 2. Hubungkan ke WiFi & FastNTP (WIB UTC+7 = 25200 detik)
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    ntp.begin(25200);

    // 3. Pasangkan NTP Sync ke RTC (sinkronkan jam fisik tiap 1 jam secara otomatis)
    rtc.syncWithNTP(ntp, 3600000UL);
}

void loop() {
    // Jalankan scheduler NTP & RTC
    ntp.update();
    rtc.tick();

    // Tampilkan status waktu setiap 2 detik
    if (fast.every(2000, 0)) {
        if (WiFi.status() == WL_CONNECTED && ntp.isSynced()) {
            // Mode Online: Waktu bersumber dari FastNTP
            fast.logf(F("[Online / NTP] %s %s"), ntp.getFormattedDate().c_str(), ntp.getFormattedTime().c_str());
        } else {
            // Mode Offline: Waktu bersumber dari RTC Fisik
            IskakDateTime dt = rtc.now();
            fast.logf(F("[Offline / RTC] %s %s"), dt.getDateString(true).c_str(), dt.getTimeString(true).c_str());
        }
    }
}
