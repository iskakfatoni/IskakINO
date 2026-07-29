#include "IskakINO_Scheduler.h"

IskakINO_Scheduler::IskakINO_Scheduler(uint8_t maxTasks) {
    _maxTasks = maxTasks;
    _prevMillis = new unsigned long[_maxTasks];
    _onceFired  = new bool[_maxTasks];
    _cancelled  = new bool[_maxTasks];
    for (uint8_t i = 0; i < _maxTasks; i++) {
        _prevMillis[i] = 0;
        _onceFired[i]  = false;
        _cancelled[i]  = false;
    }
}

IskakINO_Scheduler::~IskakINO_Scheduler() {
    delete[] _prevMillis;
    delete[] _onceFired;
    delete[] _cancelled;
}

bool IskakINO_Scheduler::every(unsigned long interval, uint8_t id) {
    if (id >= _maxTasks || _cancelled[id]) return false;
    unsigned long current = millis();
    if (current - _prevMillis[id] >= interval) {
        _prevMillis[id] = current;
        return true;
    }
    return false;
}

// Trigger sekali setelah delay (dihitung sejak objek dibuat / reset()
// terakhir untuk id ini, konsisten dengan cara every() menghitung dari
// _prevMillis).
bool IskakINO_Scheduler::once(unsigned long delay_ms, uint8_t id) {
    if (id >= _maxTasks || _cancelled[id] || _onceFired[id]) return false;
    unsigned long current = millis();
    if (current - _prevMillis[id] >= delay_ms) {
        _onceFired[id] = true;
        return true;
    }
    return false;
}

// Set ulang timer sebuah task ID: mengaktifkan kembali task yang
// di-cancel(), dan mengizinkan once() untuk id tsb terpicu lagi.
void IskakINO_Scheduler::reset(uint8_t id) {
    if (id >= _maxTasks) return;
    _prevMillis[id] = millis();
    _onceFired[id] = false;
    _cancelled[id] = false;
}

// Nonaktifkan sementara sebuah task ID. every()/once() untuk id ini akan
// selalu return false sampai reset() dipanggil.
void IskakINO_Scheduler::cancel(uint8_t id) {
    if (id >= _maxTasks) return;
    _cancelled[id] = true;
}
