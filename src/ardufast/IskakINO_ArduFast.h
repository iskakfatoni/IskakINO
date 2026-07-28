//
#ifndef ISKAKINO_ARDUFAST_H
#define ISKAKINO_ARDUFAST_H

#include <Arduino.h>

// PILOT REFACTOR — bagian dari penggabungan ekosistem IskakINO jadi satu
// library dengan shared core (lihat src/core/). Perilaku & signature semua
// fungsi publik di bawah TIDAK BERUBAH dari v1.1.0 — hanya implementasinya
// yang sekarang compose IskakINO_Scheduler (task manager) & IskakINO_Logger
// (logging) dari core/, bukan menulis ulang logikanya sendiri. FastPin<P>
// juga sudah pindah ke core/IskakINO_Platform.h (di-include transitif di
// bawah, jadi kode lama yang pakai `FastPin<5> pin;` tetap jalan tanpa ubah
// apa pun).
#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Scheduler.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Version.h"

// Ukuran buffer stack untuk logf() (printf-style). Bisa dikecilkan untuk
// board AVR yang RAM-nya sangat terbatas dengan mendefinisikan makro ini
// SEBELUM #include <IskakINO_ArduFast.h>, mis.:
//   #define ARDUFAST_LOGF_BUFFER_SIZE 32
//   #include <IskakINO_ArduFast.h>
// (diteruskan ke ISKAKINO_LOGF_BUFFER_SIZE milik core/IskakINO_Logger.h)
#ifndef ARDUFAST_LOGF_BUFFER_SIZE
#define ARDUFAST_LOGF_BUFFER_SIZE 64
#endif
#ifndef ISKAKINO_LOGF_BUFFER_SIZE
#define ISKAKINO_LOGF_BUFFER_SIZE ARDUFAST_LOGF_BUFFER_SIZE
#endif

// BEKU (frozen) — menandai kode modul ini berasal dari rilis standalone
// IskakINO_ArduFast v1.1.0, TIDAK lagi naik mengikuti rilis IskakINO.
// Kode konsumen lama yang masih cek `#if ARDUFAST_VERSION >= 10100` tetap
// valid selamanya karena nilainya tidak berubah lagi. Untuk versi library
// yang sesungguhnya (yang naik tiap rilis), pakai ISKAKINO_VERSION dari
// core/IskakINO_Version.h.
#define ARDUFAST_VERSION_MAJOR 1
#define ARDUFAST_VERSION_MINOR 1
#define ARDUFAST_VERSION_PATCH 0
#define ARDUFAST_VERSION ((ARDUFAST_VERSION_MAJOR * 10000UL) + \
                           (ARDUFAST_VERSION_MINOR * 100UL) + \
                           (ARDUFAST_VERSION_PATCH))

// --- Class Framework ---
class IskakINO_ArduFast {
private:
    IskakINO_Scheduler _scheduler; // dulu: _prevMillis/_onceFired/_cancelled manual
    IskakINO_Logger _logger;       // dulu: Serial.print langsung di tiap fungsi log*()

    // Non-copyable: _scheduler memegang memori alokasi runtime (new[]),
    // menyalinnya secara default akan menyebabkan double-free.
    IskakINO_ArduFast(const IskakINO_ArduFast&);
    IskakINO_ArduFast& operator=(const IskakINO_ArduFast&);

public:
    // maxTasks: jumlah slot task (ID 0..maxTasks-1). Default 10, sama
    // seperti versi sebelumnya — kompatibel dengan sketsa lama tanpa perubahan.
    explicit IskakINO_ArduFast(uint8_t maxTasks = 10);

    void begin(unsigned long baud = 115200);

    // --- Task Manager (delegasi ke IskakINO_Scheduler) ---
    bool every(unsigned long interval, uint8_t id);   // berulang tiap interval
    bool once(unsigned long delay_ms, uint8_t id);    // trigger sekali setelah delay
    void reset(uint8_t id);                           // set ulang timer & re-arm once()/aktifkan lagi
    void cancel(uint8_t id);                          // nonaktifkan task sampai reset() dipanggil

    // --- Analog ---
    int readNorm(uint8_t pin);
    int readStable(uint8_t pin, uint8_t samples = 16);
    int mapAnalog(uint8_t pin, int outMin, int outMax);
    // Exponential Moving Average — non-blocking, cocok dibaca tiap loop().
    // 'state' dimiliki & disimpan oleh pemanggil (bukan disimpan di dalam
    // library) supaya RAM yang dipakai proporsional dengan jumlah sensor
    // yang benar-benar dipakai user. Inisialisasi 'state' dengan pembacaan
    // pertama sebelum loop() supaya EMA tidak mulai dari nol.
    // alpha lebih besar = lebih responsif tapi kurang stabil (0.0-1.0).
    float readEMA(uint8_t pin, float &state, float alpha = 0.1f);

    // --- Logging (delegasi ke IskakINO_Logger) ---
    void log(const __FlashStringHelper* msg);
    void log(const __FlashStringHelper* msg, long val);
    void logFloat(const __FlashStringHelper* msg, float val, uint8_t decimals = 2);
    // CATATAN AVR: specifier %f TIDAK didukung oleh vsnprintf bawaan AVR
    // tanpa flag linker tambahan (-lprintf_flt) — untuk nilai float/desimal
    // pakai logFloat().
    void logf(const __FlashStringHelper* fmt, ...);

    // BARU (pilot refactor): dulu log()/logFloat()/logf() milik ArduFast
    // SELALU mencetak ke Serial tak peduli apa pun. Sekarang logger internal
    // di-default aktif (setDebug(true) dipanggil di constructor) supaya
    // perilaku lama tetap sama persis — tapi kalau Kak Iskak mau membungkam
    // logging ArduFast tanpa menghapus baris logf() di sketsa, tinggal
    // panggil fast.setDebug(false). Opsional, tidak wajib dipakai.
    void setDebug(bool debugMode) { _logger.setDebug(debugMode); }

    // --- Wrapper IO standar (untuk pin dinamis / non-template) ---
    void pinMode(uint8_t pin, uint8_t mode);
    void digitalWrite(uint8_t pin, uint8_t val);
    int digitalRead(uint8_t pin);
};

// Deklarasi instance agar bisa diakses global
//extern IskakINO_ArduFast ArduFast;

#endif
