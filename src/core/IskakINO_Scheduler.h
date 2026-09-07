/*
 * src/core/IskakINO_Scheduler.h
 * Task manager non-blocking berbasis millis() (every/once/reset/cancel).
 *
 * SUMBER: diekstrak dari task manager di IskakINO_ArduFast v1.1.0.
 * Dipisah jadi kelas mandiri (bukan menempel di IskakINO_ArduFast) karena
 * pola ini sebenarnya dipakai ulang secara implisit di banyak modul lain
 * dengan cara masing-masing menulis "unsigned long _prevMillis" sendiri:
 *   - FastNTP  : cooldown antar percobaan sinkronisasi di state machine
 *   - WifiPortal: enforce portal timeout
 *   - SmartVoice: timeout saat menunggu frame 10-byte dari DFPlayer
 * Dengan Scheduler ini, modul-modul tsb tinggal compose satu instance dan
 * pakai every()/once() dengan id slot masing-masing, bukan variabel
 * _prevMillis manual yang gampang lupa direset di titik yang salah.
 *
 * Perilaku every()/once()/reset()/cancel() identik 1:1 dengan versi
 * ArduFast supaya modul ArduFast sendiri nantinya tinggal compose kelas
 * ini (bukan menduplikasi ulang logikanya).
 */

#ifndef ISKAKINO_SCHEDULER_H
#define ISKAKINO_SCHEDULER_H

#include <Arduino.h>

struct TaskState {
    unsigned long prevMillis; // waktu referensi tiap task
    bool onceFired;           // penanda task once() yang sudah terpicu
    bool cancelled;           // penanda task yang sedang dinonaktifkan
};

class IskakINO_Scheduler {
  protected:
    TaskState *_tasks;          // terpadu 1 array struct alih-alih 3 pointer heap terpisah
    uint8_t _maxTasks;          // jumlah slot task
    bool _isDynamic;            // penanda apakah _tasks mengalokasikan heap

    // Non-copyable: menyalin objek ini secara default akan menyebabkan double-free.
    IskakINO_Scheduler(const IskakINO_Scheduler&);
    IskakINO_Scheduler& operator=(const IskakINO_Scheduler&);

  public:
    // maxTasks: jumlah slot task (ID 0..maxTasks-1). Default 10.
    explicit IskakINO_Scheduler(uint8_t maxTasks = 10);
    // Overload untuk Zero Heap / Static buffer allocation
    IskakINO_Scheduler(TaskState* staticBuffer, uint8_t maxTasks);
    virtual ~IskakINO_Scheduler();

    // Hot-path inlining untuk perbandingan millis() tanpa overhead panggilan fungsi
    inline bool every(unsigned long interval, uint8_t id) {
        if (!_tasks || id >= _maxTasks || _tasks[id].cancelled) return false;
        unsigned long current = millis();
        if (current - _tasks[id].prevMillis >= interval) {
            _tasks[id].prevMillis = current;
            return true;
        }
        return false;
    }

    inline bool once(unsigned long delay_ms, uint8_t id) {
        if (!_tasks || id >= _maxTasks || _tasks[id].cancelled || _tasks[id].onceFired) return false;
        unsigned long current = millis();
        if (current - _tasks[id].prevMillis >= delay_ms) {
            _tasks[id].onceFired = true;
            return true;
        }
        return false;
    }

    void reset(uint8_t id);
    void cancel(uint8_t id);

    uint8_t maxTasks() const { return _maxTasks; }
};

// Zero Heap / Static Storage Variant:
// Mengalokasikan array TaskState di stack/BSS (statis) tanpa malloc/new di heap.
template <uint8_t N = 10>
class IskakINO_SchedulerStatic : public IskakINO_Scheduler {
  private:
    TaskState _staticTasks[N];
  public:
    IskakINO_SchedulerStatic() : IskakINO_Scheduler(_staticTasks, N) {}
};

#endif
