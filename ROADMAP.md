# 🗺️ Roadmap & Rencana Pengembangan IskakINO

Dokumen ini memuat daftar rencana fitur baru, usulan modul tambahan, dan penyempurnaan arsitektur untuk pengembangan ekosistem **IskakINO** ke depan.

---

## 🎯 Skala Prioritas & Status Pengembangan

| Prioritas | Modul / Fitur | Platform Target | Deskripsi Singkat | Status |
|:---:|---|---|---|:---:|
| **P1** | **`IskakINO_RTC`** | Universal (AVR / ESP32 / ESP8266) | Driver RTC hardware (DS3231/DS1307/PCF8563) dengan sinkronisasi otomatis FastNTP (Hybrid Clock). | 📝 Terencana (Next P1) |
| **Release** | **`IskakINO_MQTT`** | ESP32 / ESP8266 | Client MQTT v3.1.1 zero-dependency terintegrasi siklus hidup Kernel dengan *auto-reconnect*. | ✅ Selesai (v1.3.0) |
| **Release** | **`IskakINO_Telegram`** | ESP32 / ESP8266 | Bot Telegram notifier & remote command handler via HTTPS REST. | ✅ Selesai (v1.3.0) |
| **Release** | **Multi-SSID Fallback** | ESP32 / ESP8266 | Manajemen multi-profil WiFi cadangan, Web UI delete/add profil, & runtime auto-failover pada `IskakINO_WifiPortal`. | ✅ Selesai (v1.2.0) |
| **Release** | **`IskakINO_Buzzer`** | Universal | Generator nada, status beeping, chime, dan melodi RTTTL non-blocking. | ✅ Selesai (v1.1.0) |
| **Release** | **`IskakINO_OLED`** | Universal | Driver display SSD1306/SH1106 I2C ultra-hemat memori (< 40B RAM) & animasi. | ✅ Selesai (v1.1.0) |
| **Release** | **`IskakINO_Button`** | Universal | Deteksi *gesture* tombol non-blocking (*Single*, *Double*, *Long Press*, *Hold*). | ✅ Selesai (v1.1.0) |
| **Release** | **`IskakINO_Relay`** | Universal | Driver relay pintar dengan auto-off pulse timer dan kontrol keamanan state. | ✅ Selesai (v1.1.0) |
| **Release** | **`IskakINO_Filter`** | Universal | Sinyal filter lanjutan: 1D Kalman Filter, Moving Median, EMA, & Linear Calibrator. | ✅ Selesai (v1.1.0) |
| **Release** | **`IskakINO_Cam`** | ESP32 | Driver modul kamera ESP32 (ESP32-CAM, OV2640, PSRAM, Flash LED). | ✅ Selesai (v1.1.0) |

---

## 📦 Rincian Rencana Modul Berikutnya

### 1. `IskakINO_RTC` (Hardware Real-Time Clock)
* **Target Hardware:** DS3231 (TCXO High Precision), DS1307, PCF8563 (I2C interface).
* **Fitur Utama:**
  * Pembacaan dan pengaturan tanggal/jam format terpadu.
  * **Sinergi Hybrid NTP:** Saat terhubung ke internet, `FastNTP` otomatis menyinkronkan jam fisik RTC. Ketika internet mati, sistem otomatis beralih membaca RTC tanpa jeda.
  * Dukungan kompensasi suhu dan alarm interrupt harian.

### 2. `IskakINO_MQTT` & `IskakINO_Telegram` (IoT Communication)
* **Fitur Utama:**
  * MQTT client dengan reconnect loop otomatis non-blocking.
  * Integrasi ringkas untuk publish sensor dan subscribe perintah kendali jarak jauh.
  * Pengiriman pesan bot Telegram untuk notifikasi kejadian penting (alarm, status jadwal, restart).

---

## ⚡ Rencana Penyempurnaan Modul Eksisting

### `IskakINO_WifiPortal`
1. **Multi-SSID Profiles:** Kemampuan menyimpan 2–3 daftar WiFi (misalnya WiFi Kantor dan Hotspot HP cadangan).
2. **Live Web Log Viewer (WebSocket / SSE):** Menampilkan output log sistem secara live di browser Web Dashboard tanpa memerlukan koneksi kabel USB Serial Monitor.
3. **Penyempurnaan Tampilan UI:** Dukungan mode tema gelap (*Dark Mode*) bawaan yang lebih ringan.

### `IskakINO_LiquidCrystal_I2C`
1. **Custom Icon Generator:** Helper bawaan untuk membuat ikon status baterai (0–100%), level sinyal WiFi (0–4 bar / RSSI dBm), termometer (level 0–3), dan panah tanpa manipulasi byte manual. (✅ Selesai)
2. **Dynamic Banner:** Transisi multi-halaman layar otomatis dengan jeda waktu teratur (*page flipper*), mendukung daftar teks statis maupun callback dinamis. (✅ Selesai)

### `09_SmartSchoolBell` (Production Project)
1. **Dukungan RTC Offline:** Opsi otomatis membaca DS3231 saat WiFi tidak tersedia.
2. **Fitur Ekspor/Impor Jadwal JSON:** Memungkinkan backup jadwal dari Web Dashboard ke file JSON lokal di HP/komputer dan sebaliknya.

---

## 🤝 Kontribusi & Saran
Bagi komunitas dan pengembang yang ingin mengajukan ide modul baru atau membantu implementasi, silakan buka [Issue](https://github.com/iskakfatoni/IskakINO/issues) atau ajukan [Pull Request](https://github.com/iskakfatoni/IskakINO/pulls).
