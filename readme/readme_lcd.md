# 📟 Modul: IskakINO_LiquidCrystal_I2C (LCD)

Driver layar karakter LCD I2C (16x2, 20x4) dengan animasi visual modern non-blocking (*typewriter*, *smooth scroll marquee*, grafik *progress bar*, *custom icon generator*, & *dynamic banner / page flipper*) tanpa memperlambat program utama.

---

## 🛠️ Fitur Utama

1. **Efek Mesin Ketik (*Typewriter*):** Animasi kemunculan karakter per karakter yang berjalan asinkron.
2. **Teks Berjalan (*Smooth Marquee Scroll*):** Menggulirkan teks panjang pada baris tertentu secara otomatis tanpa memblokir CPU.
3. **Grafik Progress Bar Built-in:** Menggambar indikator persentase loading/progress (0–100%) menggunakan generator karakter kustom (slot 7).
4. **Generator Ikon Kustom Dinamis (*Custom Icon Generator*):** Membuat dan menggambar ikon status baterai bertingkat (0–100%), level sinyal Wi-Fi bertingkat (0–4 bar / RSSI dBm), dan level termometer tanpa manipulasi byte manual.
5. **Dynamic Banner / Multi-Page Flipper:** Menjalankan transisi multi-halaman layar otomatis dengan interval waktu berkala (*page flipper*), mendukung konfigurasi teks statis (`LCDPage`) maupun callback dinamis (`LCDBannerCallback`) untuk sensor/data realtime.
6. **Auto-Timeout Backlight:** Penghemat daya otomatis yang mematikan lampu latar LCD setelah durasi idle tertentu.
7. **Utilitas Format Teks:** Menampilkan teks rata tengah (`printCenter()`) dan format angka siap cetak (`printFormatted()`).

---

## 💻 Contoh Penggunaan Singkat

```cpp
#include <IskakINO.h>

// Inisialisasi LCD (16 kolom, 2 baris)
LiquidCrystal_I2C lcd(16, 2);

// Definisi halaman banner
const LCDPage pages[] = {
    LCDPage("IskakINO Core", "Smart Library"),
    LCDPage("Status: Ready", "WiFi: Connected")
};

void setup() {
    lcd.begin();
    lcd.backlight();

    // Gambar custom icons otomatis di baris atas
    lcd.drawBattery(15, 0, 85);          // Baterai 85% di col 15, row 0
    lcd.drawWifiSignalRssi(14, 0, -65); // Sinyal WiFi 3 bar dari RSSI di col 14, row 0

    // Jalankan dynamic banner (ganti halaman tiap 3 detik)
    lcd.bannerStart(pages, 2, 3000);
}

void loop() {
    // WAJIB: Panggil update() di setiap putaran loop untuk menggerakkan animasi & banner
    lcd.update();
}
```

---

## 📖 Referensi API

### Kontrol Dasar & Teks
* `void begin()`: Inisialisasi komunikasi I2C LCD (auto-scan alamat `0x27` / `0x3F`).
* `void setCursor(uint8_t col, uint8_t row)`: Mengatur posisi kursor.
* `void printCenter(const char* text, int row)`: Menampilkan teks tepat di tengah baris.
* `void printFormatted(const char* format, ...)`: Menampilkan teks dengan format `printf`.
* `void backlight()` / `void noBacklight()`: Mengaktifkan atau mematikan lampu latar LCD.
* `void setBacklightTimeout(unsigned long timeoutMs)`: Mengatur durasi mati otomatis lampu latar (0 = nonaktif).

### Animasi Non-Blocking
* `void update()`: Memproses frame animasi typewriter, marquee scroll, progress bar, banner, dan timeout backlight.
* `void typewriterStart(const char* text, int row, int delayTime = 100)`: Memulai animasi typewriter.
* `void typewriterStop()`: Menghentikan animasi typewriter.
* `bool isTypewriterActive()`: Memeriksa apakah efek typewriter sedang berjalan.
* `void scrollTextStart(const char* text, int row, uint16_t intervalMs = 300)`: Memulai teks berjalan pada satu baris.
* `void scrollTextStop()`: Menghentikan teks berjalan.
* `bool isScrollActive()`: Memeriksa apakah teks berjalan sedang aktif.
* `void drawProgressBar(uint8_t percent, uint8_t row)`: Menggambar grafik progress bar (0 - 100%).

### 🎨 Custom Icon Generator
* `void createBatteryIcon(uint8_t slot, uint8_t percent)`: Membuat custom character baterai (0–100%) pada slot CGRAM tertentu (0–6).
* `void createWifiIcon(uint8_t slot, uint8_t level_0_to_4)`: Membuat custom character bar sinyal Wi-Fi (0–4 bar).
* `void createWifiIconRssi(uint8_t slot, int rssi)`: Membuat custom character bar sinyal Wi-Fi langsung dari nilai dBm RSSI (mis. `-65`).
* `void createThermometerIcon(uint8_t slot, uint8_t level_0_to_3)`: Membuat custom character termometer (level 0–3).
* `void drawBattery(uint8_t col, uint8_t row, uint8_t percent, uint8_t slot = 0)`: Membuat dan langsung mencetak ikon baterai di posisi layar.
* `void drawWifiSignal(uint8_t col, uint8_t row, uint8_t level_0_to_4, uint8_t slot = 1)`: Membuat dan langsung mencetak ikon sinyal Wi-Fi di posisi layar.
* `void drawWifiSignalRssi(uint8_t col, uint8_t row, int rssi, uint8_t slot = 1)`: Membuat dan langsung mencetak ikon sinyal Wi-Fi dari RSSI di posisi layar.
* `void drawThermometer(uint8_t col, uint8_t row, uint8_t level_0_to_3, uint8_t slot = 2)`: Membuat dan langsung mencetak ikon termometer di posisi layar.

### 🔄 Dynamic Banner / Page Flipper
* `void bannerStart(const LCDPage* pages, uint8_t pageCount, uint16_t flipIntervalMs = 3000)`: Memulai banner otomatis dengan daftar array halaman teks statis.
* `void bannerStart(uint8_t pageCount, uint16_t flipIntervalMs, LCDBannerCallback callback)`: Memulai banner otomatis dengan callback dinamis (mis. membaca data sensor realtime tiap halaman).
* `void bannerStop()`: Menghentikan banner.
* `void bannerPause()` / `void bannerResume()`: Menjeda atau melanjutkan perpindahan halaman banner.
* `void bannerNext()` / `void bannerPrev()`: Berpindah ke halaman berikutnya / sebelumnya secara manual.
* `void bannerSetPage(uint8_t pageIndex)`: Melompat ke indeks halaman tertentu.
* `uint8_t bannerGetCurrentPage() const`: Mendapatkan indeks halaman yang sedang aktif.
* `bool isBannerActive() const`: Memeriksa apakah banner sedang aktif.

---

## 📂 Penjelasan Contoh Sketsa (`examples/03_LCD_TypewriterScroll`)

* **Lokasi Sketsa:** [`examples/03_LCD_TypewriterScroll/03_LCD_TypewriterScroll.ino`](../examples/03_LCD_TypewriterScroll/03_LCD_TypewriterScroll.ino)
* **Platform Target:** Universal (Arduino AVR Uno/Nano/Mega, ESP8266, ESP32).
* **Fokus Pembelajaran:**
  1. Menggunakan animasi `typewriterStart()` untuk menyapa pengguna saat boot awal.
  2. Menampilkan teks berjalan panjang (`scrollTextStart()`) di baris bawah.
  3. Menggambar visualisasi progress bar (`drawProgressBar()`) yang bertambah secara dinamis.
  4. Menggambar Custom Icons dinamis (`drawBattery()`, `drawWifiSignalRssi()`, `drawThermometer()`).
  5. Menjalankan Dynamic Banner multi-halaman (`bannerStart()`) non-blocking.
  6. Menjaga fungsi `loop()` tetap berjalan cepat dan responsif tanpa menggunakan `delay()`.
