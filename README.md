# IskakINO Framework

[![Arduino Library](https://img.shields.io/badge/Arduino-Framework-00979D.svg?logo=arduino&logoColor=white)](https://www.arduino.cc/reference/en/libraries/)
[![Platform](https://img.shields.io/badge/platform-AVR%20%7C%20ESP8266%20%7C%20ESP32-green.svg)](#)
[![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)](LICENSE)
[![GitHub release](https://img.shields.io/github/v/release/iskakfatoni/IskakINO?color=blue&logo=github)](https://github.com/iskakfatoni/IskakINO/releases)
[![CI](https://github.com/iskakfatoni/IskakINO/actions/workflows/ci.yml/badge.svg)](https://github.com/iskakfatoni/IskakINO/actions)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-11%20%2F%2017-00599C.svg?logo=cplusplus&logoColor=white)](https://en.cppreference.com/)
[![PlatformIO Registry](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg?logo=platformio&logoColor=white)](library.json)
[![Architecture](https://img.shields.io/badge/Architecture-Modular%20Kernel-orange.svg)](#)
[![Task Scheduler](https://img.shields.io/badge/Scheduler-Non--Blocking-informational.svg)](#)
[![GitHub Stars](https://img.shields.io/github/stars/iskakfatoni/IskakINO?style=social)](https://github.com/iskakfatoni/IskakINO/stargazers)
[![GitHub Forks](https://img.shields.io/github/forks/iskakfatoni/IskakINO?style=social)](https://github.com/iskakfatoni/IskakINO/network/members)

**IskakINO** adalah library ekosistem Arduino terpadu (*Unified Library*) — menggabungkan modul `IskakINO_ArduFast`, `IskakINO_Storage`, `IskakINO_LiquidCrystal_I2C`, `IskakINO_WifiPortal`, `IskakINO_FastNTP`, dan `IskakINO_SmartVoice` menjadi **satu library ringkas** dengan shared core (`src/core/`), plus kernel modular opsional untuk mengelola siklus hidup (`begin()` / `update()`) seluruh modul secara otomatis.

---

## 🌟 Kenapa Memakai IskakINO?

Sebelumnya tiap modul merupakan library mandiri yang terpisah. Ketika dipakai bersamaan (misalnya *WifiPortal* + *FastNTP* + *LCD* + *SmartVoice* untuk bel sekolah otomatis atau jam pintar), sering terjadi duplikasi kode, bentrok nama, dan kesulitan dependensi antar-library.

**Keunggulan IskakINO:**
- **Single Entry Point:** Cukup satu baris `#include <IskakINO.h>` untuk mengakses seluruh modul.
- **Shared Core Efisien:** Deteksi platform, driver register I/O cepat, logging terpadu, result codes, dan task scheduler non-blocking digunakan bersama tanpa redundansi memori.
- **Platform-Safe:** Modul universal bekerja di semua board (AVR, ESP8266, ESP32). Modul yang membutuhkan koneksi WiFi (*WifiPortal* & *FastNTP*) otomatis non-aktif secara aman pada board non-WiFi (seperti Arduino Uno/Nano/Mega) tanpa menimbulkan error kompilasi.
- **Zero Overhead:** Modul yang tidak dipakai tidak akan membebani ukuran memori flash mikrokontroler.

---

## 📦 Instalasi

### 1. Arduino IDE
1. Download atau `git clone` repositori ini:
   ```bash
   git clone https://github.com/iskakfatoni/IskakINO.git
   ```
2. Pindahkan folder `IskakINO` ke direktori library Arduino Anda:
   - **Windows:** `Documents/Arduino/libraries/IskakINO`
   - **Linux / macOS:** `~/Arduino/libraries/IskakINO`
3. Restart Arduino IDE.

### 2. Arduino CLI
```bash
arduino-cli lib install --git-url https://github.com/iskakfatoni/IskakINO.git
```

### 3. PlatformIO
Tambahkan dependensi pada file `platformio.ini` proyek Anda:
```ini
lib_deps =
    https://github.com/iskakfatoni/IskakINO.git
```

---

## 🧩 Modul & Fitur Utama

| Modul | Class Utama | Platform | Fungsi & Kemampuan |
|---|---|---|---|
| **ArduFast** | `IskakINO_ArduFast`, `FastPin<P>` | Universal (AVR / ESP32 / ESP8266) | Direct port manipulation (nanosecond I/O), software debounce, generator pulsa, & filter EMA. |
| **Storage** | `IskakINO_Storage` (`IskakStorage`) | Universal (AVR / ESP32 / ESP8266) | Multi-backend hybrid (EEPROM di AVR, Preferences di ESP32, LittleFS di ESP8266) dengan CRC32, enkripsi ringan XOR, String helper, & Ring-buffer log. |
| **LCD** | `LiquidCrystal_I2C` | Universal (I2C / Wire) | Driver I2C LCD dengan animasi teks non-blocking (*typewriter*, *smooth scrolling*, *marquee*, & *progress bar*). |
| **SmartVoice** | `IskakINO_SmartVoice` | Universal (Stream / Serial) | Driver MP3 DFPlayer Mini non-blocking berbasis *state-machine*, antrean lagu (*playback queue*), & volume manager. |
| **WifiPortal** | `IskakINO_WifiPortal` | **ESP32 & ESP8266** | Captive Portal WiFi AP, konfigurasi parameter kustom via Web UI dinamis, & OTA firmware update. |
| **FastNTP** | `IskakINO_FastNTP` | **ESP32 & ESP8266** | Sinkronisasi waktu internet NTP non-blocking, format waktu instan, lokalisasi nama hari/bulan (Bahasa Indonesia & English), serta offset zona waktu. |

---

## 🚀 Panduan Penggunaan (Quick Start)

IskakINO mendukung dua paradigma penggunaan:

### 1. Gaya Manual (Modular & Kontrol Penuh)
Cocok jika Anda ingin mengontrol inisialisasi dan timing setiap modul secara mandiri:

```cpp
#include <IskakINO.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);

void setup() {
    fast.begin(115200);
    lcd.begin();
    lcd.typewriter("Halo IskakINO!", 0, 0, 80);
}

void loop() {
    lcd.update(); // Update animasi non-blocking LCD

    // Task non-blocking setiap 1000 ms
    if (fast.every(1000, 0)) {
        fast.log("Sistem Aktif - Uptime: %lu ms", millis());
    }
}
```

### 2. Gaya Framework / Kernel (Terpusat & Otomatis)
Mendaftarkan modul ke Kernel global `IskakINO` sehingga `begin()` dan `update()` dikelola otomatis:

```cpp
#include <IskakINO.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);

IskakINO_ArduFastModule fastMod(fast, 115200);
IskakINO_LCDModule      lcdMod(lcd);

void setup() {
    IskakINO.registerModule(&fastMod);
    IskakINO.registerModule(&lcdMod);
    
    // Memanggil begin() seluruh modul yang terdaftar
    IskakINO.begin();
}

void loop() {
    // Memanggil update() seluruh modul yang terdaftar
    IskakINO.update();
}
```

---

## 📂 Ringkasan Contoh Sketsa (`examples/`)

| No | Folder Contoh | Modul Terlibat | Platform Target | Deskripsi Ringkas |
|---|---|---|---|---|
| **01** | [`01_ArduFast_TaskManager`](examples/01_ArduFast_TaskManager/) | ArduFast, Core Scheduler | Universal | Task scheduler non-blocking, manipulasi pin register cepat, dan filter ADC. |
| **02** | [`02_Storage_SaveLoad`](examples/02_Storage_SaveLoad/) | Storage | Universal | Penyimpanan tipe struct, Arduino String, log ring-buffer, dan enkripsi XOR. |
| **03** | [`03_LCD_TypewriterScroll`](examples/03_LCD_TypewriterScroll/) | LCD I2C | Universal | Efek teks mesin ketik, teks berjalan (*marquee*), dan baris progress bar. |
| **04** | [`04_SmartVoice_PlayTrack`](examples/04_SmartVoice_PlayTrack/) | SmartVoice | Universal | Pemutaran trek suara DFPlayer Mini, antrean pesan suara, dan feedback serial. |
| **05** | [`05_WifiPortal_CaptivePortal`](examples/05_WifiPortal_CaptivePortal/) | WifiPortal | ESP32 / ESP8266 | Portal konfigurasi WiFi interaktif (Captive Portal) dengan parameter dinamis. |
| **06** | [`06_FastNTP_ClockSync`](examples/06_FastNTP_ClockSync/) | FastNTP | ESP32 / ESP8266 | Sinkronisasi jam internet, kalkulasi waktu lokal, dan teks tanggal multibahasa. |
| **07** | [`07_Unified_SmartClock`](examples/07_Unified_SmartClock/) | Semua Modul (Manual) | ESP32 / ESP8266 | Jam digital pintar lengkap (WiFi + NTP + LCD + Storage + Voice). |
| **08** | [`08_Framework_Kernel`](examples/08_Framework_Kernel/) | Semua Modul (Kernel) | ESP32 / ESP8266 | Contoh arsitektur terpadu menggunakan framework `IskakINO_Kernel`. |
| **09** | [`09_SmartSchoolBell`](examples/09_SmartSchoolBell/) | Lengkap (Real Project) | ESP32 / ESP8266 | Proyek bel sekolah otomatis berbasis jadwal NTP, pengumuman suara MP3, LCD, & Web Portal. |

---

## 📖 Penjelasan & Panduan Tiap Contoh (Examples)

Berikut adalah penjelasan mendalam mengenai tujuan, fitur utama, dan cara kerja masing-masing contoh sketsa:

### 01. `01_ArduFast_TaskManager`
- **Fokus:** Kecepatan eksekusi I/O & Penjadwalan Multi-tasking.
- **Platform:** Universal (AVR, ESP8266, ESP32).
- **Penjelasan:** Menunjukkan cara menggantikan `digitalWrite()` dan `delay()` dengan `FastPin<P>` (akses register langsung secepat siklus clock) serta task scheduler non-blocking `every()` dan `once()`. Dilengkapi demonstrasi pembacaan tombol anti-bouncing (`debounce()`) dan smoothing sensor analog menggunakan filter EMA (`readStable()`).

### 02. `02_Storage_SaveLoad`
- **Fokus:** Penyimpanan Data Non-Volatile Multi-Platform.
- **Platform:** Universal (AVR = EEPROM, ESP32 = Preferences/NVS, ESP8266 = LittleFS).
- **Penjelasan:** Memberikan contoh praktis penyimpanan data konfigurasi terstruktur (`struct`), teks dinamis (`String`), serta pencatatan riwayat sistem dengan mode ring-buffer log (`addLog()`, `readLog()`). Dilengkapi proteksi integritas data CRC32 dan enkripsi stream XOR opsional.

### 03. `03_LCD_TypewriterScroll`
- **Fokus:** Tampilan Visual & Animasi Teks Non-blocking pada I2C LCD.
- **Platform:** Universal (membutuhkan modul LCD 16x2 atau 20x4 I2C).
- **Penjelasan:** Menampilkan teks dengan efek mesin ketik (*typewriter*), teks berjalan horizontal yang mulus (*smooth marquee scroll*), dan grafik *progress bar* custom character tanpa menghentikan atau memperlambat alur program utama (`loop()` tetap responsif).

### 04. `04_SmartVoice_PlayTrack`
- **Fokus:** Pengendalian Audio MP3 DFPlayer Mini.
- **Platform:** Universal (memerlukan jalur Serial / SoftwareSerial).
- **Penjelasan:** Memutar rekaman suara MP3 berbasis *state-machine* non-blocking. Mendukung pengaturan volume digital, equalizer preset, pemutaran langsung nomor track/folder, dan mekanisme antrean pemutaran audio berurutan (*playback queue*).

### 05. `05_WifiPortal_CaptivePortal`
- **Fokus:** Konfigurasi Jaringan Mandiri via Web Portal.
- **Platform:** ESP32 dan ESP8266.
- **Penjelasan:** Menyediakan portal web konfigurasi WiFi captive portal. Jika koneksi ke router gagal, ESP secara otomatis beralih menjadi Access Point (AP) dan mengarahkan browser pengguna langsung ke halaman konfigurasi untuk memasukkan SSID, Password, serta parameter custom (misalnya Token MQTT, Host Server, dll.) tanpa perlu hardcode di sketsa.

### 06. `06_FastNTP_ClockSync`
- **Fokus:** Sinkronisasi Waktu Internet Akurat.
- **Platform:** ESP32 dan ESP8266.
- **Penjelasan:** Mengambil waktu terkini dari server NTP dunia secara asinkron tanpa memblokir CPU. Dilengkapi pengubah zona waktu otomatis, format waktu siap cetak (`HH:MM:SS`), serta teks nama hari dan bulan otomatis dalam Bahasa Indonesia maupun Bahasa Inggris.

### 07. `07_Unified_SmartClock`
- **Fokus:** Integrasi 5 Modul Sekaligus (Gaya Modular Manual).
- **Platform:** ESP32 dan ESP8266.
- **Penjelasan:** Membangun jam dinding digital pintar lengkap yang menggabungkan WiFiPortal (koneksi), FastNTP (waktu), LCD I2C (tampilan waktu & kalender), Storage (menyimpan preferensi zona waktu), dan SmartVoice (suara lonceng/chime setiap pergantian jam).

### 08. `08_Framework_Kernel`
- **Fokus:** Arsitektur Perangkat Lunak Berskala Terpadu.
- **Platform:** ESP32 dan ESP8266 (atau board lain sesuai modul yang didaftarkan).
- **Penjelasan:** Menunjukkan arsitektur aplikasi berbasis Kernel (`IskakINO_Kernel`). Seluruh modul dibungkus adapter (`IskakINO_*Module`) dan didaftarkan ke objek `IskakINO`. Cukup satu panggilan `IskakINO.begin()` di `setup()` dan satu panggilan `IskakINO.update()` di `loop()` untuk mengontrol siklus hidup semua modul secara serentak.

### 09. `09_SmartSchoolBell`
- **Fokus:** Proyek Bel Sekolah Otomatis Cerdas Siap Pakai (*Production-Ready*).
- **Platform:** ESP32 (dan kompatibel ESP8266).
- **Penjelasan:** Aplikasi bel sekolah otomatis lengkap yang memadukan jadwal tersimpan di non-volatile storage, sinkronisasi waktu presisi FastNTP, pemutaran suara pengumuman/bel MP3 via DFPlayer Mini, LCD indicator status, dan Web Dashboard responsif ([BellWebPage.h](examples/09_SmartSchoolBell/BellWebPage.h)) untuk mengubah jadwal pelajaran secara nirkabel via HP/komputer. Disertai berkas template Excel jadwal ([Template_Jadwal_Bel_Sekolah.xlsx](examples/09_SmartSchoolBell/Template_Jadwal_Bel_Sekolah.xlsx)) dan panduan tersendiri di [examples/09_SmartSchoolBell/README.md](examples/09_SmartSchoolBell/README.md).

---

## 🏗️ Struktur Repositori

```text
IskakINO/
├── src/
│   ├── IskakINO.h                                  # Entry point utama library
│   ├── core/                                       # Shared core logic & framework
│   │   ├── IskakINO_Platform.h                     # Deteksi platform & FastPin<P>
│   │   ├── IskakINO_Result.h                       # Status enum error & hasil operasi
│   │   ├── IskakINO_Logger.h                       # Logging terpadu printf-style
│   │   ├── IskakINO_Scheduler.h                    # Scheduler non-blocking every()/once()
│   │   ├── IskakINO_Module.h                       # Interface modul begin()/update()
│   │   ├── IskakINO_Kernel.h                       # Kernel registry global (IskakINO)
│   │   └── IskakINO_Version.h                      # Sumber versi tunggal (ISKAKINO_VERSION)
│   ├── ardufast/                                   # Modul GPIO & Task Scheduler
│   ├── storage/                                    # Modul Storage hybrid (EEPROM/Prefs/LittleFS)
│   ├── lcd/                                        # Modul driver I2C LCD dengan animasi
│   ├── voice/                                      # Modul DFPlayer Mini MP3 Player
│   ├── wifi/                                       # Modul Captive Portal & Web Server
│   └── ntp/                                        # Modul Fast NTP Time Client
├── examples/                                       # 9 contoh sketch lengkap dan siap pakai
│   ├── 01_ArduFast_TaskManager/
│   │   └── 01_ArduFast_TaskManager.ino             # Contoh 01: FastPin & Task Scheduler
│   ├── 02_Storage_SaveLoad/
│   │   └── 02_Storage_SaveLoad.ino                 # Contoh 02: Simpan/Muat Struct & Log
│   ├── 03_LCD_TypewriterScroll/
│   │   └── 03_LCD_TypewriterScroll.ino             # Contoh 03: Animasi Teks & Progress Bar
│   ├── 04_SmartVoice_PlayTrack/
│   │   └── 04_SmartVoice_PlayTrack.ino             # Contoh 04: Kontrol MP3 & Playback Queue
│   ├── 05_WifiPortal_CaptivePortal/
│   │   └── 05_WifiPortal_CaptivePortal.ino         # Contoh 05: Captive Portal WiFi AP
│   ├── 06_FastNTP_ClockSync/
│   │   └── 06_FastNTP_ClockSync.ino                # Contoh 06: Sinkronisasi Waktu NTP
│   ├── 07_Unified_SmartClock/
│   │   └── 07_Unified_SmartClock.ino               # Contoh 07: Jam Pintar Terpadu (Manual)
│   ├── 08_Framework_Kernel/
│   │   └── 08_Framework_Kernel.ino                 # Contoh 08: Arsitektur Kernel IskakINO
│   └── 09_SmartSchoolBell/
│       ├── 09_SmartSchoolBell.ino                  # Contoh 09: Bel Sekolah Otomatis Cerdas
│       ├── BellWebPage.h                           # Web Dashboard HTML/JS responsif
│       ├── Template_Jadwal_Bel_Sekolah.xlsx        # Format spreadsheet template jadwal
│       └── README.md                               # Panduan lengkap proyek Bel Sekolah
├── .github/workflows/                              # CI/CD otomatis via Arduino CLI matrix
│   └── ci.yml
├── library.properties                              # Arduino Library Manager metadata
├── library.json                                    # PlatformIO Library Registry metadata
├── keywords.txt                                    # Syntax highlighting Arduino IDE
├── CHANGELOG.md                                    # Riwayat perubahan dan rilis
└── LICENSE                                         # Lisensi MIT
```

---

## ⚙️ Pengujian & CI/CD

Integrasi Berkelanjutan (*Continuous Integration*) berjalan otomatis di GitHub Actions menggunakan **[Arduino CLI](https://github.com/arduino/arduino-cli)** untuk memverifikasi kompilasi seluruh contoh sketch pada arsitektur target resmi:
- **Arduino AVR:** `arduino:avr:uno`
- **ESP8266:** `esp8266:esp8266:nodemcuv2`
- **ESP32:** `esp32:esp32:esp32`

Untuk menjalankan verifikasi kompilasi lokal via terminal:
```bash
# Uji kompilasi AVR
arduino-cli compile --fqbn arduino:avr:uno examples/01_ArduFast_TaskManager

# Uji kompilasi ESP8266
arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 examples/07_Unified_SmartClock

# Uji kompilasi ESP32
arduino-cli compile --fqbn esp32:esp32:esp32 examples/09_SmartSchoolBell
```

---

## 🔄 Migrasi dari Library Lama

Bagi pengguna library versi standalone terdahulu (`IskakINO_ArduFast`, `IskakINO_Storage`, `IskakINO_LiquidCrystal_I2C`, `IskakINO_WifiPortal`, `IskakINO_FastNTP`, `IskakINO_SmartVoice`):

1. Hapus instalasi library lama.
2. Pasang library **IskakINO**.
3. Ganti header spesifik lama menjadi `#include <IskakINO.h>`.
4. Seluruh nama class publik, fungsi, dan method **100% kompatibel** tanpa perlu mengubah logika kode Anda.

---

## 📄 Lisensi

Proyek ini dilisensikan di bawah lisensi **MIT** — lihat berkas [LICENSE](LICENSE) untuk detail lengkap.

## ✍️ Author & Maintainer

**Iskak Fatoni** ([@iskakfatoni](https://github.com/iskakfatoni))  
*Nisnas Computer — SMKN 1 Jetis Mojokerto*
