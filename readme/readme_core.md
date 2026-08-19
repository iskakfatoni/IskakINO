# 🧠 IskakINO Core & Framework Kernel

Pondasi utama (*Shared Core*) dan kerangka kerja modular (*Kernel Registry*) yang digunakan oleh seluruh modul di dalam ekosistem **IskakINO**.

---

## 📌 Komponen Core

Direktori `src/core/` menyediakan serangkaian header utilitas ringan tanpa dependensi eksternal:

| Berkas | Fungsi & Kemampuan Utama |
|---|---|
| [`IskakINO_Platform.h`](../src/core/IskakINO_Platform.h) | Deteksi arsitektur mikroprosesor (`ISKAKINO_PLATFORM_AVR`, `ISKAKINO_PLATFORM_ESP32`, `ISKAKINO_PLATFORM_ESP8266`) & driver register direct GPIO template `FastPin<P>`. |
| [`IskakINO_Scheduler.h`](../src/core/IskakINO_Scheduler.h) | Task manager asinkron non-blocking (`every()`, `once()`, `reset()`, `cancel()`) berbasis slot konfigurable. |
| [`IskakINO_Logger.h`](../src/core/IskakINO_Logger.h) | Utilitas logging seragam dengan sintaks `printf`-style dan kontrol debug runtime (`setDebug()`). |
| [`IskakINO_Result.h`](../src/core/IskakINO_Result.h) | Standarisasi enum kode status dan hasil operasi di seluruh modul. |
| [`IskakINO_Version.h`](../src/core/IskakINO_Version.h) | Single source of truth untuk nomor versi library (`ISKAKINO_VERSION`). |
| [`IskakINO_Module.h`](../src/core/IskakINO_Module.h) | Interface dasar (`begin()`, `update()`, `moduleName()`) untuk adapter modul. |
| [`IskakINO_Kernel.h`](../src/core/IskakINO_Kernel.h) | Objek singleton global `IskakINO` untuk mendaftarkan dan menjalankan siklus hidup seluruh modul secara serentak. |

---

## ⚡ Direct Register I/O (`FastPin<P>`)

`FastPin<P>` adalah template C++ zero-cost abstraction untuk manipulasi pin mikrokontroler langsung ke register hardware:

```cpp
#include <IskakINO.h>

FastPin<13> ledPin;

void setup() {
    ledPin.mode(OUTPUT);
}

void loop() {
    ledPin.high();    // Set HIGH dalam 1-2 siklus clock
    delay(500);
    ledPin.low();     // Set LOW dalam 1-2 siklus clock
    delay(500);
    ledPin.toggle();  // Toggle pin langsung via register PINx / W1TC
    delay(500);
}
```

---

## ⚙️ Kernel Framework (`IskakINO_Kernel`)

Kernel memungkinkan beberapa modul berjalan bersamaan dengan manajemen siklus hidup terpusat:

```cpp
#include <IskakINO.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);

IskakINO_ArduFastModule fastMod(fast, 115200);
IskakINO_LCDModule      lcdMod(lcd);

void setup() {
    IskakINO.registerModule(&fastMod);
    IskakINO.registerModule(&lcdMod);
    
    // Memanggil begin() seluruh modul sesuai urutan pendaftaran
    IskakINO.begin();
}

void loop() {
    // Memanggil update() seluruh modul secara otomatis
    IskakINO.update();
}
```

---

## 📖 Penjelasan Contoh Terkait (`examples/08_Framework_Kernel`)

* **Lokasi Sketsa:** [`examples/08_Framework_Kernel/08_Framework_Kernel.ino`](../examples/08_Framework_Kernel/08_Framework_Kernel.ino)
* **Platform Target:** ESP32 / ESP8266 (atau board lain sesuai modul yang didaftarkan).
* **Fokus Pembelajaran:**
  1. Mendaftarkan 5 adapter modul sekaligus (`fastMod`, `storageMod`, `lcdMod`, `portalMod`, `ntpMod`) ke objek kernel `IskakINO`.
  2. Menyederhanakan fungsi `setup()` menjadi satu baris `IskakINO.begin()`.
  3. Menyederhanakan fungsi `loop()` menjadi satu baris `IskakINO.update()` untuk menggerakkan mesin waktu NTP, captive portal WiFi, dan animasi LCD.
  4. Menunjukkan bagaimana kernel mempermudah integrasi proyek berskala besar tanpa sketsa yang berantakan.
