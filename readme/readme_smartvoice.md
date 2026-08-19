# 🔊 Modul: IskakINO_SmartVoice

Driver pengendali modul pemutar suara MP3 DFPlayer Mini berbasis *state-machine* asinkron, dilengkapi manajemen antrean audio (*playback queue*), pengaturan volume bertahap, dan deteksi status kartu SD.

---

## 🛠️ Fitur Utama

1. **State-Machine Non-Blocking:** Mengirim instruksi ke DFPlayer Mini tanpa memblokir CPU.
2. **Antrean Pemutaran Audio (*Playback Queue*):** Memungkinkan beberapa trek audio dijadwalkan dan diputar berurutan secara otomatis.
3. **Manajemen Volume & EQ:** Mengatur tingkat volume (0 - 30) serta preset equalizer (Normal, Pop, Rock, Jazz, Classic, Bass).
4. **Pemutaran Fleksibel:** Memutar berkas berdasarkan nomor trek global (`playTrack()`) maupun berdasarkan nomor folder (`playFromFolder()`).
5. **Mode Pengumuman (*Announce*):** Memutar rekaman darurat / pengumuman penting secara instan.

---

## 💻 Contoh Penggunaan Singkat

```cpp
#include <IskakINO.h>

#if defined(__AVR__)
  #include <SoftwareSerial.h>
  SoftwareSerial mp3Serial(10, 11); // RX, TX untuk Arduino Uno/Nano
  #define MP3_STREAM mp3Serial
#else
  #define MP3_STREAM Serial2        // Hardware UART untuk ESP32
#endif

IskakINO_SmartVoice voice;

void setup() {
    Serial.begin(115200);

#if defined(__AVR__)
    mp3Serial.begin(9600);
#else
    Serial2.begin(9600, SERIAL_8N1, 16, 17);
#endif

    // Inisialisasi komunikasi dengan modul DFPlayer Mini
    voice.begin(MP3_STREAM);
    voice.setVolume(25);

    // Putar berkas MP3 nomor 1
    voice.playTrack(1);
}

void loop() {}
```

---

## 📖 Referensi API

### Inisialisasi & Pengaturan Audio
* `bool begin(Stream& serialPort)`: Inisialisasi komunikasi UART dengan DFPlayer Mini.
* `void setVolume(uint8_t volume)`: Mengatur volume suara (rentang 0 s/d 30).
* `void setEQ(uint8_t eqType)`: Mengatur profil Equalizer.
* `bool isSDCardReady()`: Memeriksa apakah kartu MicroSD terdeteksi.

### Pengendalian Pemutaran
* `void playTrack(uint16_t trackNumber)`: Memutar trek MP3 berdasarkan nomor berkas.
* `void playFromFolder(uint8_t folderNumber, uint8_t trackNumber)`: Memutar berkas di dalam folder tertentu (format folder `01`, `02`, dst).
* `void pause()`: Menjeda pemutaran yang sedang berlangsung.
* `void resume()`: Melanjutkan pemutaran yang dijeda.
* `void stop()`: Menghentikan pemutaran audio.
* `void announce(uint16_t trackNumber)`: Memutar trek prioritas / pengumuman.
* `bool isPlaying()`: Mengembalikan `true` jika modul sedang memutar suara.

---

## 📂 Penjelasan Contoh Sketsa (`examples/04_SmartVoice_PlayTrack`)

* **Lokasi Sketsa:** [`examples/04_SmartVoice_PlayTrack/04_SmartVoice_PlayTrack.ino`](../examples/04_SmartVoice_PlayTrack/04_SmartVoice_PlayTrack.ino)
* **Platform Target:** Universal (Arduino AVR via `SoftwareSerial`, ESP32 via `Serial2`).
* **Fokus Pembelajaran:**
  1. Melakukan inisialisasi jalur komunikasi serial yang kompatibel lintas board.
  2. Mengatur volume awal dan memutar beberapa trek secara berurutan.
  3. Memanfaatkan umpan balik status DFPlayer untuk memastikan audio diputar dengan sempurna.
