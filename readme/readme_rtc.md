# 🕒 Modul: IskakINO_RTC

Driver hardware **Real-Time Clock (RTC)** universal berarsitektur *zero-dependency* (murni protokol I2C via `Wire.h`) yang kompatibel di semua platform (**AVR**, **ESP32**, **ESP8266**).

Dilengkapi fitur deteksi otomatis tipe chip (**DS3231**, **DS1307**, **PCF8563**), pembacaan sensor suhu presisi on-chip (DS3231), lokalisasi nama hari & bulan Bahasa Indonesia/Inggris, serta kemampuan sinkronisasi waktu hybrid dengan **`IskakINO_FastNTP`**.

---

## 🛠️ Fitur Utama

1. **Multi-Chip Auto-Detection:**
   * Otomatis mendeteksi chip RTC yang terhubung pada bus I2C (DS3231 dengan TCXO presisi tinggi ±2ppm, DS1307 standar, atau PCF8563 *low-power*).
2. **Struktur Waktu & Formatter `IskakDateTime`:**
   * Konversi dua arah UNIX epoch (`toEpoch()` / `fromEpoch()`).
   * Formatter tanggal dan waktu instan (`getTimeString()`, `getDateString()`, `format("%Y-%m-%d %H:%M:%S")`).
   * Algoritma Day-of-Week Tomohiko Sakamoto bawaan dengan lokalisasi nama hari dan bulan (ID/EN).
3. **Sensor Suhu Presisi (DS3231):**
   * Pembacaan suhu internal kristal dengan resolusi 0.25°C via `getTemperature()`.
4. **Sinergi Hybrid NTP (Khusus ESP32 & ESP8266):**
   * Sinkronisasi berkala dari `IskakINO_FastNTP` ke register fisik RTC via `syncWithNTP(ntp, intervalMs)`. Saat internet online, RTC otomatis disinkronkan; saat offline/tanpa sinyal, sistem membaca waktu dari RTC fisik secara mulus tanpa jeda.

---

## 🔌 Diagram Koneksi Pin Hardware

```
[Mikrokontroler (AVR / ESP)]             [Modul RTC (DS3231 / DS1307 / PCF8563)]
VCC (3.3V / 5V) ----------------------- VCC
GND ----------------------------------- GND
SCL (A5 pada Uno / GPIO22 pada ESP32) - SCL
SDA (A4 pada Uno / GPIO21 pada ESP32) - SDA
```

---

## 💻 Contoh Penggunaan Singkat

### 1. Membaca Waktu & Kalender (Universal)
```cpp
#include <IskakINO.h>

IskakINO_RTC rtc;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    if (!rtc.begin()) {
        Serial.println("RTC tidak terdeteksi!");
        return;
    }

    Serial.print("Chip Terdeteksi: ");
    Serial.println(rtc.chipName());

    if (rtc.lostPower()) {
        Serial.println("Baterai RTC habis/baru dipasang, menyetel waktu default...");
        rtc.setDateTime(2026, 8, 29, 7, 30, 0); // YYYY, MM, DD, HH, MM, SS
    }
}

void loop() {
    IskakDateTime dt = rtc.now();

    // Format: "Sabtu, 29 Agustus 2026"
    Serial.println(dt.getDateString(true));
    // Format: "07:30:00"
    Serial.println(dt.getTimeString(true));

    if (rtc.chipType() == IskakRTCType::DS3231) {
        Serial.print("Suhu RTC: ");
        Serial.print(rtc.getTemperature());
        Serial.println(" °C");
    }

    delay(1000);
}
```

### 2. Sinergi Hybrid NTP (ESP32 / ESP8266)
```cpp
#include <IskakINO.h>

IskakINO_FastNTP ntp;
IskakINO_RTC     rtc;

void setup() {
    Serial.begin(115200);
    WiFi.begin("SSID_WIFI", "PASSWORD_WIFI");

    rtc.begin();
    ntp.begin(7); // UTC+7 WIB

    // Otomatis sinkronkan RTC dari FastNTP setiap 1 jam jika terhubung ke internet
    rtc.syncWithNTP(ntp, 3600000UL);
}

void loop() {
    ntp.tick();
    rtc.tick(); // Melakukan cek sinkronisasi otomatis di background
}
```

---

## 📖 Referensi API Publik

### Inisialisasi & Status
* `bool begin(TwoWire& wire = Wire, IskakRTCType type = IskakRTCType::AUTO)`: Memulai komunikasi I2C dan mendeteksi chip RTC.
* `bool isRunning()`: Memeriksa apakah osilator RTC aktif.
* `bool lostPower()`: Mengembalikan `true` jika catu daya RTC sempat terputus (perlu disetel ulang).
* `IskakRTCType chipType()`: Mendapatkan enum tipe chip (`DS3231`, `DS1307`, `PCF8563`).
* `const char* chipName()`: Nama teks tipe chip yang terdeteksi.

### Waktu & Kalender
* `IskakDateTime now()`: Membaca tanggal dan jam saat ini ke dalam struct `IskakDateTime`.
* `void setDateTime(const IskakDateTime& dt)`: Menyetel tanggal dan jam via struct.
* `void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)`: Menyetel waktu langsung.
* `void setEpoch(uint32_t epoch)`: Menyetel waktu RTC dari nilai UNIX epoch detik.
* `uint32_t getEpoch()`: Membaca waktu saat ini dalam format UNIX epoch.
* `float getTemperature()`: Membaca sensor suhu on-chip (khusus DS3231, mengembalikan `0.0` pada chip lain).

### Sinergi NTP & Scheduler
* `void syncWithNTP(IskakINO_FastNTP& ntp, uint32_t intervalMs = 3600000UL)`: Mendaftarkan instance FastNTP untuk sinkronisasi otomatis berkala (khusus ESP32/ESP8266).
* `void tick()`: Memproses scheduler sinkronisasi background.
