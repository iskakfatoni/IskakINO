# 📦 Modul: IskakINO_JSON

Modul serialisasi (*building*) dan deserialisasi (*parsing*) JSON berarsitektur **Zero-Dependency** dan **Zero-Copy Tokenizer** yang dirancang khusus untuk mikrokontroler dengan memori RAM terbatas (**Arduino AVR Uno/Nano 2KB RAM**, **ESP8266**, dan **ESP32**).

---

## 🛠️ Fitur Utama

1. **Zero External Dependency:** Tidak memerlukan instalasi library pihak ketiga (seperti `ArduinoJson`). Cukup sertakan `#include <IskakINO.h>`.
2. **Ultra-Hemat Memori:** Memory footprint < 48 Bytes RAM dan < 2.5 KB Flash.
3. **Bebas Fragmentasi Heap (Zero-Copy Tokenizer):** Mem-parsing nilai JSON secara langsung menggunakan pointer string tanpa mengalokasikan pohon DOM dinamis di heap RAM.
4. **Fluid Stream Builder (`IskakJSONBuilder`):** Menyusun payload JSON objek dan array bersarang dengan sintaks fluent chaining yang intuitif.
5. **Direct Stream Output:** Dapat menuliskan output JSON langsung ke stream (`Serial`, `WiFiClient`, `Print&`) tanpa alokasi buffer ganda.
6. **Ekstraksi Type-Safe (`IskakJSONReader`):** Mendukung pengambilan nilai tipe data `String`, `long`, `int`, `float`, `bool`, `getObject()`, `getArray()`, dan iterasi `forEach()`.

---

## 💻 Contoh Penggunaan Singkat

### 1. Membuat (Serialize) JSON dengan `IskakJSONBuilder`

```cpp
#include <IskakINO.h>

IskakJSONBuilder json;

void kirimData() {
    json.clear();
    json.beginObject();
    json.add("device", "ESP32-Node");
    json.add("uptime", millis() / 1000);
    json.add("temp", 28.5f, 1);
    json.add("relay_active", true);

    json.beginArray("sensors");
    json.addValue("DHT22");
    json.addValue("DS18B20");
    json.endArray();

    json.endObject();

    // Output: {"device":"ESP32-Node","uptime":120,"temp":28.5,"relay_active":true,"sensors":["DHT22","DS18B20"]}
    Serial.println(json.toString());
}
```

---

### 2. Membaca (Parse) JSON dengan `IskakJSONReader`

```cpp
#include <IskakINO.h>

void prosesPayload(const String& payload) {
    IskakJSONReader reader(payload);

    if (reader.isValid()) {
        String dev   = reader.getString("device", "Unknown");
        long uptime  = reader.getInt("uptime");
        float temp   = reader.getFloat("temp");
        bool relay   = reader.getBool("relay_active");

        Serial.printf("Device: %s | Temp: %.1f | Relay: %s\n", dev.c_str(), temp, relay ? "ON" : "OFF");

        // Iterasi Array
        reader.forEach("sensors", [](size_t idx, const String& sensorName) {
            Serial.printf("Sensor #%u: %s\n", (unsigned int)(idx + 1), sensorName.c_str());
        });
    }
}
```

---

## 📖 Referensi API Publik

### Kelas `IskakJSONBuilder`
* `void clear()`: Mengosongkan buffer JSON.
* `void beginObject()` / `void endObject()`: Membuka dan menutup struktur objek `{}`.
* `void beginArray(const char* key = nullptr)` / `void endArray()`: Membuka dan menutup struktur array `[]`.
* `void add(const char* key, const char* / String / long / float / bool value)`: Menambahkan key-value ke objek.
* `void addValue(const char* / String / long / float / bool value)`: Menambahkan item ke array.
* `void addRaw(const char* key, const char* rawJson)`: Menambahkan subtree JSON mentah ke objek.
* `String toString() const`: Mengembalikan string representasi JSON.
* `size_t length() const`: Mengembalikan panjang karakter JSON.
* `void writeTo(Print& out) const`: Menuliskan isi JSON langsung ke objek Stream/Print.
* `static String escape(const String& raw)`: Helper untuk meng-escape karakter khusus (`"`, `\`, newline, tab).

### Kelas `IskakJSONReader`
* `IskakJSONReader(const String& json)`: Konstruktor dengan string JSON input.
* `void setSource(const String& json)`: Menentukan string sumber JSON yang akan dibaca.
* `bool isValid() const`: Memeriksa apakah format JSON diawali dan diakhiri dengan `{}` atau `[]`.
* `bool hasKey(const char* key) const`: Memeriksa apakah kunci tertentu ada di dalam JSON.
* `String getString(const char* key, const String& defaultVal = "") const`: Mengambil nilai string.
* `long getInt(const char* key, long defaultVal = 0) const`: Mengambil nilai integer/long.
* `float getFloat(const char* key, float defaultVal = 0.0f) const`: Mengambil nilai float.
* `bool getBool(const char* key, bool defaultVal = false) const`: Mengambil nilai boolean (`true`/`false`/`1`/`0`).
* `String getObject(const char* key) const`: Mengambil subtree objek JSON bertingkat.
* `String getArray(const char* key) const`: Mengambil subtree array JSON.
* `size_t getArrayLength(const char* arrayKey = nullptr) const`: Menghitung total elemen di dalam array.
* `String getArrayItem(size_t index, const char* arrayKey = nullptr) const`: Mengambil elemen array pada indeks tertentu.
* `void forEach(const char* arrayKey, callback)`: Iterasi seluruh elemen array (*ESP32/ESP8266*).
* `static String unescape(const String& raw)`: Helper untuk mengembalikan karakter escape ke teks asli.

---

## 📂 Penjelasan Contoh Sketsa (`examples/25_JSON_BuildAndParse`)

* **Lokasi Sketsa:** [`examples/25_JSON_BuildAndParse/25_JSON_BuildAndParse.ino`](../examples/25_JSON_BuildAndParse/25_JSON_BuildAndParse.ino)
* **Platform Target:** **AVR Uno/Nano, ESP8266, & ESP32**.
* **Fokus Pembelajaran:**
  1. Cara menyusun objek dan array bertingkat dengan `IskakJSONBuilder`.
  2. Cara mem-parsing data JSON yang diterima dari jaringan/serial secara aman.
  3. Membaca array dan objek bertingkat tanpa risiko memori bocor (*leak*) atau fragmentasi heap.
