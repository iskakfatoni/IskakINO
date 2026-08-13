#include "IskakINO_Scheduler.h"

IskakINO_Scheduler::IskakINO_Scheduler(uint8_t maxTasks) {
    _maxTasks = maxTasks;
    _isDynamic = true;
    _tasks = new TaskState[_maxTasks];
    for (uint8_t i = 0; i < _maxTasks; i++) {
        _tasks[i].prevMillis = 0;
        _tasks[i].onceFired  = false;
        _tasks[i].cancelled  = false;
    }
}

IskakINO_Scheduler::IskakINO_Scheduler(TaskState* staticBuffer, uint8_t maxTasks) {
    _maxTasks = maxTasks;
    _isDynamic = false;
    _tasks = staticBuffer;
    for (uint8_t i = 0; i < _maxTasks; i++) {
        _tasks[i].prevMillis = 0;
        _tasks[i].onceFired  = false;
        _tasks[i].cancelled  = false;
    }
}

IskakINO_Scheduler::~IskakINO_Scheduler() {
    if (_isDynamic && _tasks != NULL) {
        delete[] _tasks;
        _tasks = NULL;
    }
}

// Set ulang timer sebuah task ID: mengaktifkan kembali task yang
// di-cancel(), dan mengizinkan once() untuk id tsb terpicu lagi.
void IskakINO_Scheduler::reset(uint8_t id) {
    if (id >= _maxTasks || _tasks == NULL) return;
    _tasks[id].prevMillis = millis();
    _tasks[id].onceFired = false;
    _tasks[id].cancelled = false;
}

// Nonaktifkan sementara sebuah task ID. every()/once() untuk id ini akan
// selalu return false sampai reset() dipanggil.
void IskakINO_Scheduler::cancel(uint8_t id) {
    if (id >= _maxTasks || _tasks == NULL) return;
    _tasks[id].cancelled = true;
}
