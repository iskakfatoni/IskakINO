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

class IskakINO_Scheduler {
  private:
    unsigned long *_prevMillis; // waktu referensi tiap task, dialokasikan runtime
    bool *_onceFired;           // penanda task once() yang sudah terpicu
    bool *_cancelled;           // penanda task yang sedang dinonaktifkan
    uint8_t _maxTasks;          // jumlah slot task, ditentukan saat konstruksi

    // Non-copyable: objek ini memegang memori alokasi runtime (new[]),
    // menyalinnya secara default (shallow copy) akan menyebabkan double-free.
    IskakINO_Scheduler(const IskakINO_Scheduler&);
    IskakINO_Scheduler& operator=(const IskakINO_Scheduler&);

  public:
    // maxTasks: jumlah slot task (ID 0..maxTasks-1). Default 10, sama
    // seperti default IskakINO_ArduFast agar perilaku familiar.
    explicit IskakINO_Scheduler(uint8_t maxTasks = 10);
    ~IskakINO_Scheduler();

    bool every(unsigned long interval, uint8_t id);   // berulang tiap interval
    bool once(unsigned long delay_ms, uint8_t id);    // trigger sekali setelah delay
    void reset(uint8_t id);                           // set ulang timer & re-arm once()/aktifkan lagi
    void cancel(uint8_t id);                          // nonaktifkan task sampai reset() dipanggil

    uint8_t maxTasks() const { return _maxTasks; }
};

#endif
