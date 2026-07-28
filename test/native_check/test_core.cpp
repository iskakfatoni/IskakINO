#include "Arduino.h"
#include "../../src/core/IskakINO_Platform.h"
#include "../../src/core/IskakINO_Result.h"
#include "../../src/core/IskakINO_Logger.h"
#include "../../src/core/IskakINO_Scheduler.h"
#include <cassert>
#include <cstdio>

extern unsigned long _mock_millis_value;

int main() {
    // --- Result ---
    IskakINO_Result r = IskakINO_Result::WRITE_FAILED;
    assert(strcmp(IskakINO_ResultToString(r), "WRITE_FAILED") == 0);
    IskakStorageResult rs = IskakINO_Result::OK; // alias kompat mundur
    assert(rs == IskakINO_Result::OK);

    // --- Logger ---
    IskakINO_Logger log;
    log.setDebug(true);
    assert(log.isDebug());
    log.log(F("test message"));
    log.log(F("value"), 42);
    log.logFloat(F("suhu"), 25.5f);
    log.logf(F("logf %d"), 7);
    log.logResult(F("op"), IskakINO_Result::TIMEOUT);

    // --- Scheduler ---
    // Catatan: _prevMillis diinisialisasi 0 di constructor, jadi every()
    // TIDAK otomatis fire di panggilan pertama pada t=0 kecuali interval
    // sudah 0 juga (0-0 >= interval hanya true kalau interval==0). Ini
    // konsisten dengan perilaku task manager ArduFast asli.
    _mock_millis_value = 0;
    IskakINO_Scheduler sched(5);
    assert(sched.maxTasks() == 5);
    assert(sched.every(100, 0) == false);  // baru t=0, interval belum lewat
    _mock_millis_value = 150;
    assert(sched.every(100, 0) == true);   // interval elapsed
    assert(sched.every(100, 0) == false);  // baru saja fire, belum lewat lagi

    // id=1 punya baseline _prevMillis independen (masih 0 sejak constructor)
    assert(sched.once(200, 1) == false);   // t=150, 150-0=150 < 200 -> belum
    _mock_millis_value = 200;
    assert(sched.once(200, 1) == true);    // t=200, 200-0=200 >= 200 -> fire
    assert(sched.once(200, 1) == false);   // won't fire again until reset()
    sched.reset(1);
    assert(sched.once(0, 1) == true);      // re-armed

    sched.cancel(2);
    assert(sched.every(0, 2) == false);    // cancelled task never fires
    sched.reset(2);
    assert(sched.every(0, 2) == true);     // re-activated after reset

    // out-of-range id harus aman (tidak crash), cuma return false
    assert(sched.every(0, 99) == false);
    assert(sched.once(0, 99) == false);

    printf("OK: semua smoke-test core module lolos\n");
    return 0;
}
