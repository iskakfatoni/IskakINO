/*
 * 17_MQTT_Telemetry.ino
 * Modul: IskakINO_MQTT & IskakINO_ArduFast (HANYA ESP32 & ESP8266)
 *
 * Menunjukkan:
 *   1. Zero-dependency MQTT v3.1.1 client (Publish & Subscribe)
 *   2. Pengiriman telemetri sensor/status periodik ke broker MQTT
 *   3. Penanganan perintah masuk (remote command) via topic callback
 *   4. Auto-reconnect dan keepalive ping non-blocking
 */

#include <IskakINO.h>

#if !defined(ISKAKINO_HAS_WIFI)
  #error "Sketsa ini hanya mendukung board ESP32 atau ESP8266."
#endif

// Konfigurasi WiFi & Broker MQTT
const char* WIFI_SSID   = "YOUR_WIFI_SSID";
const char* WIFI_PASS   = "YOUR_WIFI_PASS";
const char* MQTT_BROKER = "broker.hivemq.com";
const uint16_t MQTT_PORT = 1883;

IskakINO_ArduFast fast;
IskakINO_MQTT     mqtt;

// Callback untuk perintah yang masuk ke topik 'iskakino/cmd/led'
void onLedCommand(const char* topic, const char* payload, size_t length) {
    fast.logf(F("[MQTT Cmd] Topik: %s | Perintah: %s"), topic, payload);

    if (strcmp(payload, "ON") == 0) {
        fast.log(F("[Actuator] Menyalakan LED/Relay"));
    } else if (strcmp(payload, "OFF") == 0) {
        fast.log(F("[Actuator] Mematikan LED/Relay"));
    }
}

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - MQTT IoT Telemetry Demo    "));
    fast.log(F("========================================"));

    // Hubungkan ke WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    fast.log(F("[WiFi] Menghubungkan ke jaringan..."));

    // Inisialisasi MQTT
    mqtt.begin(MQTT_BROKER, MQTT_PORT);
    mqtt.setWill("iskakino/status", "offline", true, 0); // LWT (Last Will)
    mqtt.subscribe("iskakino/cmd/led", onLedCommand);
}

void loop() {
    // Jalankan loop MQTT untuk menangani keepalive & pesan masuk
    mqtt.tick();

    // Publish telemetri berkala setiap 5 detik saat terhubung
    if (mqtt.isConnected() && fast.every(5000, 0)) {
        static uint32_t counter = 0;
        counter++;

        char payload[64];
        snprintf(payload, sizeof(payload), "{\"uptime_ms\":%lu,\"count\":%lu}", millis(), counter);

        mqtt.publish("iskakino/telemetry", payload);
        fast.logf(F("[MQTT Pub] iskakino/telemetry -> %s"), payload);
    }
}
