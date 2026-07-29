/*
 * 02_Storage_SaveLoad.ino
 * Modul: IskakINO_Storage (universal — EEPROM di AVR, Preferences di ESP32,
 * LittleFS di ESP8266/RP2040, otomatis dipilih lewat core/IskakINO_Platform.h)
 *
 * Menunjukkan save()/load() struct sederhana, plus cara membaca lastError()
 * (IskakINO_Result) untuk tahu PERSIS kenapa sebuah operasi gagal --
 * bukan cuma "gagal" tanpa keterangan.
 */

#include <IskakINO.h>

// Struct WAJIB trivially-copyable (tanpa String, tanpa pointer) --
// save()/load() akan gagal di compile-time (static_assert) kalau tidak.
struct Settings {
    uint8_t wifiChannel;
    float   suhuTarget;
    char    namaAlat[16];
};

int settingsAddr;

void setup() {
    Serial.begin(115200);
    IskakStorage.begin("demo", true); // namespace "demo", debug aktif

    settingsAddr = IskakStorage.reserve(sizeof(Settings));

    Settings s; // dideklarasikan di sini -- load() akan MENGISI s kalau sukses
    if (IskakStorage.load(settingsAddr, s)) {
        Serial.println(F("Settings ditemukan, memuat dari penyimpanan:"));
    } else {
        // lastError() kasih tahu KENAPA gagal -- pertama kali boot biasanya
        // NOT_FOUND (belum pernah disimpan), beda dengan CRC_MISMATCH
        // (data korup) atau OUT_OF_BOUNDS (alamat salah).
        Serial.print(F("Load gagal, alasan: "));
        Serial.println(IskakINO_ResultToString(IskakStorage.lastError()));

        Serial.println(F("Pakai nilai default & simpan."));
        s.wifiChannel = 6;
        s.suhuTarget = 28.5f;
        strncpy(s.namaAlat, "Alat-01", sizeof(s.namaAlat));

        if (!IskakStorage.save(settingsAddr, s)) {
            Serial.print(F("Save GAGAL: "));
            Serial.println(IskakINO_ResultToString(IskakStorage.lastError()));
        }
    }

    Serial.print(F("wifiChannel = ")); Serial.println(s.wifiChannel);
    Serial.print(F("suhuTarget  = ")); Serial.println(s.suhuTarget);
    Serial.print(F("namaAlat    = ")); Serial.println(s.namaAlat);

    // --- Bonus: mode log ring-buffer, cocok utk histori pembacaan sensor ---
    IskakStorage.beginLog(500, 600, 10); // area log terpisah dari settingsAddr
    IskakStorage.addLog((int)s.wifiChannel);
    Serial.print(F("Jumlah entri log tersimpan: "));
    Serial.println(IskakStorage.logCount());
}

void loop() {
    // Kosong -- contoh ini murni demonstrasi setup()
}
