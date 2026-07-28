/*
 * 01_ArduFast_TaskManager.ino
 * Modul: IskakINO_ArduFast (universal — AVR/ESP32/ESP8266/RP2040)
 *
 * Menunjukkan pola dasar task manager non-blocking every()/once(), plus
 * pembacaan analog ternormalisasi & EMA filter. Tidak perlu koneksi WiFi/
 * hardware tambahan apa pun — cocok jadi titik awal belajar IskakINO.
 *
 * Rangkaian: LED bawaan board (LED_BUILTIN), potensiometer opsional di A0.
 */

#include <IskakINO.h>

IskakINO_ArduFast fast;

// ID slot task (0..9, default maxTasks=10)
#define TASK_BLINK   0
#define TASK_SENSOR  1

float emaState = 0; // state EMA disimpan di sisi sketch (bukan di dalam library)

void setup() {
    fast.begin(115200);
    fast.pinMode(LED_BUILTIN, OUTPUT);

    // Inisialisasi EMA dengan pembacaan pertama supaya tidak mulai dari nol
    emaState = fast.readNorm(A0);

    fast.log(F("ArduFast siap. Task manager berjalan..."));
}

void loop() {
    // Kedip LED tiap 500ms, tanpa delay() — loop() tetap responsif
    if (fast.every(500, TASK_BLINK)) {
        static bool state = false;
        state = !state;
        fast.digitalWrite(LED_BUILTIN, state);
    }

    // Baca & log sensor tiap 1 detik
    if (fast.every(1000, TASK_SENSOR)) {
        int stable = fast.readStable(A0, 16);      // rata-rata 16 sampel, tahan noise
        float ema = fast.readEMA(A0, emaState, 0.1f); // filter halus, responsif tapi stabil
        int persen = fast.mapAnalog(A0, 0, 100);   // langsung dipetakan ke 0-100

        fast.log(F("Stable"), stable);
        fast.logFloat(F("EMA"), ema, 1);
        fast.logf(F("Persen: %d%%"), persen);
    }

    // Contoh once(): jalan SEKALI 3 detik setelah boot, lalu tidak lagi
    // sampai fast.reset(2) dipanggil.
    if (fast.once(3000, 2)) {
        fast.log(F("3 detik sejak boot -- pesan ini cuma muncul sekali."));
    }
}
