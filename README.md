# IskakINO Framework

[![Arduino Library](https://img.shields.io/badge/Arduino-Framework-00979D.svg?logo=arduino&logoColor=white)](https://www.arduino.cc/reference/en/libraries/)
[![Platform](https://img.shields.io/badge/platform-AVR%20%7C%20ESP8266%20%7C%20ESP32-green.svg)](#)
[![Precompiled](https://img.shields.io/badge/precompiled-full-blue.svg)](#)
[![License](https://img.shields.io/badge/license-MIT-lightgrey.svg)](LICENSE)
[![GitHub release](https://img.shields.io/github/v/release/iskakfatoni/IskakINO?color=blue&logo=github)](https://github.com/iskakfatoni/IskakINO/releases)
[![CI](https://github.com/iskakfatoni/IskakINO-Dev/actions/workflows/ci.yml/badge.svg)](https://github.com/iskakfatoni/IskakINO-Dev/actions)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-11%20%2F%2017-00599C.svg?logo=cplusplus&logoColor=white)](https://en.cppreference.com/)
[![PlatformIO Registry](https://img.shields.io/badge/PlatformIO-Compatible-orange.svg?logo=platformio&logoColor=white)](library.json)
[![Examples](https://img.shields.io/badge/examples-28%20sketches-success.svg)](#)
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
| **Core & Kernel** | `IskakINO_Kernel`, `FastPin<P>`, `Scheduler`, `Diagnostic`, `Timer`, `RingBuffer`, `BitUtils` | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_core.md`](readme/readme_core.md) |
| **ArduFast** | `IskakINO_ArduFast` | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_ardufast.md`](readme/readme_ardufast.md) |
| **Storage** | `IskakINO_Storage` (`IskakStorage`) | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_storage.md`](readme/readme_storage.md) |
| **LCD** | `LiquidCrystal_I2C` | Universal (I2C / Wire) | [📖 `readme_lcd.md`](readme/readme_lcd.md) |
| **OLED** | `IskakINO_OLED` | Universal (I2C / SSD1306 / SH1106) | [📖 `readme_oled.md`](readme/readme_oled.md) |
| **Button** | `IskakINO_Button` | Universal (GPIO / Gestures) | [📖 `readme_button.md`](readme/readme_button.md) |
| **Relay** | `IskakINO_Relay` | Universal (Actuator / Pulse) | [📖 `readme_relay.md`](readme/readme_relay.md) |
| **Filter** | `IskakINO_Kalman1D`, `IskakINO_MedianFilter` | Universal (DSP & Calibration) | [📖 `readme_filter.md`](readme/readme_filter.md) |
| **RTC** | `IskakINO_RTC` | Universal (DS3231 / DS1307 / PCF8563) | [📖 `readme_rtc.md`](readme/readme_rtc.md) |
| **Sensors** | `IskakINO_DHT`, `IskakINO_DS18B20`, `IskakINO_Ultrasonic` | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_sensors.md`](readme/readme_sensors.md) |
| **JSON** | `IskakJSONBuilder`, `IskakJSONReader` | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_json.md`](readme/readme_json.md) |
| **SmartVoice** | `IskakINO_SmartVoice` | Universal (Stream / Serial) | [📖 `readme_smartvoice.md`](readme/readme_smartvoice.md) |
| **Buzzer** | `IskakINO_Buzzer` | Universal (AVR / ESP32 / ESP8266) | [📖 `readme_buzzer.md`](readme/readme_buzzer.md) |
| **BasicIOShield** | `IskakINO_BasicIOShield` (`BasicIOShield`) | **Khusus Arduino AVR** (Uno/Nano/Mega) | [📖 `readme_basicioshield.md`](readme/readme_basicioshield.md) |
| **Cam** | `IskakINO_Cam` | **Khusus ESP32** (ESP32-CAM / OV2640) | [📖 `readme_cam.md`](readme/readme_cam.md) |
| **BLE** | `IskakINO_BLE` | **Khusus ESP32** (Nordic UART Service) | [📖 `readme_ble.md`](readme/readme_ble.md) |
| **WifiPortal** | `IskakINO_WifiPortal` | **ESP32 & ESP8266** | [📖 `readme_wifiportal.md`](readme/readme_wifiportal.md) |
| **FastNTP** | `IskakINO_FastNTP` | **ESP32 & ESP8266** | [📖 `readme_fastntp.md`](readme/readme_fastntp.md) |
| **MQTT** | `IskakINO_MQTT` | **ESP32 & ESP8266** (v3.1.1 Client) | [📖 `readme_mqtt.md`](readme/readme_mqtt.md) |
| **TaskCore** | `IskakINO_TaskCore`, `IskakINO_Queue`, `IskakINO_Mutex` | **Khusus ESP32** (Dual-Core FreeRTOS) | [📖 `readme_taskcore.md`](readme/readme_taskcore.md) |
| **WebSockets** | `IskakINO_WebSocketsServer`, `IskakINO_WebSocketsClient` | **ESP32 & ESP8266** (RFC 6455 Stream) | [📖 `readme_websockets.md`](readme/readme_websockets.md) |
| **PrayerTimes** | `IskakINO_PrayerTimes`, `IskakINO_Hijri` | Universal (Jadwal Sholat & Hijriah) | [📖 `readme_prayertimes.md`](readme/readme_prayertimes.md) |

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

| No | Folder Contoh | Modul Terkait | Platform | Deskripsi Ringkas |
|:---:|---|---|---|---|
| **01** | [`01_Core_Basics`](examples/01_Core_Basics/) | Core, ArduFast, Storage, JSON | Universal | Fondasi utama: Fast GPIO, Task Scheduler, Diagnostic RAM/Heap, Timer, RingBuffer, BitUtils/CRC, Storage struct, & JSON zero-heap. |
| **02** | [`02_Basic_IO_Actuators`](examples/02_Basic_IO_Actuators/) | Button, Relay, Buzzer, Shield | Universal | Interaksi tombol multi-gesture (single/double/long), relay pintar (toggle/pulse/blink), audio buzzer non-blocking, & shield AVR. |
| **03** | [`03_Displays_Showcase`](examples/03_Displays_Showcase/) | LCD, OLED | Universal | Driver display teks & grafis I2C: typewriter, marquee scroll horizontal, dashboard IoT icon, & animated progress bar. |
| **04** | [`04_Sensors_and_Filtering`](examples/04_Sensors_and_Filtering/) | Sensors, Filter | Universal | Pembacaan DHT11/22, DS18B20 1-Wire, HC-SR04 sonar + sinergi Kalman 1D, Median, EMA, & Linear Calibrator. |
| **05** | [`05_Audio_Voice`](examples/05_Audio_Voice/) | SmartVoice | Universal | Pemutaran MP3 DFPlayer Mini / JQ6500, deteksi status MicroSD, kontrol volume, EQ preset, & navigasi lagu. |
| **06** | [`06_Timekeeping_and_RTC`](examples/06_Timekeeping_and_RTC/) | RTC, FastNTP, PrayerTimes | Universal / WiFi | Manajemen waktu terpadu: Hardware RTC (DS3231/DS1307), FastNTP auto-sync, & hisab jadwal sholat Kemenag. |
| **07** | [`07_WiFi_IoT_Suite`](examples/07_WiFi_IoT_Suite/) | WifiPortal, WebSockets, MQTT, Telegram | ESP32 / ESP8266 | Ekosistem IoT lengkap: Captive Portal AP, WebSocket streaming web dashboard, MQTT telemetry, & bot Telegram. |
| **08** | [`08_ESP32_Advanced`](examples/08_ESP32_Advanced/) | TaskCore, BLE, Cam | **Khusus ESP32** | FreeRTOS Dual-Core background task, Bluetooth Low Energy (NUS UART), & modul kamera OV2640 snapshot. |
| **09** | [`09_SmartSchoolBell`](examples/09_SmartSchoolBell/) | Proyek Produksi (Bel Sekolah) | ESP32 / ESP8266 | *Turnkey Project:* Bel sekolah otomatis lengkap dengan Web Management, MP3 voice, & template Excel. |
| **10** | [`10_SmartMosqueClock`](examples/10_SmartMosqueClock/) | Proyek Produksi (Jam Masjid JWS) | Universal | *Turnkey Project:* Jam Masjid Digital dengan hisab Kemenag, kalender Hijriah, arah kiblat, & countdown adzan/iqomah. |

---

## 🏗️ Struktur Repositori

```text
IskakINO/
├── src/
│   ├── IskakINO.h                                  # Entry point utama library
│   ├── core/                                       # Shared core logic, taskcore, & kernel
│   ├── ardufast/                                   # Modul GPIO & Task Scheduler
│   ├── storage/                                    # Modul Storage hybrid (EEPROM/Prefs/LittleFS)
│   ├── lcd/                                        # Modul driver I2C LCD dengan animasi
│   ├── oled/                                       # Modul driver I2C OLED (SSD1306/SH1106)
│   ├── button/                                     # Modul driver tombol multi-gesture
│   ├── relay/                                      # Modul driver relay & aktuator pintar
│   ├── filter/                                     # Modul filter sinyal & kalibrasi sensor
│   ├── rtc/                                        # Modul driver hardware RTC (DS3231/DS1307/PCF8563)
│   ├── sensors/                                    # Modul driver sensor DHT, DS18B20, & Ultrasonic
│   ├── json/                                       # Modul JSON Builder & Zero-Copy Tokenizer Parser
│   ├── cam/                                        # Modul driver kamera ESP32 (ESP32-CAM)
│   ├── ble/                                        # Modul Bluetooth Low Energy (NUS UART Bridge)
│   ├── voice/                                      # Modul DFPlayer Mini MP3 Player
│   ├── buzzer/                                     # Modul Driver Buzzer & RTTTL Melody Player
│   ├── shield/                                     # Modul driver EMS Basic I/O Shield (AVR)
│   ├── wifi/                                       # Modul Captive Portal & Web Server
│   ├── ntp/                                        # Modul Fast NTP Time Client
│   ├── mqtt/                                       # Modul MQTT v3.1.1 Client
│   ├── telegram/                                   # Modul Bot Notifier Telegram HTTPS REST
│   ├── websockets/                                 # Modul WebSockets RFC 6455 Server & Client
│   └── prayertimes/                                # Modul Jadwal Sholat, Hijriah, & Arah Kiblat
├── readme/                                         # Dokumentasi teknis terpisah per-modul
│   ├── readme_core.md                              # Dokumentasi Core, FastPin, & Kernel
│   ├── readme_taskcore.md                          # Dokumentasi Dual-Core FreeRTOS Task Manager
│   ├── readme_ardufast.md                          # Dokumentasi Modul ArduFast
│   ├── readme_storage.md                           # Dokumentasi Modul Storage
│   ├── readme_lcd.md                               # Dokumentasi Modul LCD I2C
│   ├── readme_oled.md                              # Dokumentasi Modul OLED I2C
│   ├── readme_button.md                            # Dokumentasi Modul Button
│   ├── readme_relay.md                             # Dokumentasi Modul Relay
│   ├── readme_filter.md                            # Dokumentasi Modul Filter
│   ├── readme_rtc.md                               # Dokumentasi Modul RTC & Hybrid NTP
│   ├── readme_sensors.md                           # Dokumentasi Modul Sensors Suite
│   ├── readme_json.md                              # Dokumentasi Modul JSON Builder & Parser
│   ├── readme_cam.md                               # Dokumentasi Modul Cam ESP32
│   ├── readme_ble.md                               # Dokumentasi Modul BLE NUS UART
│   ├── readme_smartvoice.md                        # Dokumentasi Modul SmartVoice
│   ├── readme_buzzer.md                            # Dokumentasi Modul Buzzer & RTTTL
│   ├── readme_basicioshield.md                     # Dokumentasi Modul Basic I/O Shield
│   ├── readme_wifiportal.md                        # Dokumentasi Modul WifiPortal
│   ├── readme_fastntp.md                           # Dokumentasi Modul FastNTP
│   ├── readme_mqtt.md                              # Dokumentasi Modul MQTT v3.1.1
│   ├── readme_telegram.md                          # Dokumentasi Modul Telegram Bot
│   ├── readme_websockets.md                        # Dokumentasi Modul WebSockets
│   └── readme_prayertimes.md                       # Dokumentasi Modul PrayerTimes
├── examples/                                       # 10 contoh sketch terpadu (Suite) siap pakai
├── .github/workflows/                              # CI/CD otomatis via Arduino CLI matrix (dengan build caching)
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
