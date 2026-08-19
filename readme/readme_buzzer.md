# 🔔 Modul: IskakINO_Buzzer

Driver buzzer *non-blocking* lintas platform untuk mikrokontroler Arduino AVR, ESP8266, dan ESP32. Menyediakan generator nada status instan, pemutar melodi format ringtone RTTTL, dan urutan nada kustom (*tone sequence*) tanpa menggunakan `delay()`.

---

## 🛠️ Fitur Utama

1. **100% Non-Blocking:** Seluruh transisi nada, durasi, dan jeda nada (*gap articulation*) diproses di latar belakang via `update()`, menjaga `loop()` utama tetap responsif.
2. **Dukungan Dua Jenis Hardware:**
   * **Passive Buzzer (`ISKAK_BUZZER_PASSIVE`):** Memainkan variasi frekuensi tangga nada ($Hz$), melodi musik, dan RTTTL.
   * **Active Buzzer (`ISKAK_BUZZER_ACTIVE`):** Memainkan pola ketukan on/off (beeping, alarm, peringatan) berbasis GPIO digital.
3. **Preset Nada Status Instan:** Siap pakai untuk suara `beep()`, `playSuccess()`, `playError()`, `playWarning()`, `playNotification()`, dan `playAlarm()`.
4. **Player Melodi RTTTL Ringtone:** Memainkan teks ringtone RTTTL (format klasik Nokia) langsung dari memori RAM maupun `PROGMEM` / Flash (`F("...")`) tanpa alokasi memori dinamis.
5. **Dukungan Framework Kernel:** Dapat didaftarkan ke `IskakINO_Kernel` via adapter `IskakINO_BuzzerModule`.

---

## 🔌 Diagram Koneksi Pin Hardware

```
[Mikrokontroler (AVR / ESP)]             [Buzzer Module]
GPIO Pin (mis. D8) --------[ Resistor ]-------- (+) VCC / Sinyal
GND ------------------------------------------- (-) GND
```
> **Catatan:** Untuk passive buzzer pada board 3.3V (ESP32/ESP8266) atau 5V (Arduino Uno), disarankan menggunakan modul buzzer dengan driver transistor bawaan (tipe KY-006 / KY-012) atau menambahkan resistor pembatas arus (100–220 $\Omega$).

---

## 💻 Contoh Penggunaan Singkat

### 1. Pola Modular Manual
```cpp
#include <IskakINO.h>

// Inisialisasi pin 8, Passive Buzzer
IskakINO_Buzzer buzzer(8, ISKAK_BUZZER_PASSIVE);

void setup() {
    buzzer.begin();
    
    // Mainkan nada sukses saat sistem selesai booting (non-blocking)
    buzzer.playSuccess();
}

void loop() {
    // WAJIB: Panggil update() di loop untuk memproses transisi nada
    buzzer.update();
    
    // Kode program Anda yang lain tetap berjalan lancar tanpa terhenti!
}
```

### 2. Memutar Melodi Ringtone RTTTL
```cpp
#include <IskakINO.h>

IskakINO_Buzzer buzzer(8);

void setup() {
    buzzer.begin();
    
    // Putar melodi Super Mario Bros dari Flash memory (PROGMEM)
    buzzer.playRTTTL(F("Mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g"));
}

void loop() {
    buzzer.update();
}
```

---

## 📖 Referensi API Publik

### Konstruktor & Inisialisasi
* `IskakINO_Buzzer(uint8_t pin = 255, IskakBuzzerType type = ISKAK_BUZZER_PASSIVE, bool activeHigh = true)`
* `void begin()`
* `void begin(uint8_t pin, IskakBuzzerType type = ISKAK_BUZZER_PASSIVE, bool activeHigh = true)`

### Kontrol Siklus & Suara
* `void update()`: Memproses state machine pemutaran nada (wajib dipanggil di `loop()`).
* `void stop()`: Menghentikan suara dan mereset antrean seketika.
* `bool isPlaying() const`: Mengembalikan `true` jika buzzer sedang berbunyi.
* `void setMute(bool mute)`: Mengaktifkan atau menonaktifkan mode hening (mute).
* `bool isMuted() const`: Mengecek apakah sistem dalam keadaan hening.

### Nada Status & Beep
* `void beep(uint16_t durationMs = 80, uint16_t frequency = 2000)`: Bunyi beep tunggal.
* `void playSuccess()`: Nada akor ceria naik (C5 - E5 - G5 - C6).
* `void playError()`: Nada rendah 3x berurutan (G3 - E3 - C3).
* `void playWarning()`: Nada peringatan 2 ketukan (A4).
* `void playNotification()`: Nada notifikasi ganda (E5 - G5).
* `void playAlarm(uint8_t repeatCount = 3)`: Sirine bergantian (A5 - E5) berulang.

### Melodi & Urutan Nada
* `void playSequence(const uint16_t* notes, const uint16_t* durationsMs, size_t count)`: Memutar array frekuensi nada kustom.
* `void playRTTTL(const char* rtttl)`: Memutar teks RTTTL dari RAM.
* `void playRTTTL(const __FlashStringHelper* rtttl)`: Memutar teks RTTTL dari `PROGMEM` Flash (`F("...")`).
