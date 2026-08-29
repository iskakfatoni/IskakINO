# 🚀 Release Notes — IskakINO v1.1.0

**Release Tag:** `1.1.0`  
**Nama Rilis:** Ekspansi Modul IoT, Periferal, Sensor Suite, BLE, & Zero-Dependency JSON  
**Tanggal Rilis:** 29 Agustus 2026  
**Author:** Iskak Fatoni ([@iskakfatoni](https://github.com/iskakfatoni))

---

## 🌟 Sorotan Utama Rilis v1.1.0

Versi **1.1.0** merupakan pembaruan besar (*Minor Release*) pada ekosistem **IskakINO**. Pembaruan ini memperluas pustaka dari yang sebelumnya berfokus pada utilitas dasar menjadi ekosistem *embedded framework* yang lengkap untuk kebutuhan IoT industri, sistem cerdas, dan proyek mikrokontroler mandiri tanpa dependensi pihak ketiga.

---

## 📦 Modul Baru (Total 20 Modul)

### 1. 🌐 `IskakINO_JSON` (JSON Builder & Parser) [BARU]
* **Arsitektur Zero-Dependency & Zero-Copy Tokenizer:** Tidak memerlukan library eksternal (seperti `ArduinoJson`).
* **Ultra-Hemat Memori:** RAM footprint < 48 Bytes dan Flash < 2.5 KB.
* **Kompatibilitas Penuh:** Aman digunakan di **Arduino AVR Uno (RAM 2KB)**, **ESP8266**, dan **ESP32** tanpa risiko *heap fragmentation*.
* **Stream Builder (`IskakJSONBuilder`):** Menyusun payload JSON objek dan array bertingkat secara fluid, bisa langsung di-*stream* ke `Serial` / `WiFiClient`.
* **Type-Safe Extractor (`IskakJSONReader`):** Ekstraksi nilai string, int, float, bool, sub-objek, dan iterasi array `forEach`.
* **Contoh Sketsa:** [`examples/25_JSON_BuildAndParse/25_JSON_BuildAndParse.ino`](examples/25_JSON_BuildAndParse/25_JSON_BuildAndParse.ino)

### 2. 📡 `IskakINO_MQTT` (MQTT v3.1.1 Client) [BARU]
* Client protokol MQTT native di atas `WiFiClient` tanpa library eksternal.
* Mendukung Publish, Subscribe dengan callback spesifik topik, Last Will & Testament (LWT), keepalive ping (15 detik), dan *auto-reconnect* non-blocking.
* **Contoh Sketsa:** [`examples/17_MQTT_Telemetry/17_MQTT_Telemetry.ino`](examples/17_MQTT_Telemetry/17_MQTT_Telemetry.ino)

### 3. 🤖 `IskakINO_Telegram` (Telegram Bot Notifier & Command) [BARU]
* Notifikasi instan dan kendali interaktif via bot Telegram HTTPS REST API.
* Dilengkapi helper `sendMessage()`, `sendAlert()`, pendaftaran command handler (`onCommand("/relay", cb)`), dan polling asinkron non-blocking (`enablePolling()`).
* **Contoh Sketsa:** [`examples/18_Telegram_AlertBot/18_Telegram_AlertBot.ino`](examples/18_Telegram_AlertBot/18_Telegram_AlertBot.ino)

### 4. 🕒 `IskakINO_RTC` (Driver RTC Multi-Chip & Hybrid Clock) [BARU]
* Auto-deteksi chip RTC I2C: **DS3231**, **DS1307**, atau **PCF8563**.
* Formatter waktu `IskakDateTime` (Epoch UNIX, kalender Bahasa Indonesia/Inggris, nama hari & bulan).
* **Sinergi Hybrid NTP:** Otomatis menyinkronkan jam fisik RTC dari internet via `syncWithNTP(ntp)` saat online dan beralih membaca jam dari RTC saat offline.
* **Contoh Sketsa:** [`examples/19_RTC_ClockCalendar/19_RTC_ClockCalendar.ino`](examples/19_RTC_ClockCalendar/19_RTC_ClockCalendar.ino), [`examples/20_RTC_HybridNTP/20_RTC_HybridNTP.ino`](examples/20_RTC_HybridNTP/20_RTC_HybridNTP.ino)

### 5. 🌡️ `IskakINO_Sensors` (Suite Sensor Terpadu) [BARU]
* **`IskakINO_DHT`:** Driver DHT11, DHT22, AM2302 dengan pembacaan suhu, kelembapan, Heat Index, dan proteksi checksum 8-bit.
* **`IskakINO_DS18B20`:** Driver termometer 1-Wire presisi tinggi 12-bit (0.0625°C) dengan validasi CRC-8 internal.
* **`IskakINO_Ultrasonic`:** Pengukuran jarak akustik HC-SR04 dengan Moving Median Filter bawaan (5-sample) anti-noise.
* **Contoh Sketsa:** [`examples/21_Sensors_DHT_Environment`](examples/21_Sensors_DHT_Environment/), [`examples/22_Sensors_DS18B20_1Wire`](examples/22_Sensors_DS18B20_1Wire/), [`examples/23_Sensors_Ultrasonic_Filtered`](examples/23_Sensors_Ultrasonic_Filtered/)

### 6. 📶 `IskakINO_BLE` (Bluetooth Low Energy NUS Bridge) [BARU]
* Komunikasi nirkabel dua arah berbasis standar Nordic UART Service (NUS) untuk ESP32.
* Telemetri nirkabel (`send()`, `sendf()`), command dispatcher, dan *auto re-advertising*.
* **Contoh Sketsa:** [`examples/24_BLE_SerialTerminal/24_BLE_SerialTerminal.ino`](examples/24_BLE_SerialTerminal/24_BLE_SerialTerminal.ino)

### 7. 🔔 `IskakINO_Buzzer`, 🖥️ `IskakINO_OLED`, 🔘 `IskakINO_Button`, ⚡ `IskakINO_Relay`, 📈 `IskakINO_Filter`, & 📷 `IskakINO_Cam` [BARU]
* Driver buzzer nada status & RTTTL melody player non-blocking.
* Driver OLED I2C SSD1306/SH1106 ultra-hemat RAM (< 40B RAM).
* Driver tombol multi-gesture (*single*, *double*, *long press*, *hold*).
* Driver relay pintar dengan auto-off pulse & blink cadence.
* Filter sinyal lanjutan: 1D Kalman Filter, Moving Median, EMA, & Linear Calibrator.
* Driver kamera ESP32 (ESP32-CAM, OV2640, Flash LED).

---

## ⚡ Penyempurnaan Fitur Modul Eksisting

### 1. `IskakINO_WifiPortal`
* **Multi-SSID Profiles:** Menyimpan beberapa kredensial WiFi cadangan dengan *auto-failover* ke sinyal RSSI terkuat.
* **Live Web Log Viewer:** Endpoint REST `/api/logs` dan konsol interaktif di browser untuk memantau log sistem secara realtime tanpa kabel Serial USB.
* **Tema Dark Mode Bawaan:** Desain antarmuka modern berbasis CSS Variables dengan dukungan otomatis preferensi OS dan tombol manual toggle `🌓 Tema`.

### 2. Proyek Produksi `09_SmartSchoolBell`
* **Dukungan RTC Offline:** Terintegrasi dengan modul `IskakINO_RTC` (DS3231) untuk fallback jam offline saat WiFi sekolah padam.
* **Ekspor & Impor Jadwal Format JSON:** Endpoint `/bell/export_json` dan `/bell/import_json` serta tombol backup/restore di Web Dashboard.

---

## 📂 Ringkasan Sketsa Contoh (Total 25 Contoh)
Repositori kini menyediakan **25 contoh sketsa lengkap** di dalam direktori `examples/` yang mencakup implementasi mandiri hingga proyek skala produksi.

---

## 🛠️ Panduan Instalasi / Update

### Arduino IDE
1. Buka **Sketch** > **Include Library** > **Manage Libraries...**
2. Cari `IskakINO`
3. Pilih versi `1.1.0` lalu klik **Update / Install**.

### PlatformIO (`platformio.ini`)
```ini
lib_deps =
    iskakfatoni/IskakINO @ ^1.1.0
```

---

## 📄 Kompatibilitas Matrix CI/CD
Telah lulus uji verifikasi kompilasi otomatis melalui GitHub Actions untuk:
* ✅ **Arduino AVR:** `arduino:avr:uno`
* ✅ **ESP8266:** `esp8266:esp8266:nodemcuv2`
* ✅ **ESP32:** `esp32:esp32:esp32`
