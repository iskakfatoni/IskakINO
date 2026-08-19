# 📟 Modul: IskakINO_LiquidCrystal_I2C (LCD)

Driver layar karakter LCD I2C (16x2, 20x4) dengan animasi visual modern non-blocking (*typewriter*, *smooth scroll marquee*, & grafik *progress bar*) tanpa memperlambat program utama.

---

## 🛠️ Fitur Utama

1. **Efek Mesin Ketik (*Typewriter*):** Animasi kemunculan karakter per karakter yang berjalan asinkron.
2. **Teks Berjalan (*Smooth Marquee Scroll*):** Menggulirkan teks panjang pada baris tertentu secara otomatis tanpa memblokir CPU.
3. **Grafik Progress Bar Custom:** Menggambar indikator persentase loading/progress menggunakan generator custom character.
4. **Auto-Timeout Backlight:** Penghemat daya otomatis yang mematikan lampu latar LCD setelah durasi tertentu.
5. **Utilitas Format Teks:** Menampilkan teks rata tengah (`printCenter()`) dan format angka siap cetak (`printFormatted()`).

---

## 💻 Contoh Penggunaan Singkat

```cpp
#include <IskakINO.h>

// Inisialisasi LCD (alamat default 0x27, 16 kolom, 2 baris)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    lcd.begin();
    lcd.backlight();

    // Jalankan efek mesin ketik di baris 0
    lcd.typewriterStart("IskakINO LCD", 0, 0, 80);

    // Jalankan teks berjalan di baris 1
    lcd.scrollTextStart("Selamat datang di ekosistem IskakINO!", 1, 250);
}

void loop() {
    // WAJIB: Panggil update() di setiap putaran loop untuk menggerakkan animasi
    lcd.update();
}
```

---

## 📖 Referensi API

### Kontrol Dasar & Teks
* `void begin()`: Inisialisasi komunikasi I2C LCD.
* `void setCursor(uint8_t col, uint8_t row)`: Mengatur posisi kursor.
* `void printCenter(const char* text, uint8_t row)`: Menampilkan teks tepat di tengah baris.
* `void printFormatted(uint8_t col, uint8_t row, const char* format, ...)`: Menampilkan teks dengan format `printf`.
* `void backlight()` / `void noBacklight()`: Mengaktifkan atau mematikan lampu latar LCD.
* `void setBacklightTimeout(uint32_t timeoutMs)`: Mengatur durasi mati otomatis lampu latar.

### Animasi Non-Blocking
* `void update()`: Memproses frame animasi typewriter, marquee scroll, dan timeout backlight.
* `void typewriterStart(const char* text, uint8_t col, uint8_t row, uint16_t delayMs)`: Memulai animasi typewriter.
* `void typewriterStop()`: Menghentikan animasi typewriter.
* `void scrollTextStart(const char* text, uint8_t row, uint16_t delayMs)`: Memulai teks berjalan pada satu baris.
* `void scrollTextStop()`: Menghentikan teks berjalan.
* `void drawProgressBar(uint8_t row, uint8_t percent)`: Menggambar grafik progress bar (0 - 100%).

---

## 📂 Penjelasan Contoh Sketsa (`examples/03_LCD_TypewriterScroll`)

* **Lokasi Sketsa:** [`examples/03_LCD_TypewriterScroll/03_LCD_TypewriterScroll.ino`](../examples/03_LCD_TypewriterScroll/03_LCD_TypewriterScroll.ino)
* **Platform Target:** Universal (Arduino AVR, ESP8266, ESP32).
* **Fokus Pembelajaran:**
  1. Menggunakan animasi `typewriterStart()` untuk menyapa pengguna saat boot awal.
  2. Menampilkan teks berjalan panjang (`scrollTextStart()`) di baris bawah.
  3. Menggambar visualisasi progress bar (`drawProgressBar()`) yang bertambah secara dinamis.
  4. Menjaga fungsi `loop()` tetap berjalan cepat dan responsif tanpa menggunakan `delay()`.
