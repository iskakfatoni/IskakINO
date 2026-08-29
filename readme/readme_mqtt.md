# 📡 Modul: IskakINO_MQTT

Client protokol **MQTT v3.1.1** mandiri (*zero-dependency*) khusus untuk platform mikrokontroler WiFi (**ESP32** & **ESP8266**).

Dirancang langsung di atas socket `WiFiClient` tanpa memerlukan pustaka pihak ketiga seperti `PubSubClient`, hemat RAM, serta terintegrasi mulus dengan *logger*, *scheduler*, dan arsitektur *Kernel* framework IskakINO.

---

## 🛠️ Fitur Utama

1. **Zero-Dependency Native Client:**
   * Implementasi paket MQTT v3.1.1 biner murni (CONNECT, CONNACK, PUBLISH, PUBACK, SUBSCRIBE, SUBACK, PINGREQ, PINGRESP, DISCONNECT) tanpa dependensi eksternal.
2. **Publish & Subscribe:**
   * Publish data string, biner, ataupun JSON payload dengan opsi retain.
   * Subscribe hingga 8 topik individual dengan fungsi callback spesifik per-topik atau global message handler.
3. **Auto-Reconnect & Keepalive Non-Blocking:**
   * Loop auto-reconnect cerdas saat sambungan broker terputus dengan interval yang dapat diatur.
   * Keepalive ping periodik otomatis (default 15 detik) untuk mencegah timeout koneksi.
4. **Last Will and Testament (LWT):**
   * Pengaturan pesan wasiat otomatis saat perangkat mati mendadak (*ungraceful disconnect*).
5. **Dukungan Autentikasi:**
   * Kredensial username dan password broker MQTT.

---

## 💻 Contoh Penggunaan Singkat

### 1. Pola Modular Manual
```cpp
#include <IskakINO.h>

IskakINO_MQTT mqtt;

void onRelayCommand(const char* topic, const char* payload, size_t length) {
    Serial.printf("Perintah diterima [%s]: %s\n", topic, payload);
}

void setup() {
    Serial.begin(115200);
    WiFi.begin("SSID_WIFI", "PASSWORD_WIFI");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    // Inisialisasi broker: host, port, clientId, user, pass
    mqtt.begin("broker.hivemq.com", 1883, "ESP32-IskakINO-Client");

    // Pasang pesan Last Will (LWT)
    mqtt.setWill("iskakino/device/status", "offline", true);

    // Hubungkan ke broker
    if (mqtt.connect()) {
        Serial.println("Terhubung ke MQTT Broker!");
        mqtt.publish("iskakino/device/status", "online", true);

        // Berlangganan topik perintah
        mqtt.subscribe("iskakino/device/cmd/relay", onRelayCommand);
    }
}

void loop() {
    // WAJIB: Panggil tick() di loop untuk menjaga koneksi dan memproses pesan masuk
    mqtt.tick();

    // Kirim data telemetri berkala setiap 5 detik
    static uint32_t lastPub = 0;
    if (millis() - lastPub >= 5000) {
        lastPub = millis();
        if (mqtt.isConnected()) {
            mqtt.publish("iskakino/sensor/temp", "28.5");
        }
    }
}
```

### 2. Pola Framework Kernel
```cpp
#include <IskakINO.h>

IskakINO_MQTT mqtt;
IskakINO_MQTTModule mqttMod(mqtt, "broker.emqx.io", 1883, "ESP-Kernel-Node");

void setup() {
    IskakINO.registerModule(&mqttMod);
    IskakINO.begin();
}

void loop() {
    IskakINO.update();
}
```

---

## 📖 Referensi API Publik

### Inisialisasi & Koneksi
* `void begin(const char* broker, uint16_t port = 1883, const char* clientId = nullptr, const char* username = nullptr, const char* password = nullptr)`: Konfigurasi host broker, port, dan kredensial.
* `bool connect()`: Membuka koneksi TCP dan mengirim paket CONNECT MQTT.
* `void disconnect()`: Mengirim paket DISCONNECT dan menutup soket.
* `bool isConnected()`: Memeriksa apakah status koneksi aktif.
* `IskakMQTTState state()`: Mengembalikan status koneksi (`DISCONNECTED`, `CONNECTING`, `CONNECTED`).

### Publish & Subscribe
* `bool publish(const char* topic, const char* payload, bool retain = false, uint8_t qos = 0)`: Mengirim pesan teks string ke topik tertentu.
* `bool publish(const char* topic, const uint8_t* payload, size_t length, bool retain = false, uint8_t qos = 0)`: Mengirim buffer payload biner.
* `bool subscribe(const char* topic, IskakMQTTCallback callback = nullptr, uint8_t qos = 0)`: Berlangganan topik tertentu dengan fungsi callback.
* `bool unsubscribe(const char* topic)`: Berhenti berlangganan dari topik.
* `void onMessage(IskakMQTTCallback callback)`: Menyetel callback global untuk semua topik masuk yang belum ditangani secara khusus.

### Konfigurasi & Maintenance
* `void setWill(const char* topic, const char* message, bool retain = false, uint8_t qos = 0)`: Mengatur Last Will and Testament (LWT).
* `void setKeepAlive(uint16_t seconds)`: Mengatur interval keepalive ping.
* `void setAutoReconnect(bool autoReconnect, uint32_t intervalMs = 5000)`: Mengaktifkan/menonaktifkan reconnect otomatis saat putus.
* `void tick()`: Memproses polling TCP socket, parsing paket masuk, dan timer keepalive.
