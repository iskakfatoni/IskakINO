// Verifikasi IskakINO_Kernel: registerModule(), begin(), update() semua
// terpanggil otomatis dan urut sesuai pendaftaran. Pakai modul dummy
// (bukan modul asli) supaya test ini fokus ke kernel-nya sendiri, terpisah
// dari kebenaran tiap modul (yang sudah diverifikasi masing-masing di
// test/mock_wifi, mock_ntp, mock_lcd, dst.)
#include "ArduinoExtra.h"
#include "../../src/core/IskakINO_Kernel.h"
#include <cassert>
#include <cstdio>
#include <vector>
#include <string>

std::vector<std::string> callLog;

class DummyModule : public IskakINO_Module {
    const char* _name;
  public:
    int beginCount = 0;
    int updateCount = 0;
    explicit DummyModule(const char* name) : _name(name) {}
    void begin() override { beginCount++; callLog.push_back(std::string(_name) + ":begin"); }
    void update() override { updateCount++; callLog.push_back(std::string(_name) + ":update"); }
    const char* moduleName() const override { return _name; }
};

int main() {
    DummyModule modA("A");
    DummyModule modB("B");
    DummyModule modC("C");

    assert(IskakINO.moduleCount() == 0);
    assert(IskakINO.registerModule(&modA) == true);
    assert(IskakINO.registerModule(&modB) == true);
    assert(IskakINO.registerModule(&modC) == true);
    assert(IskakINO.moduleCount() == 3);

    IskakINO.begin();
    assert(modA.beginCount == 1 && modB.beginCount == 1 && modC.beginCount == 1);
    // urutan begin() harus sesuai urutan pendaftaran
    assert(callLog[0] == "A:begin" && callLog[1] == "B:begin" && callLog[2] == "C:begin");

    IskakINO.update();
    IskakINO.update();
    assert(modA.updateCount == 2 && modB.updateCount == 2 && modC.updateCount == 2);

    // update() TIDAK boleh ikut memanggil begin() lagi
    assert(modA.beginCount == 1);

    // --- Uji slot penuh ---
    // ISKAKINO_KERNEL_MAX_MODULES default 8, sudah terisi 3, coba isi 5 lagi
    // (total 8, pas), lalu 1 lagi harus gagal (return false, tidak crash).
    DummyModule extra[6] = {
        DummyModule("D"), DummyModule("E"), DummyModule("F"),
        DummyModule("G"), DummyModule("H"), DummyModule("I")
    };
    int okCount = 0;
    for (int i = 0; i < 6; i++) {
        if (IskakINO.registerModule(&extra[i])) okCount++;
    }
    assert(okCount == 5); // slot 4..8 terisi (total jadi 8), yang ke-6 (I) gagal
    assert(IskakINO.moduleCount() == 8);
    assert(IskakINO.registerModule(&extra[5]) == false); // 'I' tetap gagal, slot penuh

    printf("OK: semua smoke-test IskakINO_Kernel lolos\n");
    return 0;
}
