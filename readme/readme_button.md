# 🔘 Modul: IskakINO_Button

Driver tombol fisik pintar dengan deteksi *gesture* multi-aksi non-blocking (*Single Click*, *Double Click*, *Multi-Click*, & *Long Press / Hold*), dilengkapi *software debouncing* otomatis.

---

## 🛠️ Fitur Utama

1. **Multi-Aksi dari 1 Pin Tombol:**
   * Membedakan klik tunggal (*Single Click*), klik ganda (*Double Click*), klik berturut-turut (*Multi-Click*), dan tekan-tahan (*Long Press / Hold*).
2. **Software Debouncing Otomatis:**
   * Menghilangkan derau getaran mekanis saklar/push-button tanpa komponen hardware tambahan.
3. **Fleksibel dalam Pengkabelan:**
   * Mendukung mode `ACTIVE_LOW` (default dengan internal `INPUT_PULLUP`) dan mode `ACTIVE_HIGH` (`INPUT` dengan pull-down eksternal).
4. **Dua Gaya Pemrograman:**
   * **Polling Real-Time:** `if (btn.isClicked()) ...`, `if (btn.isDoubleClicked()) ...`, `if (btn.isLongPressed()) ...`
   * **Event-Driven Callback:** `btn.onClick(cb)`, `btn.onDoubleClick(cb)`, `btn.onLongPressStart(cb)`.

---

## 🔌 Diagram Koneksi Pin Hardware

```
[Mikrokontroler (AVR / ESP)]             [Push Button]
GND --------------------------------------- Pin 1 Tombol
Pin GPIO (misal D2 / GPIO4) --------------- Pin 2 Tombol
```
*(Tidak memerlukan resistor eksternal karena menggunakan internal `INPUT_PULLUP`)*

---

## 💻 Contoh Penggunaan Singkat

### 1. Pola Polling Real-Time
```cpp
#include <IskakINO.h>

// Inisialisasi tombol pada pin D2 (Active LOW / INPUT_PULLUP)
IskakINO_Button btn(2);

void setup() {
    Serial.begin(115200);
    btn.begin();
}

void loop() {
    // WAJIB: Panggil update() di loop untuk memproses state-machine tombol
    btn.update();

    if (btn.isClicked()) {
        Serial.println(">> Single Click terdeteksi!");
    }

    if (btn.isDoubleClicked()) {
        Serial.println(">> Double Click terdeteksi!");
    }

    if (btn.isLongPressed()) {
        Serial.println(">> Long Press dimulai!");
    }
}
```

### 2. Pola Event Callback
```cpp
#include <IskakINO.h>

IskakINO_Button btn(2);

void onBtnClick() {
    Serial.println("Tombol diklik sekali!");
}

void onBtnDouble() {
    Serial.println("Tombol diklik dua kali!");
}

void setup() {
    Serial.begin(115200);
    btn.begin();
    
    btn.onClick(onBtnClick);
    btn.onDoubleClick(onBtnDouble);
}

void loop() {
    btn.update();
}
```

---

## 📖 Referensi API Publik

### Inisialisasi & Konfigurasi Timing
* `void begin(uint8_t pin, bool activeLow = true, bool pullup = true)`: Inisialisasi pin tombol.
* `void setDebounceMs(uint16_t ms)`: Mengatur durasi debouncing (default: 40 ms).
* `void setClickWindowMs(uint16_t ms)`: Jendela waktu menunggu klik berikutnya (default: 250 ms).
* `void setLongPressMs(uint16_t ms)`: Ambang batas waktu tekan-tahan long press (default: 600 ms).

### Polling Status & Event
* `void update()`: Memproses pembacaan dan status tombol (wajib di `loop()`).
* `bool isPressed() const`: Memeriksa apakah tombol sedang dalam kondisi fisik tertekan.
* `bool isReleased() const`: Memeriksa apakah tombol sedang dalam kondisi lepas.
* `bool isClicked()`: Bernilai `true` sekali jika terjadi *Single Click*.
* `bool isDoubleClicked()`: Bernilai `true` sekali jika terjadi *Double Click*.
* `bool isLongPressed()`: Bernilai `true` sekali saat tombol ditahan melewati threshold long press.
* `bool isHolding() const`: Bernilai `true` terus menerus selama tombol masih ditahan.
* `uint32_t getHoldDuration() const`: Mengembalikan durasi (ms) tombol telah ditahan saat ini.
* `uint8_t getClickCount()`: Mengembalikan akumulasi total klik (1, 2, 3, dst.).

### Callback Event
* `void onClick(ButtonCallback cb)`: Mendaftarkan fungsi callback saat single click.
* `void onDoubleClick(ButtonCallback cb)`: Mendaftarkan fungsi callback saat double click.
* `void onLongPressStart(ButtonCallback cb)`: Mendaftarkan fungsi callback saat long press dimulai.
* `void onLongPressEnd(ButtonCallback cb)`: Mendaftarkan fungsi callback saat tombol dilepas setelah long press.
