/*
 * examples/25_JSON_BuildAndParse/25_JSON_BuildAndParse.ino
 *
 * Contoh penggunaan modul IskakINO_JSON (IskakJSONBuilder & IskakJSONReader).
 * Modul ini zero-dependency, ultra-hemat RAM (< 48 bytes footprint),
 * dan kompatibel penuh di semua board (Arduino AVR Uno/Nano, ESP8266, ESP32).
 *
 * Fitur yang dicontohkan:
 *   1. IskakJSONBuilder: Membuat JSON objek & array bersarang secara fluid.
 *   2. IskakJSONReader: Membaca nilai string, int, float, bool, dan array.
 *   3. Iterasi array JSON (forEach).
 */

#include <IskakINO.h>

IskakJSONBuilder builder;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Tunggu serial monitor

    Serial.println(F("\n=============================================="));
    Serial.println(F("   IskakINO_JSON: Build & Parse Demo"));
    Serial.println(F("==============================================\n"));

    // ------------------------------------------------------------------------
    // BAGIAN 1: MEMBUAT (SERIALIZE) DATA JSON DENGAN IskakJSONBuilder
    // ------------------------------------------------------------------------
    Serial.println(F("[1] Membuat Payload JSON Objek & Array..."));

    builder.clear();
    builder.beginObject();
    builder.add("device", "IskakINO-Node-01");
    builder.add("firmware", "1.1.0");
    builder.add("uptime_sec", 3600L);
    builder.add("temperature", 28.75f, 2);
    builder.add("humidity", 65.4f, 1);
    builder.add("relay_active", true);

    // Menambahkan Array Bersarang: "sensors"
    builder.beginArray("sensors");
    builder.addValue("DHT22");
    builder.addValue("DS18B20");
    builder.addValue("HC-SR04");
    builder.endArray();

    // Menambahkan Objek Bersarang Mentah: "network"
    builder.addRaw("network", "{\"ip\":\"192.168.1.50\",\"rssi\":-62}");

    builder.endObject();

    String jsonResult = builder.toString();
    Serial.println(F("Hasil JSON:"));
    Serial.println(jsonResult);
    Serial.print(F("Panjang karakter: "));
    Serial.print(builder.length());
    Serial.println(F(" bytes\n"));

    // ------------------------------------------------------------------------
    // BAGIAN 2: MEMBACA (DESERIALIZE) DATA JSON DENGAN IskakJSONReader
    // ------------------------------------------------------------------------
    Serial.println(F("[2] Membaca & Mem-parsing Payload JSON..."));

    IskakJSONReader reader(jsonResult);

    if (reader.isValid()) {
        Serial.println(F("Status: JSON Valid!"));

        // Ekstraksi nilai dengan tipe data aman (type-safe)
        String devName = reader.getString("device", "Unknown");
        String ver     = reader.getString("firmware");
        long uptime    = reader.getInt("uptime_sec");
        float temp     = reader.getFloat("temperature");
        float hum      = reader.getFloat("humidity");
        bool relay     = reader.getBool("relay_active");

        Serial.print(F("-> Device Name  : ")); Serial.println(devName);
        Serial.print(F("-> Firmware     : ")); Serial.println(ver);
        Serial.print(F("-> Uptime       : ")); Serial.print(uptime); Serial.println(F(" detik"));
        Serial.print(F("-> Suhu         : ")); Serial.print(temp, 2); Serial.println(F(" *C"));
        Serial.print(F("-> Kelembapan   : ")); Serial.print(hum, 1); Serial.println(F(" %"));
        Serial.print(F("-> Status Relay : ")); Serial.println(relay ? F("AKTIF") : F("NONAKTIF"));

        // Membaca Objek Bersarang
        String netJson = reader.getObject("network");
        IskakJSONReader netReader(netJson);
        Serial.print(F("-> Network IP   : ")); Serial.println(netReader.getString("ip"));
        Serial.print(F("-> Sinyal RSSI  : ")); Serial.print(netReader.getInt("rssi")); Serial.println(F(" dBm"));

        // Membaca Elemen Array
        size_t sensorCount = reader.getArrayLength("sensors");
        Serial.print(F("-> Total Sensor : ")); Serial.print(sensorCount); Serial.println(F(" sensor"));

        for (size_t i = 0; i < sensorCount; i++) {
            String item = reader.getArrayItem(i, "sensors");
            Serial.print(F("   [")); Serial.print(i + 1); Serial.print(F("] Sensor: "));
            Serial.println(item);
        }

        #if defined(ESP32) || defined(ESP8266)
        // Alternatif iterasi dengan lambda forEach (ESP32/ESP8266)
        Serial.println(F("-> Iterasi via forEach lambda:"));
        reader.forEach("sensors", [](size_t idx, const String& s) {
            Serial.print(F("   - (Lambda) #")); Serial.print(idx + 1); Serial.print(F(": "));
            Serial.println(s);
        });
        #endif

    } else {
        Serial.println(F("Error: Format JSON tidak valid!"));
    }

    Serial.println(F("\nDemo selesai. Menunggu loop..."));
}

void loop() {
    delay(5000);
}
