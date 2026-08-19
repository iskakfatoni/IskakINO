# 🗺️ Roadmap & Rencana Pengembangan IskakINO

Dokumen ini memuat daftar rencana fitur baru, usulan modul tambahan, dan penyempurnaan arsitektur untuk pengembangan ekosistem **IskakINO** ke depan.

---

## 🎯 Skala Prioritas Pengembangan

| Prioritas | Modul / Fitur | Platform Target | Deskripsi Singkat | Status |
|:---:|---|---|---|:---:|
| **P1** | **`IskakINO_RTC`** | Universal (AVR / ESP32 / ESP8266) | Driver RTC hardware (DS3231/DS1307/PCF8563) dengan sinkronisasi otomatis FastNTP (Hybrid Clock). | 📝 Terencana |
| **P1** | **`IskakINO_Button`** | Universal | Deteksi *gesture* tombol non-blocking (*Single*, *Double*, *Long Press*, *Hold*). | 📝 Terencana |
| **P2** | **`IskakINO_Buzzer`** | Universal | Generator nada, status beeping, chime, dan melodi RTTTL non-blocking tanpa `delay()`. | ✅ Selesai (v1.1.0) |
| **P2** | **`IskakINO_Relay`** | Universal | Driver relay pintar dengan auto-off pulse timer dan kontrol keamanan state. | 📝 Terencana |
| **P2** | **Multi-SSID Fallback** | ESP32 / ESP8266 | Penyimpanan multi-profil WiFi cadangan pada `IskakINO_WifiPortal`. | 📝 Terencana |
| **P3** | **`IskakINO_Filter`** | Universal | Sinyal filter lanjutan: 1D Kalman Filter, Moving Median, dan kalibrasi multi-titik linear. | 💡 Ide |
| **P3** | **`IskakINO_MQTT`** | ESP32 / ESP8266 | Client MQTT ringan terintegrasi siklus hidup Kernel dengan *auto-reconnect*. | 💡 Ide |
| **P3** | **`IskakINO_Telegram`** | ESP32 / ESP8266 | Notifier bot Telegram sederhana untuk alert sistem / bel sekolah. | 💡 Ide |
| **P3** | **`IskakINO_OLED`** | Universal | Driver display SSD1306/SH1106 I2C ultra-hemat memori dengan animasi teks. | 💡 Ide |

---

## 📦 Rincian Rencana Modul Baru

### 1. `IskakINO_RTC` (Hardware Real-Time Clock)
* **Target Hardware:** DS3231 (TCXO High Precision), DS1307, PCF8563 (I2C interface).
* **Fitur Utama:**
  * Pembacaan dan pengaturan tanggal/jam format terpadu.
  * **Sinergi Hybrid NTP:** Saat terhubung ke internet, `FastNTP` otomatis menyinkronkan jam fisik RTC. Ketika internet mati, sistem otomatis beralih membaca RTC tanpa jeda.
  * Dukungan kompensasi suhu dan alarm interrupt harian.

### 2. `IskakINO_Button` (Advanced Gesture Button)
* **Fitur Utama:**
  * Kontrol multi-aksi hanya dengan satu pin tombol fisik (Active LOW / PULLUP).
  * Callback event: `onClick()`, `onDoubleClick()`, `onLongPressStart()`, `onLongPressEnd()`, `duringLongPress()`.
  * Sepenuhnya non-blocking berbasis state-machine internal.

### 3. `IskakINO_Buzzer` (Non-Blocking Chime & Melody)
* **Fitur Utama:**
  * Nada status instan: `beep()`, `successTone()`, `errorTone()`, `warningTone()`.
  * Pemutaran not melodi / string RTTTL ringtone secara asinkron.
  * Sangat cocok sebagai alternatif audio murah tanpa memerlukan DFPlayer Mini.

### 4. `IskakINO_Relay` (Smart Actuator Manager)
* **Fitur Utama:**
  * Proteksi active-level (Active HIGH / Active LOW terkonfigurasi).
  * Fungsi pulsa otomatis non-blocking: `pulse(3000)` (menyalakan relay selama 3 detik lalu mati otomatis).
  * Toggle, blink, dan proteksi batasan frekuensi switching untuk mencegah kerusakan mekanis relay.

### 5. `IskakINO_Filter` (Advanced Signal Processing)
* **Fitur Utama:**
  * **1D Kalman Filter:** Menstabilkan pembacaan sensor berisik seperti sensor jarak ultrasonik HC-SR04, ToF VL53L0X, dan load cell.
  * **Moving Median Filter:** Mengeliminasi *spike* data ekstrem.
  * **Multi-Point Linear Regression:** Kalibrasi sensor analog multi-titik (misal sensor pH, TDS, atau suhu NTC).

### 6. `IskakINO_MQTT` & `IskakINO_Telegram` (IoT Communication)
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
1. **Custom Icon Generator:** Helper bawaan untuk membuat ikon status baterai, level sinyal WiFi (0–4 bar), termometer, dan panah tanpa manipulasi byte manual.
2. **Dynamic Banner:** Transisi multi-halaman layar otomatis dengan jeda waktu teratur (*page flipper*).

### `09_SmartSchoolBell` (Production Project)
1. **Dukungan RTC Offline:** Opsi otomatis membaca DS3231 saat WiFi tidak tersedia.
2. **Fitur Ekspor/Impor Jadwal JSON:** Memungkinkan backup jadwal dari Web Dashboard ke file JSON lokal di HP/komputer dan sebaliknya.

---

## 🤝 Kontribusi & Saran
Bagi komunitas dan pengembang yang ingin mengajukan ide modul baru atau membantu implementasi, silakan buka [Issue](https://github.com/iskakfatoni/IskakINO/issues) atau ajukan [Pull Request](https://github.com/iskakfatoni/IskakINO/pulls).
