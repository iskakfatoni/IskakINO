/*
 * 07_Unified_SmartClock.ino
 * Modul: SEMUA (ArduFast + Storage + LCD + WifiPortal + FastNTP)
 *
 * Inilah tepatnya alasan ekosistem IskakINO digabung jadi satu library:
 * sebelum penggabungan, contoh seperti ini TIDAK BISA di-compile dalam CI
 * WifiPortal standalone (lihat CHANGELOG -- "01_Basic_Clock.ino dilewati
 * karena butuh IskakINO_FastNTP terpisah"). Sekarang, satu #include cukup.
 *
 * Alur:
 *   1. WifiPortal menyalakan captive portal kalau belum ada WiFi tersimpan.
 *   2. Setelah WiFi tersambung, FastNTP mulai sinkronisasi jam.
 *   3. Epoch waktu terakhir yang berhasil disinkronkan disimpan ke Storage,
 *      supaya reboot berikutnya LCD bisa langsung tampil waktu masuk akal
 *      (walau belum sempat sync ulang), bukan "00:00:00".
 *   4. LCD menampilkan jam + status koneksi, di-refresh tiap 1 detik lewat
 *      ArduFast scheduler (bukan LCD-nya sendiri yang di-refresh terus-
 *      menerus tiap loop() -- itu boros bus I2C tanpa manfaat).
 *
 * CATATAN: sketch ini HANYA bisa di-compile untuk board ESP32/ESP8266
 * (karena memakai WifiPortal & FastNTP). Modul-modul lain di dalamnya
 * (ArduFast, Storage, LCD) sebenarnya universal.
 */

#include <IskakINO.h>
#include <WiFiUdp.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);
IskakINO_WifiPortal portal;
WiFiUDP ntpUdp;
IskakINO_FastNTP ntp(ntpUdp, "pool.ntp.org");

#define TASK_LCD_REFRESH 0
#define EPOCH_STORAGE_ADDR_PLACEHOLDER 0 // diisi hasil reserve() di setup()
int epochStorageAddr;

void onSyncSukses(uint32_t utcEpoch) {
    ntp.setUtcEpoch(utcEpoch); // no-op sebenarnya (sudah otomatis), contoh eksplisit
    uint32_t toSave = ntp.getUtcEpoch();
    IskakStorage.save(epochStorageAddr, toSave); // simpan supaya reboot berikutnya punya fallback
}

void setup() {
    Serial.begin(115200);
    fast.begin(115200);

    IskakStorage.begin("smartclock");
    epochStorageAddr = IskakStorage.reserve(sizeof(uint32_t));

    lcd.begin();
    lcd.printCenter("IskakINO Clock", 0);

    // --- Fallback waktu dari penyimpanan, sebelum WiFi/NTP sempat sync ---
    uint32_t savedEpoch;
    if (IskakStorage.load(epochStorageAddr, savedEpoch)) {
        ntp.setUtcEpoch(savedEpoch);
        fast.log(F("Waktu fallback dimuat dari Storage."));
    }

    // --- WiFi provisioning non-blocking ---
    portal.setPortalTimeout(180);
    portal.beginAsync("IskakINO-Clock");

    // --- NTP: mulai sinkronisasi begitu WiFi tersambung (dicek di loop()) ---
    ntp.onSync(onSyncSukses);
    ntp.setSyncInterval(3600000UL); // 1 jam
}

void loop() {
    portal.tick(); // state machine WiFi
    ntp.update();  // state machine NTP (aman dipanggil terus walau WiFi belum tersambung)
    lcd.update();  // efek non-blocking LCD (kalau ada typewriter/scroll aktif)

    // Refresh tampilan LCD cuma 1x/detik -- bukan tiap loop(), supaya bus
    // I2C tidak dibanjiri write yang sia-sia (nilai jam kan cuma berubah
    // tiap detik, bukan tiap microdetik loop() berputar).
    if (fast.every(1000, TASK_LCD_REFRESH)) {
        lcd.setCursor(0, 1);
        if (portal.state() == IskakPortalState::PORTAL) {
            lcd.print(F("Setup WiFi...   "));
        } else if (!portal.isConnected()) {
            lcd.print(F("Menyambung...   "));
        } else if (!ntp.isTimeSet()) {
            lcd.print(F("Sync waktu...   "));
        } else {
            lcd.print(ntp.getFormattedTime());
            lcd.print(F("        ")); // padding, hapus sisa karakter lama
        }
    }
}
