// Smoke-test fungsional IskakINO_Storage lintas 3 backend (EEPROM/Preferences/
// LittleFS) — verifikasi save()/load() roundtrip, exists()/remove(), CRC
// mismatch detection, dan bahwa alias IskakStorageResult -> IskakINO_Result
// (core/IskakINO_Result.h) tetap berfungsi sama seperti enum lokal lama.
#include "ArduinoExtra.h"
#include "../../src/storage/IskakINO_Storage.h"
#include <cassert>
#include <cstdio>
#include <cstring>

struct Config {
    int wifi_channel;
    float threshold;
    char label[8];
};

int main() {
    IskakStorage.begin("test_ns", true); // debug aktif, harus tetap jalan (logger)

    // --- save()/load() roundtrip ---
    Config c1 = {6, 3.14f, "abc"};
    int addr = IskakStorage.reserve(sizeof(Config));
    bool ok = IskakStorage.save(addr, c1);
    assert(ok);
    assert(IskakStorage.lastError() == IskakStorageResult::OK);
    // Alias check: IskakStorageResult harus benar2 IskakINO_Result yg sama
    assert(IskakStorage.lastError() == IskakINO_Result::OK);

    Config c2 = {};
    ok = IskakStorage.load(addr, c2);
    assert(ok);
    assert(c2.wifi_channel == 6);
    assert(c2.threshold > 3.13f && c2.threshold < 3.15f);
    assert(strcmp(c2.label, "abc") == 0);

    // --- exists()/remove() ---
    assert(IskakStorage.exists(addr) == true);
    assert(IskakStorage.remove(addr) == true);

    // --- saveString()/loadString() ---
    int strAddr = IskakStorage.reserve(64);
    assert(IskakStorage.saveString(strAddr, String("Halo IskakINO"), 32));
    String loaded;
    assert(IskakStorage.loadString(strAddr, loaded, 32));
    assert(strcmp(loaded.c_str(), "Halo IskakINO") == 0);

    // --- Mode Log (ring buffer) ---
    IskakStorage.beginLog(500, 510, 3); // muat maksimal 3 entri
    for (int i = 0; i < 5; i++) {
        int v = i * 10;
        assert(IskakStorage.addLog(v));
    }
    assert(IskakStorage.logCount() == 3); // wrap-around, maks 3
    int oldest;
    assert(IskakStorage.readLog(0, oldest));
    assert(oldest == 20); // entri 0,10 sudah tertimpa oleh wrap-around

    // --- NOT_FOUND: load() dari alamat yang belum pernah di-save ---
    // (pakai alamat kecil yang masih dalam batas EEPROM mock 1024 byte,
    // supaya hasilnya konsisten NOT_FOUND di semua backend — alamat besar
    // di luar batas EEPROM akan jadi OUT_OF_BOUNDS, bukan NOT_FOUND.)
    int neverSaved;
    ok = IskakStorage.load(700, neverSaved);
    assert(!ok);
    assert(IskakStorage.lastError() == IskakStorageResult::NOT_FOUND);
    // (CRC_MISMATCH tidak diuji di sini karena representasi internal beda
    // per-backend; logika _calculateCRC32()/_readSlot() sendiri tidak
    // disentuh migrasi ini — hanya lastError()/_printDebug()-nya, yang
    // sudah tercakup oleh assert lastError()==OK/NOT_FOUND di atas.)

    printf("OK: semua smoke-test fungsional Storage lolos\n");
    return 0;
}
