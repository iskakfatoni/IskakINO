/*
 * 08_Framework_Kernel.ino
 * Modul: SEMUA (ArduFast + Storage + LCD + WifiPortal + FastNTP), lewat
 * lapisan framework IskakINO_Kernel (src/core/IskakINO_Kernel.h).
 *
 * Ini versi framework dari 07_Unified_SmartClock -- fungsinya PERSIS SAMA,
 * tapi bandingkan setup()/loop() di sini dengan versi manual di 07:
 *
 *   07 (manual)                          08 (framework, contoh ini)
 *   ---------------------------------    ---------------------------------
 *   fast.begin(115200);                  IskakINO.registerModule(&fastMod);
 *   IskakStorage.begin("smartclock");    IskakINO.registerModule(&storageMod);
 *   lcd.begin();                         IskakINO.registerModule(&lcdMod);
 *   portal.beginAsync("...");            IskakINO.registerModule(&portalMod);
 *   ...                                  IskakINO.registerModule(&ntpMod);
 *                                         IskakINO.begin();   // <- 1 baris
 *
 *   portal.tick();                       IskakINO.update();  // <- 1 baris
 *   ntp.update();
 *   lcd.update();
 *
 * Dua pola ini SAMA-SAMA valid dan didukung penuh -- pilih mana yang lebih
 * cocok untuk proyek Anda. Pola manual (07) memberi kontrol lebih detail
 * (mis. urutan pemanggilan custom, kondisional per modul); pola framework
 * (08, contoh ini) lebih ringkas untuk proyek dengan banyak modul.
 *
 * CATATAN: sketch ini HANYA bisa di-compile untuk board ESP32/ESP8266
 * (karena memakai WifiPortal & FastNTP, sama seperti 07).
 */

#include <IskakINO.h>
#include <WiFiUdp.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);
IskakINO_WifiPortal portal;
WiFiUDP ntpUdp;
IskakINO_FastNTP ntp(ntpUdp, "pool.ntp.org");

// --- Adapter, satu per modul (lihat komentar di masing-masing header
// src/*/IskakINO_*Module.h untuk detail kenapa pola adapter dipakai) ---
IskakINO_ArduFastModule   fastMod(fast, 115200);
IskakINO_StorageModule    storageMod("smartclock");
IskakINO_LCDModule        lcdMod(lcd);
IskakINO_WifiPortalModule portalMod(portal, "IskakINO-Clock");
IskakINO_FastNTPModule    ntpMod(ntp, 25200, 0); // GMT+7, tanpa DST

#define TASK_LCD_REFRESH 0
int epochStorageAddr;

void onSyncSukses(uint32_t utcEpoch) {
    IskakStorage.save(epochStorageAddr, utcEpoch); // fallback utk reboot berikutnya
}

void setup() {
    Serial.begin(115200);

    IskakINO.setDebug(true);
    IskakINO.registerModule(&fastMod);
    IskakINO.registerModule(&storageMod);
    IskakINO.registerModule(&lcdMod);
    IskakINO.registerModule(&portalMod);
    IskakINO.registerModule(&ntpMod);
    IskakINO.begin(); // panggil begin() KELIMA modul di atas, satu baris

    // Beberapa hal tetap perlu disiapkan manual (reserve address, callback)
    // -- kernel cuma mengurus lifecycle begin()/update(), bukan seluruh logika.
    epochStorageAddr = IskakStorage.reserve(sizeof(uint32_t));
    uint32_t savedEpoch;
    if (IskakStorage.load(epochStorageAddr, savedEpoch)) {
        ntp.setUtcEpoch(savedEpoch);
    }
    ntp.onSync(onSyncSukses);

    lcd.printCenter("IskakINO Clock", 0);
}

void loop() {
    IskakINO.update(); // panggil update() KELIMA modul di atas, satu baris
                        // (ArduFast & Storage: no-op, memang tidak butuh update())

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
            lcd.print(F("        "));
        }
    }
}
