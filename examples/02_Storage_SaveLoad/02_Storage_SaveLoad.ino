/*
 * 02_Storage_SaveLoad.ino
 * Modul: IskakINO_Storage & IskakINO_ArduFast
 * (Universal — EEPROM di AVR, Preferences di ESP32, LittleFS di ESP8266)
 *
 * Menunjukkan:
 *   1. Penyimpanan struct konfigurasi via IskakStorage.save()/load()
 *   2. Membaca IskakINO_Result via lastError() untuk diagnostik presisi
 *   3. Logging terpadu menggunakan IskakINO_ArduFast (fast.log / fast.logf)
 *   4. Riwayat pencatatan kejadian dengan ring-buffer log
 */

#include <IskakINO.h>

IskakINO_ArduFast fast;

// Struct konfigurasi (trivially-copyable)
struct Settings {
    uint8_t wifiChannel;
    float   suhuTarget;
    char    namaAlat[16];
};

int settingsAddr;

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - Storage Save & Load Demo   "));
    fast.log(F("========================================"));

    // Inisialisasi storage dengan namespace "demo"
    IskakStorage.begin("demo", true);

    // Pesan alamat memori untuk struct Settings
    settingsAddr = IskakStorage.reserve(sizeof(Settings));

    Settings s;
    if (IskakStorage.load(settingsAddr, s)) {
        fast.log(F("Settings ditemukan, berhasil dimuat dari penyimpanan."));
    } else {
        // Diagnostik error terperinci
        fast.logf(F("Load gagal (alasan: %s) -> Menyiapkan nilai default."),
                  IskakINO_ResultToString(IskakStorage.lastError()));

        s.wifiChannel = 6;
        s.suhuTarget  = 28.5f;
        strncpy(s.namaAlat, "Alat-01", sizeof(s.namaAlat));

        if (!IskakStorage.save(settingsAddr, s)) {
            fast.logf(F("Save GAGAL: %s"), IskakINO_ResultToString(IskakStorage.lastError()));
        } else {
            fast.log(F("Nilai default berhasil disimpan ke storage."));
        }
    }

    // Tampilkan data konfigurasi via ArduFast
    fast.logf(F(">> wifiChannel : %u"), s.wifiChannel);
    fast.logf(F(">> namaAlat    : %s"), s.namaAlat);
    fast.log(F(">> suhuTarget  : "), (long)s.suhuTarget);

    // --- Bonus: Ring-Buffer Logging untuk histori sensor/event ---
    IskakStorage.beginLog(500, 600, 10);
    IskakStorage.addLog((int)s.wifiChannel);
    fast.logf(F("Jumlah entri log tersimpan: %u"), IskakStorage.logCount());
}

void loop() {
    // Kosong -- contoh ini murni demonstrasi setup & storage lifecycle
}
