/*
 * 22_Sensors_DS18B20_1Wire.ino
 * Modul: IskakINO_DS18B20 & IskakINO_ArduFast (UNIVERSAL: AVR, ESP32, ESP8266)
 *
 * Menunjukkan:
 *   1. Pembacaan sensor suhu waterproof Maxim DS18B20 1-Wire zero-dependency
 *   2. Validasi CRC-8 internal pada scratchpad data
 *   3. Pembacaan suhu presisi tinggi Celcius dan Fahrenheit
 */

#include <IskakINO.h>

#define ONE_WIRE_PIN 2 // Pin GPIO yang terhubung ke kabel data DS18B20 (perlu resistor pullup 4.7k)

IskakINO_ArduFast fast;
IskakINO_DS18B20  tempSensor;

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - DS18B20 1-Wire Demo        "));
    fast.log(F("========================================"));

    // Inisialisasi pin 1-Wire
    tempSensor.begin(ONE_WIRE_PIN);

    if (tempSensor.isConnected()) {
        fast.log(F("[DS18B20] Sensor 1-Wire terdeteksi dan merespon presence pulse."));
    } else {
        fast.log(F("[DS18B20] Peringatan: Sensor tidak merespon. Pastikan resistor pull-up 4.7k terpasang."));
    }
}

void loop() {
    // Baca suhu setiap 2 detik non-blocking
    if (fast.every(2000, 0)) {
        if (tempSensor.read()) {
            float tempC = tempSensor.getTemperatureC();
            float tempF = tempSensor.getTemperatureF();

            fast.logf(F("[Suhu DS18B20] %.2f °C | %.2f °F"), tempC, tempF);
        } else {
            fast.log(F("[Error] Gagal membaca suhu DS18B20!"));
        }
    }
}
