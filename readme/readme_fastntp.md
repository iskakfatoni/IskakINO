# ⏰ Modul: IskakINO_FastNTP

Modul sinkronisasi waktu internet NTP (*Network Time Protocol*) asinkron dan presisi untuk **ESP32 & ESP8266**. Dilengkapi konversi waktu lokal instan, lokalisasi nama hari/bulan (Bahasa Indonesia & English), dan sistem *multi-server fallback*.

---

## 🛠️ Fitur Utama

1. **Sinkronisasi Non-Blocking:** Mengambil waktu dari server NTP dunia di latar belakang tanpa menghentikan eksekusi program (`delay` = 0).
2. **Multi-Server Redundancy:** Beralih otomatis ke server cadangan jika server NTP utama tidak merespons.
3. **Format Waktu & Tanggal Siap Cetak:** Menyediakan fungsi instan seperti `getFormattedTime()` (`14:05:30`) dan `getFormattedDate()`.
4. **Lokalisasi Bahasa (Bahasa Indonesia & English):** Konversi nama hari (*Senin, Selasa, dst.*) dan bulan (*Januari, Februari, dst.*) otomatis.
5. **Kompensasi Zona Waktu:** Pengaturan *offset* GMT/UTC fleksibel (misal: GMT+7 = `25200` detik untuk WIB).

---

## 💻 Contoh Penggunaan Singkat

```cpp
#include <IskakINO.h>
#include <WiFiUdp.h>

WiFiUDP ntpUdp;
IskakINO_FastNTP ntp(ntpUdp, "pool.ntp.org");

void setup() {
    Serial.begin(115200);

    // Atur timezone GMT+7 (WIB = 25200 detik), tanpa DST (0)
    ntp.begin(25200, 0);
}

void loop() {
    // WAJIB: Panggil update() di setiap loop untuk sinkronisasi waktu berkala
    ntp.update();

    if (ntp.isTimeSet()) {
        // Cetak format waktu instan: "Senin, 19 Agustus 2026 10:30:00"
        Serial.print(ntp.getDayName(LANG_ID));
        Serial.print(", ");
        Serial.print(ntp.getDay());
        Serial.print(" ");
        Serial.print(ntp.getMonthName(LANG_ID));
        Serial.print(" ");
        Serial.print(ntp.getYear());
        Serial.print(" - ");
        Serial.println(ntp.getFormattedTime());
    }
    delay(1000);
}
```

---

## 📖 Referensi API

### Inisialisasi & Siklus Hidup
* `void begin(long gmtOffset = 25200, int daylightOffset = 0)`: Mengonfigurasi offset zona waktu (detik) dan offset DST.
* `void update()`: Memproses request UDP dan sinkronisasi berkala.
* `void forceUpdate()`: Memaksa sinkronisasi ulang ke server NTP secara langsung.
* `bool isTimeSet()`: Mengembalikan `true` jika waktu sudah berhasil disinkronkan minimal satu kali.
* `bool isTimeReliable()`: Mengembalikan `true` jika sinkronisasi masih dalam batas validitas interval.

### Akses Nilai Waktu & Tanggal
* `uint32_t getEpoch()`: Mengembalikan timestamp Unix lokal (detik sejak 1 Jan 1970).
* `uint32_t getUtcEpoch()`: Mengembalikan timestamp Unix UTC murni.
* `uint8_t getHours()`, `uint8_t getMinutes()`, `uint8_t getSeconds()`: Jam, menit, dan detik.
* `uint8_t getDay()`, `uint8_t getMonth()`, `uint16_t getYear()`: Tanggal, bulan, dan tahun.
* `uint8_t getDayOfWeek()`: Hari dalam seminggu (0 = Minggu s/d 6 = Sabtu).
* `String getDayName()`: Nama hari dalam teks (misal: *"Senin"* atau *"Monday"*).
* `String getMonthName()`: Nama bulan dalam teks (misal: *"Agustus"* atau *"August"*).
* `String getFormattedTime()`: Teks waktu dengan format `HH:MM:SS`.

---

## 📂 Penjelasan Contoh Sketsa (`examples/06_FastNTP_ClockSync`)

* **Lokasi Sketsa:** [`examples/06_FastNTP_ClockSync/06_FastNTP_ClockSync.ino`](../examples/06_FastNTP_ClockSync/06_FastNTP_ClockSync.ino)
* **Platform Target:** **ESP32 & ESP8266**.
* **Fokus Pembelajaran:**
  1. Menghubungkan modul `FastNTP` dengan koneksi WiFi aktif.
  2. Mendaftarkan callback keberhasilan sinkronisasi (`onSync()`).
  3. Memperagakan pemformatan teks tanggal dan waktu berbahasa Indonesia yang siap ditampilkan ke Serial Monitor atau LCD.
