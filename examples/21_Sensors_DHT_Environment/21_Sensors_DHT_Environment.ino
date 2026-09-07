/*
 * 21_Sensors_DHT_Environment.ino
 * Modul: IskakINO_DHT & IskakINO_ArduFast (UNIVERSAL: AVR, ESP32, ESP8266)
 *
 * Menunjukkan:
 *   1. Pembacaan suhu dan kelembapan dari DHT11 / DHT22 zero-dependency
 *   2. Perhitungan Heat Index (indeks kenyamanan termal manusia)
 *   3. Polling non-blocking dengan interval aman
 */

#include <IskakINO.h>

#define DHT_PIN 4 // Sesuaikan pin GPIO (mis. Pin D4 di Arduino / GPIO4 di ESP)

IskakINO_ArduFast fast;
IskakINO_DHT      dht;

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - DHT Environment Sensor     "));
    fast.log(F("========================================"));

    // Inisialisasi DHT11 (Ganti ke IskakDHTType::DHT22 jika menggunakan DHT22/AM2302)
    dht.begin(DHT_PIN, IskakDHTType::DHT11);
}

void loop() {
    // Baca sensor setiap 2 detik non-blocking
    if (fast.every(2000, 0)) {
        if (dht.read()) {
            float temp = dht.getTemperature();
            float hum = dht.getHumidity();
            float heatIdx = dht.getHeatIndex();

            fast.logf(F("[Sensor] Suhu: %.1f °C | Kelembapan: %.1f %% | Heat Index: %.1f °C"),
                       temp, hum, heatIdx);
        } else {
            fast.log(F("[Error] Gagal membaca sensor DHT! Cek koneksi pin."));
        }
    }
}
