# 🌡️ Modul: IskakINO_Sensors

Paket driver sensor populer universal berarsitektur **zero-dependency** (murni manipulasi direct I/O & mikrodetik) yang kompatibel di semua platform (**AVR**, **ESP32**, **ESP8266**).

---

## 🛠️ Sub-Driver Sensor

### 1. `IskakINO_DHT` (Sensor Suhu & Kelembapan)
* **Model yang Didukung:** DHT11, DHT22, AM2302, DHT21, AM2301.
* **Fitur Utama:**
  * Pembacaan suhu Celcius & Fahrenheit.
  * Pembacaan kelembapan relatif (% RH).
  * Perhitungan Indeks Kenyamanan Panas (*Heat Index*).
  * Validasi checksum 8-bit otomatis dan pencegahan over-polling non-blocking sesuai spesifikasi sensor (1 detik untuk DHT11, 2 detik untuk DHT22).

### 2. `IskakINO_DS18B20` (Sensor Suhu 1-Wire Waterproof)
* **Model yang Didukung:** Maxim DS18B20, DS18S20.
* **Fitur Utama:**
  * Protokol bit-banging 1-Wire mandiri tanpa library eksternal (`OneWire`/`DallasTemperature`).
  * Resolusi tinggi 12-bit (0.0625°C).
  * Verifikasi integritas data scratchpad via algoritma CRC-8 native.
  * Deteksi kehadiran fisik sensor (*Presence pulse*).

### 3. `IskakINO_Ultrasonic` (Sensor Jarak Akustik)
* **Model yang Didukung:** HC-SR04, HC-SR04P, JSN-SR04T (Waterproof), RCWL-1601.
* **Fitur Utama:**
  * Pengukuran jarak dalam satuan Centimeter, Millimeter, dan Inci.
  * **Filter Penghalus Sinyal Bawaan:** Terintegrasi langsung dengan `IskakINO_MedianFilter<5>` untuk menyingkirkan lonjakan nilai acak (*noise spikes*) akibat pantulan akustik.
  * Batas jarak maksimum yang dapat disesuaikan (default 400 cm).

---

## 🔌 Diagram Koneksi Pin Hardware

### DHT11 / DHT22:
```
[Mikrokontroler]                   [Modul DHT]
VCC (3.3V / 5V) ------------------ VCC
GND ------------------------------ GND
DATA (GPIO / Digital Pin) -------- DATA (Pasang pull-up 4.7kΩ ke VCC jika modul tanpa resistor)
```

### DS18B20 (1-Wire):
```
[Mikrokontroler]                   [Sensor DS18B20]
VCC (3.3V / 5V) ------------------ VCC (Kabel Merah)
GND ------------------------------ GND (Kabel Hitam)
DATA (GPIO / Digital Pin) -------- DATA (Kabel Kuning + Pull-up 4.7kΩ ke VCC)
```

### Ultrasonic HC-SR04:
```
[Mikrokontroler]                   [HC-SR04]
VCC (5V) ------------------------- VCC
GND ------------------------------ GND
TRIG (Digital Pin) --------------- TRIG
ECHO (Digital Pin) --------------- ECHO (Gunakan voltage divider 5V->3.3V pada ESP)
```

---

## 💻 Contoh Penggunaan Singkat

```cpp
#include <IskakINO.h>

IskakINO_DHT        dht;
IskakINO_DS18B20    ds18b20;
IskakINO_Ultrasonic sonar;

void setup() {
    Serial.begin(115200);

    dht.begin(4, IskakDHTType::DHT22);  // Pin 4, model DHT22
    ds18b20.begin(5);                  // Pin 5
    sonar.begin(12, 13);               // Trig Pin 12, Echo Pin 13
}

void loop() {
    // 1. Baca DHT22
    if (dht.read()) {
        Serial.printf("DHT -> Suhu: %.1f C, Kelembapan: %.1f %%, Heat Index: %.1f C\n",
                      dht.getTemperature(), dht.getHumidity(), dht.getHeatIndex());
    }

    // 2. Baca DS18B20
    if (ds18b20.read()) {
        Serial.printf("DS18B20 -> Suhu: %.2f C\n", ds18b20.getTemperatureC());
    }

    // 3. Baca Jarak Ultrasonic (Otomatis terfilter Median)
    float jarakCm = sonar.getDistanceCm();
    Serial.printf("Ultrasonic -> Jarak: %.1f cm\n", jarakCm);

    delay(2000);
}
```

---

## 📖 Referensi API Publik

### `IskakINO_DHT`
* `void begin(uint8_t pin, IskakDHTType type = IskakDHTType::DHT11)`: Menginisialisasi pin dan tipe sensor.
* `bool read()`: Membaca pulsa sinyal 40-bit dari sensor.
* `float getTemperature(bool inFahrenheit = false)`: Mengambil data suhu terakhir.
* `float getHumidity()`: Mengambil data kelembapan relatif terakhir.
* `float getHeatIndex(bool inFahrenheit = false)`: Menghitung indeks kenyamanan suhu tubuh.

### `IskakINO_DS18B20`
* `void begin(uint8_t pin)`: Menginisialisasi pin bus 1-Wire.
* `bool read()`: Memulai konversi suhu dan membaca scratchpad.
* `float getTemperatureC()`: Membaca suhu dalam satuan Celcius.
* `float getTemperatureF()`: Membaca suhu dalam satuan Fahrenheit.
* `bool isConnected()`: Memeriksa keberadaan sensor pada bus.

### `IskakINO_Ultrasonic`
* `void begin(uint8_t trigPin, uint8_t echoPin, uint16_t maxDistanceCm = 400)`: Mengatur pin trigger, echo, dan batas jarak.
* `float getDistanceCm(bool filtered = true)`: Mengukur jarak dalam Centimeter (default terfilter).
* `float getDistanceMm(bool filtered = true)`: Mengukur jarak dalam Millimeter.
* `float getDistanceInch(bool filtered = true)`: Mengukur jarak dalam Inci.
* `void resetFilter()`: Mengosongkan buffer median filter.
