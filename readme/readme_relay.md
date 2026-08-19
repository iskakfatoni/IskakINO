# ⚡ Modul: IskakINO_Relay

Driver relay dan aktuator pintar non-blocking dengan fitur *auto-off pulse timer*, pola kedip *blink cadence*, status startup yang aman (*glitch-free boot*), serta proteksi *switching chatter*.

---

## 🛠️ Fitur Utama

1. **Pulse Timer Otomatis Non-Blocking:**
   * `pulse(3000)`: Menyalakan relay selama 3 detik lalu otomatis mati sendiri tanpa `delay()`. Sangat ideal untuk selenoid pintu otomatis, kran air, atau saklar bel.
2. **Blink Cadence Non-Blocking:**
   * `blink(onMs, offMs, repeatCount)`: Mengedipkan/memutus-sambung relay secara teratur dengan jumlah perulangan tertentu atau berulang tanpa henti.
3. **Booting Bebas Glitch (Safe Boot):**
   * Menjamin relay tetap dalam kondisi mati (OFF) saat mikrokontroler baru menyala atau restart.
4. **Mendukung Active HIGH & Active LOW:**
   * Kompatibel dengan modul relay optocoupler standar (Active LOW) maupun transistor driver (Active HIGH).
5. **Proteksi Switching Chatter:**
   * Mencegah perpindahan status relay yang terlalu cepat untuk memperpanjang usia mekanis kontak relay.

---

## 🔌 Diagram Koneksi Pin Hardware

```
[Mikrokontroler (AVR / ESP)]             [Modul Relay I/O]
VCC (5V / 3.3V) ------------------------------- VCC
GND ------------------------------------------- GND
Pin GPIO (misal D7 / GPIO12) ------------------ IN (Signal)
```

---

## 💻 Contoh Penggunaan Singkat

### 1. Kontrol Relay Sederhana & Auto-Off Pulse
```cpp
#include <IskakINO.h>

// Inisialisasi relay pada pin 7 (Active LOW default)
IskakINO_Relay relay(7, ISKAK_RELAY_ACTIVE_LOW);

void setup() {
    relay.begin();

    // Picu relay menyala selama 2.5 detik lalu mati sendiri
    relay.pulse(2500);
}

void loop() {
    // WAJIB: Panggil update() di loop untuk memproses timer pulse & blink
    relay.update();
}
```

### 2. Pola Blink / Kedipan Ritmik
```cpp
#include <IskakINO.h>

IskakINO_Relay relay(7);

void setup() {
    relay.begin();

    // Kedipkan relay 3 kali (ON 300ms, OFF 200ms)
    relay.blink(300, 200, 3);
}

void loop() {
    relay.update();
}
```

---

## 📖 Referensi API Publik

### Inisialisasi
* `void begin(uint8_t pin, IskakRelayMode mode = ISKAK_RELAY_ACTIVE_LOW, bool initialOn = false)`: Inisialisasi pin relay dan set level aman.
* `void setMinSwitchInterval(uint32_t intervalMs)`: Mengatur durasi proteksi jeda minimum antar perpindahan state (ms).

### Kontrol Status
* `void on()`: Menyalakan relay secara permanen.
* `void off()`: Mematikan relay secara permanen.
* `void toggle()`: Membalikkan status relay (ON $\leftrightarrow$ OFF).
* `void set(bool state)`: Mengatur status relay langsung (`true` = ON, `false` = OFF).
* `void pulse(uint32_t durationMs)`: Menyalakan relay selama durasi tertentu lalu mati otomatis.
* `void blink(uint32_t onMs, uint32_t offMs, uint8_t repeatCount = 0)`: Menjalankan ritme blink.
* `void stop()`: Menghentikan efek pulse / blink dan mematikan relay.

### Polling Status
* `void update()`: Memproses timer pulse dan cadence blink (wajib di `loop()`).
* `bool isOn() const`: Memeriksa apakah relay sedang aktif (ON).
* `bool isOff() const`: Memeriksa apakah relay sedang mati (OFF).
* `bool isPulsing() const`: Memeriksa apakah relay sedang dalam mode pulse timer.
* `bool isBlinking() const`: Memeriksa apakah relay sedang dalam mode blink.
