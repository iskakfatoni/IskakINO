# 🛡️ Modul: IskakINO_BasicIOShield

Driver terpadu untuk modul pembelajaran **EMS Basic I/O Shield** (diproduksi oleh Innovative Electronics - Training Division) khusus untuk mikrokontroler berbasis **Arduino AVR** (Arduino Uno, Nano, Mega, Duemilanove, dll).

---

## 🛠️ Fitur & Spesifikasi Hardware

* 💡 **4x Output LED Digital:** LED Merah (*Red* / D9), Kuning (*Yellow* / D8), Biru (*Blue* / D7), dan Hijau (*Green* / D6).
* 🔘 **2x Push Button Input Digital:** Button 1 (D2) & Button 2 (D4).
* 🎛️ **1x Potensiometer Input Analog:** Pembacaan ADC tegangan putar (Pin A1, rentang 0 s/d 1023).
* 🔢 **Dual-Digit 7-Segment Display:** Peragaan angka multiplexing 2-digit *non-blocking* (Pin A2 & A3 untuk selektor transistor digit, Pin 6-13 untuk segmen A..DP).
* ⚡ **10-Bit I2C Digital-to-Analog Converter (DAC):** Output tegangan analog presisi via IC **AD5612** (Alamat I2C `0x0E`, Pin SDA A4 & SCL A5).

---

## 📌 Pemetaan Pinout Hardware (Pin Mapping)

| Komponen Hardware | Pin Arduino | Fungsi & Arah Data |
|---|---|---|
| **LED Hijau (*Green*)** | `D6` | Output Digital (`HIGH`/`LOW`) |
| **LED Biru (*Blue*)** | `D7` | Output Digital (`HIGH`/`LOW`) |
| **LED Kuning (*Yellow*)** | `D8` | Output Digital (`HIGH`/`LOW`) |
| **LED Merah (*Red*)** | `D9` | Output Digital (`HIGH`/`LOW`) |
| **Button 1** | `D2` | Input Digital (Aktif `HIGH`) |
| **Button 2** | `D4` | Input Digital (Aktif `HIGH`) |
| **Potensiometer** | `A1` | Input Analog ADC (0 – 1023) |
| **7-Segment Transistor Digit 1** | `A2` | Selektor Digit Satuan / Multiplexer |
| **7-Segment Transistor Digit 2** | `A3` | Selektor Digit Puluhan / Multiplexer |
| **7-Segment Segmen A..DP** | `D6` s/d `D13` | Segmen A, B, C, D, E, F, G, DP |
| **I2C DAC (IC AD5612)** | `SDA (A4)`, `SCL (A5)` | Antarmuka I2C DAC (Alamat `0x0E`) |

---

## 💻 Contoh Penggunaan Singkat

```cpp
#include <IskakINO.h>

IskakINO_BasicIOShield shield;

void setup() {
    // Inisialisasi seluruh pin dan komunikasi I2C
    shield.begin();

    // Atur angka yang ingin ditampilkan (0 - 99) secara non-blocking
    shield.setDisplay(42);
}

void loop() {
    // WAJIB: Panggil update() di setiap putaran loop untuk me-refresh 7-segment
    shield.update();

    // Kontrol LED berdasarkan tombol
    if (shield.Button1State() == HIGH) {
        shield.RedOn();
    } else {
        shield.RedOff();
    }

    // Baca potensiometer dan kirim ke output DAC
    uint16_t pot = shield.ReadPotentiometer();
    shield.WriteDAC(pot);
}
```

---

## 📖 Referensi API

### Inisialisasi & Input
* `void begin()`: Mengonfigurasi `pinMode` seluruh LED, tombol, 7-segment, dan `Wire.begin()`.
* `uint16_t ReadPotentiometer()`: Membaca nilai ADC potensiometer (0 - 1023).
* `bool Button1State()`: Membaca status Button 1 (`HIGH` saat ditekan).
* `bool Button2State()`: Membaca status Button 2 (`HIGH` saat ditekan).

### Kontrol LED
* `void RedOn()` / `void RedOff()`: Menyalakan / mematikan LED Merah.
* `void GreenOn()` / `void GreenOff()`: Menyalakan / mematikan LED Hijau.
* `void BlueOn()` / `void BlueOff()`: Menyalakan / mematikan LED Biru.
* `void YellowOn()` / `void YellowOff()`: Menyalakan / mematikan LED Kuning.

### 7-Segment Multiplexing
* `void setDisplay(uint8_t number)`: Menentukan angka 2-digit (0 s/d 99) untuk display non-blocking.
* `void update()`: Me-refresh pergantian transistor digit multiplexing tanpa fungsi `delay()`.
* `void clearDisplay()`: Mematikan seluruh segmen dan display 7-segment.
* `void setRefreshInterval(uint16_t intervalMicros = 3000)`: Mengatur durasi multiplexing antar-digit (default: 3 ms).
* `void PrintSevenSegment(uint8_t number, uint8_t times = 10, uint8_t delayTime = 5)`: Menampilkan angka dengan loop blocking (*legacy*).

### Output Analog I2C DAC
* `bool WriteDAC(uint16_t value)`: Mengirimkan data digital 10-bit (0 s/d 1023) ke IC DAC AD5612. Mengembalikan `true` jika komunikasi I2C berhasil.

---

## 📂 Penjelasan Contoh Sketsa (`examples/10_BasicIOShield_Overview`)

* **Lokasi Sketsa:** [`examples/10_BasicIOShield_Overview/10_BasicIOShield_Overview.ino`](../examples/10_BasicIOShield_Overview/10_BasicIOShield_Overview.ino)
* **Platform Target:** **Khusus Arduino AVR** (Arduino Uno, Nano, Mega, Duemilanove, dll).
* **Fokus Pembelajaran:**
  1. Menjalankan inisialisasi shield dengan `shield.begin()`.
  2. Membaca interaksi Button 1 & Button 2 untuk mengendalikan kombinasi 4 LED.
  3. Membaca nilai analog potensiometer (A1), mengonversikannya ke rentang 0-99, dan memperbarui display 7-segment secara asinkron (`setDisplay()`).
  4. Mengalirkan tegangan analog presisi ke pin DAC AD5612 (`WriteDAC()`) secara real-time.
