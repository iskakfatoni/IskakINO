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

**IskakINO** adalah ekosistem framework library Arduino terpadu (*Unified Library*) yang menggabungkan berbagai modul periferal, komunikasi, penyimpanan, dan antarmuka hardware dalam **satu library ringkas** dengan *shared core* berkinerja tinggi serta kernel modular opsional.

---

## 🌟 Keunggulan Utama

* **Single Entry Point:** Cukup satu baris `#include <IskakINO.h>` untuk mengakses seluruh kemampuan modul.
* **Shared Core Efisien:** Driver register direct I/O, logging terpadu, result codes, dan task scheduler *non-blocking* digunakan bersama tanpa redundansi memori.
* **Platform-Safe:** Modul universal bekerja di semua board (AVR, ESP8266, ESP32). Modul khusus jaringan (*WifiPortal* & *FastNTP*) otomatis *no-op* di board non-WiFi, dan modul hardware AVR (*BasicIOShield*) otomatis *no-op* di board non-AVR tanpa menimbulkan kesalahan kompilasi.
* **Zero Overhead:** Fitur dan modul yang tidak dipanggil di dalam sketsa tidak akan membebani konsumsi memori Flash/RAM mikrokontroler.

---

## 📦 Instalasi

### 1. Arduino IDE
1. Download atau *clone* repositori ini:
   ```bash
   git clone https://github.com/iskakfatoni/IskakINO.git
   ```
2. Salin folder `IskakINO` ke folder library Arduino Anda (`Documents/Arduino/libraries/`).
3. Restart Arduino IDE.

### 2. Arduino CLI
```bash
arduino-cli lib install --git-url https://github.com/iskakfatoni/IskakINO.git
```

### 3. PlatformIO
Tambahkan dependensi pada file `platformio.ini`:
```ini
lib_deps =
    https://github.com/iskakfatoni/IskakINO.git
```

---

## 🧩 Modul Ekosistem IskakINO

Dokumentasi detail, referensi API lengkap, dan panduan teknis tiap modul tersedia pada direktori [`readme/`](readme/):

| Modul | Class Utama | Platform Target | Dokumentasi Lengkap |
|---|---|---|---|
| **Core & Kernel** | `IskakINO_Kernel`, `FastPin<P>`, `IskakINO_Scheduler` | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_core.md`](readme/readme_core.md) |
| **ArduFast** | `IskakINO_ArduFast` | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_ardufast.md`](readme/readme_ardufast.md) |
| **Storage** | `IskakINO_Storage` (`IskakStorage`) | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_storage.md`](readme/readme_storage.md) |
| **LCD** | `LiquidCrystal_I2C` | Universal (I2C / Wire) | [📖 `readme_lcd.md`](readme/readme_lcd.md) |
| **OLED** | `IskakINO_OLED` | Universal (I2C / SSD1306 / SH1106) | [📖 `readme_oled.md`](readme/readme_oled.md) |
| **Button** | `IskakINO_Button` | Universal (GPIO / Gestures) | [📖 `readme_button.md`](readme/readme_button.md) |
| **Relay** | `IskakINO_Relay` | Universal (Actuator / Pulse) | [📖 `readme_relay.md`](readme/readme_relay.md) |
| **Filter** | `IskakINO_Kalman1D`, `IskakINO_MedianFilter` | Universal (DSP & Calibration) | [📖 `readme_filter.md`](readme/readme_filter.md) |
| **SmartVoice** | `IskakINO_SmartVoice` | Universal (Stream / Serial) | [📖 `readme_smartvoice.md`](readme/readme_smartvoice.md) |
| **Buzzer** | `IskakINO_Buzzer` | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_buzzer.md`](readme/readme_buzzer.md) |
| **BasicIOShield** | `IskakINO_BasicIOShield` (`BasicIOShield`) | **Khusus Arduino AVR** (Uno/Nano/Mega) | [📖 `readme_basicioshield.md`](readme/readme_basicioshield.md) |
| **WifiPortal** | `IskakINO_WifiPortal` | **ESP32 & ESP8266** | [📖 `readme_wifiportal.md`](readme/readme_wifiportal.md) |
| **FastNTP** | `IskakINO_FastNTP` | **ESP32 & ESP8266** | [📖 `readme_fastntp.md`](readme/readme_fastntp.md) |

> 💡 **Rencana Modul Baru & Roadmap:** Lihat berkas [ROADMAP.md](ROADMAP.md) untuk melihat daftar modul baru dan penyempurnaan fitur yang sedang direncanakan.

---

## 🚀 Panduan Penggunaan Singkat (Quick Start)

IskakINO mendukung dua paradigma pemrograman:

### 1. Pola Modular Manual (Kontrol Penuh)
```cpp
#include <IskakINO.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);

void setup() {
    fast.begin(115200);
    lcd.begin();
    lcd.typewriterStart("Halo IskakINO!", 0, 0, 80);
}

void loop() {
    lcd.update(); // Update animasi LCD non-blocking

    // Eksekusi task berkala setiap 1000 ms
    if (fast.every(1000, 0)) {
        fast.log("Sistem Aktif - Uptime: %lu ms", millis());
    }
}
```

### 2. Pola Framework Kernel (Otomatis & Terpadu)
```cpp
#include <IskakINO.h>

IskakINO_ArduFast fast;
LiquidCrystal_I2C lcd(16, 2);

IskakINO_ArduFastModule fastMod(fast, 115200);
IskakINO_LCDModule      lcdMod(lcd);

void setup() {
    IskakINO.registerModule(&fastMod);
    IskakINO.registerModule(&lcdMod);
    IskakINO.begin(); // Otomatis memanggil begin() seluruh modul
}

void loop() {
    IskakINO.update(); // Otomatis me-refresh update() seluruh modul
}
```

---

## 📂 Ringkasan Contoh Sketsa (`examples/`)

| No | Folder Contoh | Modul Utama | Platform | Deskripsi Ringkas |
|:---:|---|---|---|---|
| **01** | [`01_ArduFast_TaskManager`](examples/01_ArduFast_TaskManager/) | ArduFast, Core | Universal | Task scheduler non-blocking, manipulasi pin register, & filter ADC. |
| **02** | [`02_Storage_SaveLoad`](examples/02_Storage_SaveLoad/) | Storage | Universal | Simpan/muat struct, String dinamis, ring-buffer log, & enkripsi XOR. |
| **03** | [`03_LCD_TypewriterScroll`](examples/03_LCD_TypewriterScroll/) | LCD I2C | Universal | Efek mesin ketik, teks berjalan horizontal, & grafik progress bar. |
| **04** | [`04_SmartVoice_PlayTrack`](examples/04_SmartVoice_PlayTrack/) | SmartVoice | Universal | Pemutaran MP3 DFPlayer Mini, antrean suara, & feedback status. |
| **05** | [`05_WifiPortal_CaptivePortal`](examples/05_WifiPortal_CaptivePortal/) | WifiPortal | ESP32 / ESP8266 | Portal konfigurasi WiFi captive portal dengan parameter web kustom. |
| **06** | [`06_FastNTP_ClockSync`](examples/06_FastNTP_ClockSync/) | FastNTP | ESP32 / ESP8266 | Sinkronisasi jam internet NTP, format waktu instan, & kalender ID/EN. |
| **07** | [`07_Unified_SmartClock`](examples/07_Unified_SmartClock/) | Multi-Modul (Manual) | ESP32 / ESP8266 | Jam dinding pintar lengkap (WiFi + NTP + LCD + Storage + Voice). |
| **08** | [`08_Framework_Kernel`](examples/08_Framework_Kernel/) | Multi-Modul (Kernel) | ESP32 / ESP8266 | Arsitektur kernel terpusat menggunakan `IskakINO_Kernel`. |
| **09** | [`09_SmartSchoolBell`](examples/09_SmartSchoolBell/) | Proyek Produksi | ESP32 / ESP8266 | Bel sekolah otomatis berbasis NTP, jadwal storage, LCD, & Web Dashboard. |
| **10** | [`10_BasicIOShield_Overview`](examples/10_BasicIOShield_Overview/) | BasicIOShield, Core | **Khusus AVR** | Driver hardware EMS Basic I/O Shield (LED, Tombol, 7-Seg, & DAC). |
| **11** | [`11_Buzzer_MelodyBeep`](examples/11_Buzzer_MelodyBeep/) | Buzzer, Core | Universal | Driver buzzer non-blocking, nada status instan, melodi RTTTL, & menu interaktif. |
| **12** | [`12_OLED_GraphicsAnimation`](examples/12_OLED_GraphicsAnimation/) | OLED, Core | Universal | Driver OLED I2C ultra-hemat RAM, animasi teks, dashboard ikon IoT, & progress bar. |
| **13** | [`13_Button_MultiGesture`](examples/13_Button_MultiGesture/) | Button, Core | Universal | Deteksi tombol multi-gesture (Single, Double, Multi-Click, & Long Press). |
| **14** | [`14_Relay_PulseBlink`](examples/14_Relay_PulseBlink/) | Relay, Core | Universal | Kontrol relay pintar dengan auto-off pulse timer, blink cadence, & menu Serial. |
| **15** | [`15_Filter_SensorSmoothing`](examples/15_Filter_SensorSmoothing/) | Filter, Core | Universal | Komparasi filter sensor (Raw vs Moving Median vs 1D Kalman vs Linear Calibrator). |

---

## 🏗️ Struktur Repositori

```text
IskakINO/
├── src/
│   ├── IskakINO.h                                  # Entry point utama library
│   ├── core/                                       # Shared core logic & framework kernel
│   ├── ardufast/                                   # Modul GPIO & Task Scheduler
│   ├── storage/                                    # Modul Storage hybrid (EEPROM/Prefs/LittleFS)
│   ├── lcd/                                        # Modul driver I2C LCD dengan animasi
│   ├── oled/                                       # Modul driver I2C OLED (SSD1306/SH1106)
│   ├── button/                                     # Modul driver tombol multi-gesture
│   ├── relay/                                      # Modul driver relay & aktuator pintar
│   ├── filter/                                     # Modul filter sinyal & kalibrasi sensor
│   ├── voice/                                      # Modul DFPlayer Mini MP3 Player
│   ├── buzzer/                                     # Modul Driver Buzzer & RTTTL Melody Player
│   ├── shield/                                     # Modul driver EMS Basic I/O Shield (AVR)
│   ├── wifi/                                       # Modul Captive Portal & Web Server
│   └── ntp/                                        # Modul Fast NTP Time Client
├── readme/                                         # Dokumentasi teknis terpisah per-modul
│   ├── readme_core.md                              # Dokumentasi Core, FastPin, & Kernel
│   ├── readme_ardufast.md                          # Dokumentasi Modul ArduFast
│   ├── readme_storage.md                           # Dokumentasi Modul Storage
│   ├── readme_lcd.md                               # Dokumentasi Modul LCD I2C
│   ├── readme_oled.md                              # Dokumentasi Modul OLED I2C
│   ├── readme_button.md                            # Dokumentasi Modul Button
│   ├── readme_relay.md                             # Dokumentasi Modul Relay
│   ├── readme_filter.md                            # Dokumentasi Modul Filter
│   ├── readme_smartvoice.md                        # Dokumentasi Modul SmartVoice
│   ├── readme_buzzer.md                            # Dokumentasi Modul Buzzer & RTTTL
│   ├── readme_basicioshield.md                     # Dokumentasi Modul Basic I/O Shield
│   ├── readme_wifiportal.md                        # Dokumentasi Modul WifiPortal
│   └── readme_fastntp.md                           # Dokumentasi Modul FastNTP
├── examples/                                       # 15 contoh sketch lengkap dan siap pakai
├── .github/workflows/                              # CI/CD otomatis via Arduino CLI matrix
├── library.properties                              # Arduino Library Manager metadata
├── library.json                                    # PlatformIO Library Registry metadata
├── keywords.txt                                    # Syntax highlighting Arduino IDE
├── CHANGELOG.md                                    # Riwayat perubahan dan rilis
└── LICENSE                                         # Lisensi MIT
```

---

## ⚙️ Pengujian & CI/CD

Integrasi Berkelanjutan (*Continuous Integration*) berjalan otomatis di GitHub Actions menggunakan **[Arduino CLI](https://github.com/arduino/arduino-cli)** untuk memverifikasi kompilasi seluruh contoh sketch pada arsitektur target resmi:
* **Arduino AVR:** `arduino:avr:uno`
* **ESP8266:** `esp8266:esp8266:nodemcuv2`
* **ESP32:** `esp32:esp32:esp32`

---

## 📄 Lisensi

Proyek ini dilisensikan di bawah lisensi **MIT** — lihat berkas [LICENSE](LICENSE) untuk detail lengkap.

## ✍️ Author & Maintainer

**Iskak Fatoni** ([@iskakfatoni](https://github.com/iskakfatoni))  
*Supported by Nisnas Computer for SMKN 1 Jetis Mojokerto*
