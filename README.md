# IskakINO Framework

[![Arduino Library](https://img.shields.io/badge/Arduino-Framework-00979D.svg?logo=arduino&logoColor=white)](https://www.arduino.cc/reference/en/libraries/)
[![Platform](https://img.shields.io/badge/platform-AVR%20%7C%20ESP8266%20%7C%20ESP32-green.svg)](#)
[![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)](LICENSE)
[![GitHub release](https://img.shields.io/github/v/release/iskakfatoni/IskakINO?color=blue&logo=github)](https://github.com/iskakfatoni/IskakINO/releases)
[![CI](https://github.com/iskakfatoni/IskakINO/actions/workflows/ci.yml/badge.svg)](https://github.com/iskakfatoni/IskakINO/actions)


[![Architecture](https://img.shields.io/badge/Architecture-Modular%20Kernel-orange.svg)](#)
[![Task Scheduler](https://img.shields.io/badge/Scheduler-Non--Blocking-informational.svg)](#)
[![Testing](https://img.shields.io/badge/Unit%20Tests-Mock%20Native-success.svg)](#)


[![Arduino Library](https://img.shields.io/badge/Arduino-Framework-00979D.svg?logo=arduino&logoColor=white)](https://www.arduino.cc/reference/en/libraries/)
[![Platform](https://img.shields.io/badge/platform-AVR%20%7C%20ESP8266%20%7C%20ESP32-green.svg)](#)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-11%20%2F%2017-00599C.svg?logo=cplusplus&logoColor=white)](https://en.cppreference.com/)
[![CI](https://github.com/iskakfatoni/IskakINO/actions/workflows/ci.yml/badge.svg)](https://github.com/iskakfatoni/IskakINO/actions)
[![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)](LICENSE)
[![GitHub release](https://img.shields.io/github/v/release/iskakfatoni/IskakINO?color=blue&logo=github)](https://github.com/iskakfatoni/IskakINO/releases)

[![GitHub Stars](https://img.shields.io/github/stars/iskakfatoni/IskakINO?style=social)](https://github.com/iskakfatoni/IskakINO/stargazers)
[![GitHub Forks](https://img.shields.io/github/forks/iskakfatoni/IskakINO?style=social)](https://github.com/iskakfatoni/IskakINO/network/members)


Library Arduino gabungan — hasil penggabungan seluruh ekosistem
`IskakINO_ArduFast`, `IskakINO_Storage`, `IskakINO_LiquidCrystal_I2C`,
`IskakINO_WifiPortal`, `IskakINO_FastNTP`, dan `IskakINO_SmartVoice` menjadi
**satu library** dengan shared core, plus lapisan framework opsional untuk
mengelola siklus hidup (`begin()`/`update()`) semua modul sekaligus.

> Status: **belum dirilis** (`version=1.0.0` di `library.properties`, belum
> ditag/dipublikasikan ke Arduino Library Manager).

## Kenapa digabung?

Sebelumnya tiap modul adalah library terpisah dengan versi & rilis sendiri.
Masalahnya: modul-modul itu sering dipakai BERSAMAAN (mis. WifiPortal +
FastNTP + LCD untuk jam pintar), tapi tidak bisa saling `depend` satu sama
lain secara bersih di Arduino Library Manager, dan banyak kode terduplikasi
(deteksi platform, logging, task manager non-blocking) di tiap modul.

Sekarang: satu `#include <IskakINO.h>`, satu versi, kode inti (`src/core/`)
dipakai bersama, dan modul yang butuh WiFi (WifiPortal, FastNTP) otomatis
"menghilang" secara aman di board non-WiFi seperti AVR Uno/Nano — bukan
gagal compile.

## Instalasi

Belum ada di Arduino Library Manager. Sementara ini:

1. Download/`git clone` repo ini.
2. Salin (atau symlink) foldernya ke `Documents/Arduino/libraries/IskakINO`.
3. Restart Arduino IDE.

Kalau lewat `arduino-cli` dari Git URL (butuh `enable_unsafe_install: true`
di config `arduino-cli`, karena `--git-url` melewati proses submission
Library Manager):
```bash
arduino-cli lib install --git-url https://github.com/iskakfatoni/IskakINO.git
```

## Modul yang tersedia

| Modul | Class utama | Platform | Fungsi |
|---|---|---|---|
| ArduFast | `IskakINO_ArduFast`, `FastPin<P>` | Universal (AVR/ESP32/ESP8266/RP2040) | Akses register GPIO langsung, task manager non-blocking, EMA filter |
| Storage | `IskakINO_Storage` (instance global `IskakStorage`) | Universal | Storage hybrid EEPROM/Preferences/LittleFS, mode log ring-buffer |
| LCD | `LiquidCrystal_I2C` | Universal (perlu I2C/Wire) | LCD karakter I2C, typewriter/scroll non-blocking, progress bar |
| SmartVoice | `IskakINO_SmartVoice` | Universal (perlu `Stream&` tambahan) | Kontrol modul MP3 DFPlayer Mini |
| WifiPortal | `IskakINO_WifiPortal` | **ESP32/ESP8266 saja** | Captive portal WiFi + custom parameter |
| FastNTP | `IskakINO_FastNTP` | **ESP32/ESP8266 saja** | Sinkronisasi waktu NTP non-blocking |

Modul "Universal" aman di-`#include <IskakINO.h>` di board apa pun. Modul
WiFi-only otomatis kosong (bukan error) kalau di-compile untuk board non-WiFi
— lihat [`src/wifi/IskakINO_WifiPortal.h`](src/wifi/IskakINO_WifiPortal.h)
untuk detail mekanismenya.

## Quick Start

Ada dua gaya pemakaian — pilih salah satu, keduanya didukung penuh.

### Gaya manual (kontrol penuh per modul)

```cpp
#include <IskakINO.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);

void setup() {
    fast.begin(115200);
    lcd.begin();
}

void loop() {
    lcd.update(); // wajib dipanggil tiap loop() untuk efek non-blocking LCD

    if (fast.every(1000, 0)) {
        lcd.setCursor(0, 0);
        lcd.print(fast.readStable(A0));
    }
}
```

### Gaya framework (satu `begin()`, satu `update()`)

```cpp
#include <IskakINO.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);

IskakINO_ArduFastModule fastMod(fast, 115200);
IskakINO_LCDModule      lcdMod(lcd);

void setup() {
    IskakINO.registerModule(&fastMod);
    IskakINO.registerModule(&lcdMod);
    IskakINO.begin();  // panggil begin() semua modul terdaftar, urut pendaftaran
}

void loop() {
    IskakINO.update(); // panggil update() semua modul terdaftar
}
```

Lihat [`examples/07_Unified_SmartClock`](examples/07_Unified_SmartClock) (gaya
manual) vs [`examples/08_Framework_Kernel`](examples/08_Framework_Kernel)
(gaya framework) untuk perbandingan langsung memakai 5 modul sekaligus.

## Contoh (`examples/`)

| # | Nama | Modul | Platform |
|---|---|---|---|
| 01 | `ArduFast_TaskManager` | ArduFast | Universal |
| 02 | `Storage_SaveLoad` | Storage | Universal |
| 03 | `LCD_TypewriterScroll` | LCD | Universal |
| 04 | `SmartVoice_PlayTrack` | SmartVoice | Universal |
| 05 | `WifiPortal_CaptivePortal` | WifiPortal | ESP32/ESP8266 |
| 06 | `FastNTP_ClockSync` | FastNTP | ESP32/ESP8266 |
| 07 | `Unified_SmartClock` | Semua (gaya manual) | ESP32/ESP8266 |
| 08 | `Framework_Kernel` | Semua (gaya framework) | ESP32/ESP8266 |

## Struktur proyek

```
IskakINO/
├── src/
│   ├── IskakINO.h              # entry point tunggal — #include ini saja
│   ├── core/                   # dipakai bersama semua modul
│   │   ├── IskakINO_Platform.h #   deteksi platform + FastPin<P>
│   │   ├── IskakINO_Result.h   #   enum status operasi terpusat
│   │   ├── IskakINO_Logger.h   #   logging printf-style, flag debug per-instance
│   │   ├── IskakINO_Scheduler.h#   task manager non-blocking every()/once()
│   │   ├── IskakINO_Module.h   #   interface begin()/update() seragam
│   │   ├── IskakINO_Kernel.h   #   registry/kernel (instance global `IskakINO`)
│   │   └── IskakINO_Version.h  #   satu sumber versi (ISKAKINO_VERSION)
│   ├── ardufast/  storage/  lcd/  voice/  wifi/  ntp/
│   │   ├── IskakINO_<Modul>.h/.cpp        # implementasi modul
│   │   └── IskakINO_<Modul>Module.h       # adapter utk IskakINO_Kernel
├── examples/      # 8 contoh, per-subfolder (konvensi Arduino Library Manager)
├── test/          # native unit test (g++ langsung, tanpa toolchain Arduino)
│   ├── native_check/   # mock Arduino.h dasar
│   ├── mock_esp32/     # mock WiFi/WebServer/DNSServer/Preferences/LittleFS
│   ├── mock_storage/   # mock EEPROM
│   ├── mock_lcd/       # mock Wire/Print
│   ├── mock_voice/     # mock Stream
│   ├── mock_ntp/       # mock UDP/WiFiUdp
│   ├── mock_kernel/    # test IskakINO_Kernel murni
│   └── mock_examples/  # test tiap file di examples/
├── library.properties
├── keywords.txt
├── CHANGELOG.md
└── .github/workflows/ci.yml
```

## Menjalankan test

Semua modul, kernel, dan contoh diverifikasi lewat **native unit test**
(compile+run langsung dengan `g++`, tanpa perlu toolchain Arduino/board
fisik) memakai mock hardware yang dibuat khusus untuk repo ini. Lihat
`.github/workflows/ci.yml` job `native-test` untuk daftar lengkap perintahnya
— setiap command di CI itu bisa dijalankan langsung dari root repo, contoh:

```bash
g++ -std=c++11 -Wall -I test/native_check -I src \
  test/native_check/Arduino.cpp src/core/IskakINO_Scheduler.cpp \
  test/native_check/test_core.cpp -o test_core
./test_core
```

Job `arduino-cli-smoke` di CI yang sama melakukan compile sungguhan lintas
board AVR/ESP32/ESP8266 lewat `arduino-cli` — inilah validasi paling akurat
sebelum dipakai di hardware asli.

## Menambah modul baru

Ringkasnya:
1. `src/<nama_modul>/IskakINO_<NamaModul>.h/.cpp` — kode modul.
2. Kalau platform-spesifik (butuh WiFi dkk.), bungkus **seluruh isi** file
   dengan `#if defined(ISKAKINO_HAS_WIFI)` (atau makro serupa) — JANGAN
   `#error`, karena Arduino mengkompilasi semua `.cpp` di `src/` terlepas
   dari pemakaian sketch.
3. `src/<nama_modul>/IskakINO_<NamaModul>Module.h` — adapter turunan
   `IskakINO_Module` (lihat 6 contoh yang sudah ada sebagai referensi).
4. Tambahkan 2 baris `#include` ke `src/IskakINO.h`.
5. Update `keywords.txt`, tambah entry di `CHANGELOG.md`.
6. Bikin contoh di `examples/` + native test di `test/mock_<nama_modul>/`.

## Migrasi dari library lama

Library standalone (`IskakINO_ArduFast`, `IskakINO_Storage`,
`IskakINO_LiquidCrystal_I2C`, `IskakINO_WifiPortal`, `IskakINO_FastNTP`,
`IskakINO_SmartVoice`) **deprecated** sejak penggabungan ini. Semua nama
class publik TIDAK BERUBAH (`IskakINO_WifiPortal`, `LiquidCrystal_I2C`, dst.
tetap sama persis), jadi migrasi cukup:

1. Uninstall library lama, install `IskakINO`.
2. Ganti `#include <IskakINO_WifiPortal.h>` (dst.) jadi `#include <IskakINO.h>`.
3. Selesai — tidak ada perubahan lain yang dibutuhkan di kode sketch.

Detail perubahan internal tiap modul ada di [`CHANGELOG.md`](CHANGELOG.md).

## Lisensi

MIT — lihat [`LICENSE`](LICENSE).

## Author

Iskak Fatoni ([github.com/iskakfatoni](https://github.com/iskakfatoni)) —
Nisnas Computer, SMKN 1 Jetis Mojokerto.
