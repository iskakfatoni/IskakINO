# 💾 Modul: IskakINO_Storage

Modul penyimpanan data non-volatile multi-backend yang secara otomatis menyesuaikan media penyimpanan berdasarkan mikrokontroler yang digunakan (EEPROM di AVR, Preferences/NVS di ESP32, dan LittleFS di ESP8266).

---

## 🛠️ Fitur Utama

1. **Multi-Backend Otomatis:** Menulis kode penyimpanan yang sama persis tanpa memikirkan perbedaan EEPROM, NVS, atau filesystem flash.
2. **Proteksi Integritas Data (CRC32):** Mencegah pemuatan data rusak (*corrupted data*) saat memori belum diinisialisasi atau mengalami kegagalan daya.
3. **Enkripsi Stream XOR Ringan:** Menjaga kerahasiaan data sensitif (seperti password WiFi atau token API).
4. **Alokasi Dinamis & String Helper:** Menyimpan teks `String` variabel panjang dengan alokasi otomatis.
5. **Ring-Buffer Logging:** Menyimpan riwayat log kejadian (*event logs*) secara melingkar tanpa khawatir memori meluap.

---

## 💻 Contoh Penggunaan Singkat

```cpp
#include <IskakINO.h>

struct ConfigData {
    uint16_t volume;
    bool enableAlarm;
};

int configAddr;

void setup() {
    Serial.begin(115200);

    // Inisialisasi storage dengan namespace / key
    IskakStorage.begin("my_app");

    // Pesan ruang memori untuk struct
    configAddr = IskakStorage.reserve(sizeof(ConfigData));

    // Simpan data
    ConfigData cfg = { 85, true };
    IskakStorage.save(configAddr, cfg);

    // Muat kembali data
    ConfigData loadedCfg;
    if (IskakStorage.load(configAddr, loadedCfg)) {
        Serial.print("Volume tersimpan: ");
        Serial.println(loadedCfg.volume);
    }
}

void loop() {}
```

---

## 📖 Referensi API

### Inisialisasi & Alokasi
* `bool begin(const char* namespaceName = "IskakINO")`: Inisialisasi storage.
* `bool beginEncrypted(const char* namespaceName, const char* key)`: Inisialisasi dengan enkripsi XOR.
* `int reserve(size_t size)`: Memesan alamat/slot memori statis.

### Baca & Tulis Data
* `template<typename T> bool save(int address, const T& data)`: Menyimpan data bertipe dasar atau `struct`.
* `template<typename T> bool load(int address, T& data)`: Memuat data tersimpan dan memvalidasi CRC32.
* `bool saveString(int address, const String& text)`: Menyimpan teks string dinamis.
* `bool loadString(int address, String& text)`: Membaca teks string dinamis.
* `bool clear()`: Menghapus dan mereset seluruh data pada namespace aktif.

### Ring-Buffer Logging
* `bool beginLog(int startAddress, uint8_t maxEntries, size_t entrySize)`: Menyiapkan ring-buffer.
* `template<typename T> bool addLog(const T& entry)`: Menambahkan satu baris data log baru.
* `template<typename T> bool readLog(uint8_t index, T& entry)`: Membaca log pada indeks tertentu.
* `uint8_t logCount()`: Mendapatkan jumlah log yang saat ini tersimpan.
* `void clearLog()`: Mengosongkan riwayat log.

---

## 📂 Penjelasan Contoh Sketsa (`examples/02_Storage_SaveLoad`)

* **Lokasi Sketsa:** [`examples/02_Storage_SaveLoad/02_Storage_SaveLoad.ino`](../examples/02_Storage_SaveLoad/02_Storage_SaveLoad.ino)
* **Platform Target:** Universal (Arduino AVR, ESP8266, ESP32).
* **Fokus Pembelajaran:**
  1. Memesan alamat memori terstruktur menggunakan `reserve()`.
  2. Menyimpan dan memuat kembali konfigurasi struct serta string.
  3. Mencatat dan membaca kembali riwayat kejadian dengan ring-buffer log tanpa risiko *memory overflow*.
  4. Menunjukkan bagaimana kode yang sama berjalan mulus di EEPROM (Arduino Uno), NVS (ESP32), dan LittleFS (NodeMCU ESP8266).
