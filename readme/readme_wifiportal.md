# 🌐 Modul: IskakINO_WifiPortal

Modul manajemen koneksi jaringan WiFi cerdas untuk **ESP32 & ESP8266**. Dilengkapi fitur *Captive Portal*, antarmuka Web UI dinamis untuk konfigurasi parameter kustom, serta OTA (*Over-The-Air*) *firmware update*.

---

## 🛠️ Fitur Utama

1. **Captive Portal Otomatis:** Jika perangkat gagal tersambung ke router, ESP otomatis menyalakan Access Point (AP) dan mengarahkan peramban (browser) pengguna langsung ke halaman konfigurasi.
2. **Parameter Kustom Dinamis (`IskakParam`):** Menyediakan formulir input teks, angka, maupun pilihan dropdown di Web UI (misalnya untuk Token MQTT, Host Server, atau API Key).
3. **Multi-WiFi Fallback:** Mampu menyimpan dan mencoba beberapa daftar kredensial SSID WiFi secara berurutan.
4. **OTA Web Firmware Update:** Mengunggah dan memperbarui berkas binary `.bin` firmware langsung melalui halaman web browser.
5. **Proteksi PIN Admin:** Mengunci halaman konfigurasi sensitif dengan kode PIN.
6. **Eksekusi Non-Blocking (`beginAsync()` & `tick()`):** Tidak memblokir alur kerja pembacaan sensor saat mencoba menyambung ke router.

---

## 💻 Contoh Penggunaan Singkat

```cpp
#include <IskakINO.h>

IskakINO_WifiPortal portal;
IskakParam mqttServer("mqtt_host", "MQTT Broker Host", "broker.hivemq.com", 64);

void setup() {
    Serial.begin(115200);

    // Tambahkan parameter kustom ke halaman web portal
    portal.addParameter(&mqttServer);

    // Aktifkan OTA update
    portal.enableOTA(true);

    // Mulai koneksi / Captive Portal secara asinkron
    portal.beginAsync("IskakINO-Device");
}

void loop() {
    // WAJIB: Panggil tick() di setiap loop untuk melayani web server & DNS
    portal.tick();

    if (portal.isConnected()) {
        // Logika saat ESP sudah terhubung ke WiFi
    }
}
```

---

## 📖 Referensi API

### Inisialisasi & Konfigurasi Jaringan
* `bool beginAsync(const char* apName, const char* apPassword = nullptr)`: Memulai proses koneksi atau AP portal secara asinkron.
* `void tick()`: Memproses request DNS, HTTP Web Server, dan auto-reconnect.
* `bool isConnected()`: Mengembalikan `true` jika perangkat terhubung ke router WiFi.
* `IskakPortalState state()`: Mengembalikan status saat ini (`IDLE`, `CONNECTING`, `CONNECTED`, `PORTAL`).

### Kustomisasi Portal & Fitur Tambahan
* `void addParameter(IskakParam* param)`: Menambahkan form input parameter ke Web UI.
* `void addWifi(const char* ssid, const char* password)`: Menambahkan kredensial WiFi manual.
* `void setBrandName(const char* name)`: Mengubah judul branding pada halaman web.
* `void setAdminPin(const char* pin)`: Mengaktifkan autentikasi PIN admin di halaman portal.
* `void enableOTA(bool enable)`: Mengaktifkan menu upload update firmware via web.
* `void resetSettings()`: Menghapus seluruh kredensial WiFi dan parameter yang tersimpan.

---

## 📂 Penjelasan Contoh Sketsa (`examples/05_WifiPortal_CaptivePortal`)

* **Lokasi Sketsa:** [`examples/05_WifiPortal_CaptivePortal/05_WifiPortal_CaptivePortal.ino`](../examples/05_WifiPortal_CaptivePortal/05_WifiPortal_CaptivePortal.ino)
* **Platform Target:** **ESP32 & ESP8266**.
* **Fokus Pembelajaran:**
  1. Menyiapkan parameter custom (`mqttServer`, `mqttPort`, `deviceLocation`) yang otomatis tersimpan di NVS/LittleFS.
  2. Mengaktifkan fitur *Captive Portal* dan OTA web update.
  3. Memperagakan penanganan status koneksi secara elegan tanpa hardcode SSID/Password di sketsa.
