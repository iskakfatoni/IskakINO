/*
 * 06_FastNTP_ClockSync.ino
 * Modul: IskakINO_FastNTP (HANYA ESP32/ESP8266)
 *
 * Menunjukkan sinkronisasi waktu via NTP setelah WiFi tersambung (di sini
 * pakai WiFi.begin() polos supaya contoh berdiri sendiri -- lihat
 * 07_Unified_SmartClock untuk versi yang dipadukan dengan WifiPortal).
 *
 * CATATAN: sketch ini HANYA bisa di-compile untuk board ESP32/ESP8266.
 */

#include <IskakINO.h>
#include <WiFiUdp.h>

const char* WIFI_SSID = "Nama_WiFi_Anda";
const char* WIFI_PASS = "Password_WiFi_Anda";

WiFiUDP ntpUdp;
IskakINO_FastNTP ntp(ntpUdp, "pool.ntp.org");

void onSyncSukses(uint32_t utcEpoch) {
    (void)utcEpoch;
    Serial.println(F("[NTP] Sinkronisasi berhasil."));
}

void onSyncGagal(uint8_t consecutiveFails) {
    Serial.print(F("[NTP] Gagal sync, percobaan ke-"));
    Serial.println(consecutiveFails);
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print(F("Menyambungkan WiFi"));
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(F("."));
    }
    Serial.println(F(" tersambung."));

    ntp.setDebug(true);
    ntp.onSync(onSyncSukses);
    ntp.onSyncFail(onSyncGagal);
    ntp.setSyncInterval(3600000UL); // sync ulang tiap 1 jam

    ntp.begin(25200, 0); // GMT+7 (WIB), tanpa DST
}

void loop() {
    ntp.update(); // WAJIB dipanggil tiap loop() -- non-blocking, tidak pernah delay()

    static unsigned long lastPrint = 0;
    if (ntp.isTimeSet() && millis() - lastPrint >= 1000) {
        lastPrint = millis();
        Serial.print(ntp.getFormattedDate());
        Serial.print(F(" "));
        Serial.println(ntp.getFormattedTime());

        // Contoh alarm: cetak pesan tepat jam 07:00:00, cuma sekali (tidak
        // retrigger terus selama detiknya masih 0 di frame yg sama)
        static bool alarmFired = false;
        if (ntp.isAlarmActive(7, 0, 0, alarmFired)) {
            Serial.println(F(">> Alarm 07:00 berbunyi! <<"));
        }
    }
}
