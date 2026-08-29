# 📶 Modul: IskakINO_BLE

Modul komunikasi nirkabel **Bluetooth Low Energy (BLE) UART Bridge** berbasis standar industri **Nordic UART Service (NUS)** khusus untuk platform **ESP32**.

Memungkinkan komunikasi serial dua arah berkecepatan tinggi antara mikrokontroler ESP32 dengan aplikasi smartphone (Android, iOS, dan Web Bluetooth) tanpa memerlukan pairing PIN klasik yang rumit.

---

## 🛠️ Fitur Utama

1. **Nordic UART Service (NUS) Standard:**
   * Kompatibel dengan berbagai aplikasi mobile populer: *Serial Bluetooth Terminal*, *nRF Connect*, *Adafruit Bluefruit LE*, dan *Web Bluetooth API*.
   * RX UUID: `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`
   * TX UUID: `6E400003-B5A3-F393-E0A9-E50E24DCCA9E`
2. **Telemetri & Command Dispatcher:**
   * Mengirim data telemetri ke smartphone via BLE Notify (`send()`, `sendf()`).
   * Menangani perintah interaktif dari aplikasi smartphone via pendaftaran command (`onCommand("/cmd", cb)`).
3. **Auto Re-Advertising:**
   * Otomatis mengaktifkan kembali mode BLE Advertising saat smartphone terputus, sehingga siap dihubungkan kembali sewaktu-waktu.
4. **Integrasi Kernel:**
   * Didukung oleh adapter `IskakINO_BLEModule` untuk inisialisasi dan refresh otomatis pada `IskakINO_Kernel`.

---

## 💻 Contoh Penggunaan Singkat

### 1. Pola Modular Manual
```cpp
#include <IskakINO.h>

IskakINO_BLE ble;

void handleRelay(const String& cmd, const String& args) {
    if (args == "ON") {
        ble.send("[ACK] Relay dinyalakan!");
    } else if (args == "OFF") {
        ble.send("[ACK] Relay dimatikan!");
    } else {
        ble.send("[ERR] Gunakan: /relay ON atau /relay OFF");
    }
}

void setup() {
    Serial.begin(115200);

    // Inisialisasi nama perangkat BLE yang akan muncul saat scanning
    ble.begin("IskakINO-ESP32");

    // Daftarkan handler perintah
    ble.onCommand("/relay", handleRelay);
}

void loop() {
    // WAJIB: Panggil tick() di loop untuk mengelola event status BLE
    ble.tick();

    // Kirim data telemetri berkala saat terhubung
    static uint32_t lastSend = 0;
    if (millis() - lastSend >= 2000) {
        lastSend = millis();
        if (ble.isConnected()) {
            ble.sendf("Uptime: %lu s, Heap: %u bytes", millis() / 1000, ESP.getFreeHeap());
        }
    }
}
```

### 2. Pola Framework Kernel
```cpp
#include <IskakINO.h>

IskakINO_BLE ble;
IskakINO_BLEModule bleMod(ble, "ESP32-IoT-Node");

void setup() {
    IskakINO.registerModule(&bleMod);
    IskakINO.begin();
}

void loop() {
    IskakINO.update();
}
```

---

## 📖 Referensi API Publik

### Inisialisasi & Status
* `bool begin(const char* deviceName = "IskakINO-BLE")`: Menginisialisasi stack BLE ESP32, membuat service NUS, dan memulai advertising.
* `bool isConnected() const`: Memeriksa apakah ada perangkat client/smartphone yang sedang terhubung.

### Pengiriman Data
* `void send(const char* text)`: Mengirim teks ke smartphone via notifikasi TX characteristic.
* `void send(const String& text)`: Mengirim objek `String`.
* `void sendf(const char* format, ...)`: Mengirim format teks printf-style.

### Penanganan Perintah & Data Masuk
* `void onData(BLEDataCallback callback)`: Menangani seluruh data teks mentah yang dikirim oleh smartphone.
* `void onCommand(const char* cmd, BLECmdCallback callback)`: Mendaftarkan fungsi penanganan perintah berbasis kata kunci (misal: `"/status"` atau `"/relay"`).
* `void tick()`: Memproses status koneksi dan auto re-advertising saat terputus.
