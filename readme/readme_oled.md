# 🖥️ Modul: IskakINO_OLED

Driver layar grafis OLED I2C (chipset **SSD1306** & **SH1106**) yang dirancang **ultra-hemat RAM** dengan arsitektur *Direct-Page Streaming Rendering* (< 40 bytes RAM), dilengkapi animasi teks *non-blocking*, generator ikon siap pakai, teks ukuran ganda (2x), dan grafik progress bar.

---

## 🛠️ Fitur Utama

1. **Ultra-Hemat Memori RAM (< 40 bytes RAM):**
   * Berbeda dengan library umum yang memakan buffer 1024 bytes (50% dari 2KB RAM Arduino Uno), modul ini melakukan rendering langsung ke internal RAM OLED per-page tanpa membebani RAM mikrokontroler.
2. **Multi-Chipset & Resolusi:**
   * Mendukung controller **SSD1306** (128x64 & 128x32) dan **SH1106** (128x64) secara otomatis pada alamat I2C `0x3C` atau `0x3D`.
3. **Animasi Non-Blocking:**
   * **Typewriter:** Animasi teks mesin ketik per karakter tanpa `delay()`.
   * **Marquee Scrolling:** Teks berjalan horizontal pada baris/halaman tertentu.
   * **Progress Bar:** Grafik bar progres (0 - 100%) dengan batas kurung `[====  ]`.
4. **Ikon Grafis Siap Pakai di Flash (`PROGMEM`):**
   * Level Sinyal WiFi (0 s/d 4 bar).
   * Indikator Baterai (0% s/d 100% dan mode Charging).
   * Simbol Status: Lonceng, Termometer, Gembok/Kunci, Centang, Silang, Panah.
5. **Dukungan Format & Font:**
   * Ukuran font standar 1x (6x8 px, muat 21 karakter per baris) dan 2x (12x16 px).
   * Helper `printCenter()`, `printRight()`, dan `printf()`.

---

## 🔌 Diagram Koneksi Pin Hardware

```
[Mikrokontroler (AVR / ESP)]             [OLED Display I2C]
VCC (3.3V / 5V) ------------------------------- VCC
GND ------------------------------------------- GND
SCL (A5 pada Uno / GPIO22 pada ESP32) --------- SCL
SDA (A4 pada Uno / GPIO21 pada ESP32) --------- SDA
```

---

## 💻 Contoh Penggunaan Singkat

### 1. Pola Modular Manual
```cpp
#include <IskakINO.h>

// Inisialisasi OLED 128x64 pada alamat I2C 0x3C
IskakINO_OLED oled(128, 64, 0x3C);

void setup() {
    oled.begin();
    oled.clear();

    // Tampilkan judul rata tengah dengan font 2x
    oled.setTextSize(2);
    oled.printCenter("IskakINO", 0);

    // Tampilkan status & ikon di baris bawah
    oled.setTextSize(1);
    oled.setCursor(0, 4);
    oled.print("WiFi:");
    oled.drawWifiIcon(4, 35, 4); // 4 bar penuh

    oled.setCursor(60, 4);
    oled.print("Bat:");
    oled.drawBatteryIcon(3, 90, 4); // 75%

    // Jalankan efek teks mesin ketik di baris 6
    oled.typewriterStart("System Ready!", 0, 6, 80);
}

void loop() {
    // WAJIB: Panggil update() di loop untuk memproses animasi teks
    oled.update();
}
```

### 2. Pola Framework Kernel
```cpp
#include <IskakINO.h>

IskakINO_OLED oled(128, 64);
IskakINO_OLEDModule oledMod(oled);

void setup() {
    IskakINO.registerModule(&oledMod);
    IskakINO.begin(); // Otomatis menginisialisasi I2C & OLED
    
    oled.printCenter("Kernel Mode", 2);
}

void loop() {
    IskakINO.update(); // Otomatis me-refresh animasi di background
}
```

---

## 📖 Referensi API Publik

### Inisialisasi & Pengaturan Layar
* `bool begin(TwoWire &wirePort = Wire)`: Inisialisasi komunikasi I2C & konfigurasi chip.
* `bool isConnected() const`: Memeriksa apakah layar terdeteksi pada bus I2C.
* `void clear()`: Membersihkan seluruh layar (8 baris/page).
* `void clearRow(uint8_t row)`: Membersihkan 1 baris/page tertentu (row: 0 - 7).
* `void clearArea(uint8_t startCol, uint8_t row, uint8_t widthCols)`: Membersihkan sebagian kolom.
* `void setCursor(uint8_t col, uint8_t row)`: Mengatur posisi kolom (0-127) dan baris (0-7).
* `void setTextSize(uint8_t size)`: Mengatur ukuran teks (1 = Normal 6x8px, 2 = Double 12x16px).
* `void invertDisplay(bool invert)`: Mode invert warna (latar putih, piksel hitam).
* `void setContrast(uint8_t contrast)`: Mengatur tingkat kecerahan layar (0 - 255).
* `void displayOn()` / `void displayOff()`: Menyalakan atau mematikan panel OLED.

### Teks & Format
* `void printCenter(const char* text, uint8_t row)`: Mencetak teks tepat di tengah baris.
* `void printRight(const char* text, uint8_t row)`: Mencetak teks rata kanan.
* `void printf(const char* fmt, ...)`: Mencetak teks dengan format string `printf`.

### Ikon & Grafis
* `void drawWifiIcon(uint8_t level, uint8_t col, uint8_t row)`: Menggambar ikon sinyal WiFi (level: 0 s/d 4).
* `void drawBatteryIcon(uint8_t level, uint8_t col, uint8_t row)`: Menggambar ikon baterai (level: 0 s/d 5).
* `void drawIcon(const uint8_t* iconProgmem, uint8_t col, uint8_t row, uint8_t width = 8)`: Menggambar ikon bitmap dari `PROGMEM`.
* `void drawHLine(uint8_t startCol, uint8_t row, uint8_t width, uint8_t pattern = 0x01)`: Garis horizontal custom pattern.
* `void drawProgressBar(uint8_t percent, uint8_t row, uint8_t startCol = 0, uint8_t endCol = 127)`: Menggambar progress bar (0 - 100%).

### Animasi Non-Blocking
* `void update()`: Memproses frame typewriter dan teks berjalan (wajib di `loop()`).
* `void typewriterStart(const char* text, uint8_t col = 0, uint8_t row = 0, uint16_t intervalMs = 60)`: Memulai animasi efek mesin ketik.
* `void typewriterStop()`: Menghentikan efek mesin ketik.
* `bool isTypewriterActive() const`: Mengecek status aktif typewriter.
* `void scrollTextStart(const char* text, uint8_t row = 0, uint16_t intervalMs = 150)`: Memulai teks berjalan pada baris tertentu.
* `void scrollTextStop()`: Menghentikan teks berjalan.
* `bool isScrollActive() const`: Mengecek status aktif teks berjalan.
