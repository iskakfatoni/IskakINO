# ⚡ Modul: IskakINO_ArduFast

Modul pendukung kecepatan I/O, eksekusi register mikrokontroler instan, penjadwalan multi-tasking non-blocking, serta filter pembacaan sensor analog/digital.

---

## 🛠️ Fitur Utama

1. **Direct Port Manipulation (`FastPin<P>`):** Operasi I/O ultra cepat (1-2 siklus instruksi clock pada AVR/ESP).
2. **Task Scheduler Non-Blocking:** Menjalankan fungsi periodik (`every()`) atau satu kali pemicu (`once()`) tanpa fungsi `delay()`.
3. **Software Debouncing & Noise Filter:** Membaca input tombol mekanik yang stabil bebas pantulan kontaktor (`readStable()`).
4. **Exponential Moving Average (EMA) Filter:** Penghalusan data sensor analog (`readEMA()`) untuk menghilangkan noise pembacaan ADC.
5. **Konversi & Normalisasi Data:** Memetakan dan menormalisasi pembacaan analog ke rentang 0.0 - 1.0 (`readNorm()`).
6. **Logging Terpadu:** Output serial `printf`-style dengan dukungan format desimal (`logFloat()`).

---

## 💻 Contoh Penggunaan Singkat

```cpp
#include <IskakINO.h>

IskakINO_ArduFast fast;
FastPin<13> ledPin;

void setup() {
    fast.begin(115200);
    ledPin.mode(OUTPUT);
}

void loop() {
    // Task 1: Kedipkan LED setiap 500 ms tanpa delay
    if (fast.every(500, 0)) {
        ledPin.toggle();
    }

    // Task 2: Baca sensor analog dengan filter EMA setiap 100 ms
    if (fast.every(100, 1)) {
        float smoothedVal = fast.readEMA(A0, 0.2f);
        fast.log("Sensor Terfilter: ");
        fast.logFloat(smoothedVal, 2);
    }
}
```

---

## 📖 Referensi API

### Inisialisasi & Logging
* `void begin(unsigned long baudRate = 115200)`: Inisialisasi komunikasi serial bawaan.
* `void log(const char* msg)`: Menampilkan pesan teks ke Serial Monitor.
* `void logf(const char* format, ...)`: Menampilkan pesan teks berformat `printf`.
* `void logFloat(float val, uint8_t decimals = 2)`: Menampilkan bilangan desimal ke Serial.
* `void setDebug(bool enabled)`: Mengaktifkan atau menonaktifkan logging debug.

### Task Scheduler (Non-Blocking)
* `bool every(uint32_t intervalMs, uint8_t taskSlot = 0)`: Mengembalikan `true` setiap kali interval waktu tercapai.
* `bool once(uint32_t delayMs, uint8_t taskSlot = 0)`: Mengembalikan `true` tepat satu kali setelah delay waktu tercapai.
* `void reset(uint8_t taskSlot)`: Mereset timer pada slot task tertentu.
* `void cancel(uint8_t taskSlot)`: Membatalkan timer pada slot task tertentu.

### Sensor & Input Helpers
* `float readEMA(uint8_t pin, float alpha = 0.1f)`: Membaca ADC dengan filter penghalus Exponential Moving Average.
* `int readStable(uint8_t pin, uint8_t sampleCount = 5)`: Membaca nilai digital/analog rata-rata dari beberapa sampel.
* `float readNorm(uint8_t pin)`: Membaca nilai analog dan menormalisasikannya ke rentang 0.0 s/d 1.0.

---

## 📂 Penjelasan Contoh Sketsa (`examples/01_ArduFast_TaskManager`)

* **Lokasi Sketsa:** [`examples/01_ArduFast_TaskManager/01_ArduFast_TaskManager.ino`](../examples/01_ArduFast_TaskManager/01_ArduFast_TaskManager.ino)
* **Platform Target:** Universal (Arduino AVR, ESP8266, ESP32).
* **Fokus Pembelajaran:**
  1. Menggunakan `FastPin<LED_BUILTIN>` untuk *toggling* output pin dalam hitungan nanodetik.
  2. Menjalankan dua jadwal independen secara paralel (`TASK_BLINK_ID` interval 500 ms dan `TASK_ADC_ID` interval 1000 ms).
  3. Membaca sensor analog dengan penghalusan `readEMA()` dan mencetak telemetri performa sistem.
