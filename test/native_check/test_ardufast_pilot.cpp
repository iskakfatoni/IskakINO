// test/native/test_task_manager.cpp
//
// Unit test native (host) untuk logika platform-independent
// IskakINO_ArduFast: every(), once(), reset(), cancel(), mapAnalog().
// Dikompilasi & dijalankan langsung dengan g++ di CI, TANPA toolchain
// AVR/ESP8266/ESP32 dan tanpa hardware — jauh lebih cepat & tidak butuh
// board fisik dibanding compile-check.yml yang hanya menjamin *compile*,
// bukan *behavior*.

#include "Arduino.h"
#include "../../src/ardufast/IskakINO_ArduFast.h"
#include <cstdio>

static int failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                    \
        if (!(cond)) {                                                      \
            std::printf("FAIL: %s (line %d)\n", #cond, __LINE__);           \
            failures++;                                                     \
        } else {                                                            \
            std::printf("OK:   %s\n", #cond);                               \
        }                                                                   \
    } while (0)

void test_every() {
    std::printf("\n-- test_every --\n");
    IskakINO_ArduFast fast(5);
    _mock_millis_value = 0;

    CHECK(fast.every(1000, 0) == false);  // baru dibuat, elapsed 0ms < interval -> belum trigger
    CHECK(fast.every(1000, 0) == false);  // masih belum lewat 1000ms

    _mock_millis_value = 999;
    CHECK(fast.every(1000, 0) == false);  // masih kurang 1ms

    _mock_millis_value = 1000;
    CHECK(fast.every(1000, 0) == true);   // tepat 1000ms, harus trigger

    CHECK(fast.every(1000, 5) == false);  // id 5 di luar rentang (maxTasks=5, valid 0..4)
}

void test_once() {
    std::printf("\n-- test_once --\n");
    IskakINO_ArduFast fast(5);
    _mock_millis_value = 0;

    CHECK(fast.once(500, 1) == false);    // belum lewat delay
    _mock_millis_value = 500;
    CHECK(fast.once(500, 1) == true);     // trigger sekali

    _mock_millis_value = 1000;
    CHECK(fast.once(500, 1) == false);    // tidak boleh trigger lagi (one-shot)
}

void test_reset_and_cancel() {
    std::printf("\n-- test_reset_and_cancel --\n");
    IskakINO_ArduFast fast(5);
    _mock_millis_value = 0;

    fast.cancel(2);
    CHECK(fast.every(0, 2) == false);     // dibatalkan -> selalu false walau interval 0

    fast.reset(2);
    CHECK(fast.every(0, 2) == true);      // setelah reset -> aktif lagi

    _mock_millis_value = 0;
    CHECK(fast.once(100, 3) == false);
    _mock_millis_value = 100;
    CHECK(fast.once(100, 3) == true);
    CHECK(fast.once(100, 3) == false);    // sudah fired sekali

    fast.reset(3);                        // re-arm
    CHECK(fast.once(0, 3) == true);       // delay 0 dari titik reset -> langsung trigger
}

void test_map_analog() {
    std::printf("\n-- test_map_analog --\n");
    IskakINO_ArduFast fast(5);

    _mock_analog_value = 0;
    CHECK(fast.mapAnalog(0, 0, 255) == 0);

    _mock_analog_value = 1023;
    CHECK(fast.mapAnalog(0, 0, 255) == 255);

    _mock_analog_value = 512; // tengah rentang 0-1023
    int mapped = fast.mapAnalog(0, 0, 255);
    CHECK(mapped >= 126 && mapped <= 129); // toleransi pembulatan integer di sekitar tengah
}

void test_ema() {
    std::printf("\n-- test_ema --\n");
    IskakINO_ArduFast fast(5);
    float state = 0.0f;

    _mock_analog_value = 1000;
    fast.readEMA(0, state, 0.5f); // state: 0 -> 500
    CHECK(state > 400 && state < 600);

    // Setelah beberapa iterasi dengan alpha 0.5, state harus konvergen
    // mendekati nilai raw (1000), bukan menjauh.
    for (int i = 0; i < 10; i++) {
        fast.readEMA(0, state, 0.5f);
    }
    CHECK(state > 950); // sudah konvergen dekat 1000
}

int main() {
    test_every();
    test_once();
    test_reset_and_cancel();
    test_map_analog();
    test_ema();

    std::printf("\n============================\n");
    if (failures == 0) {
        std::printf("SEMUA TEST LULUS\n");
    } else {
        std::printf("%d TEST GAGAL\n", failures);
    }
    return failures == 0 ? 0 : 1;
}
