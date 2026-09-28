/*
 * 01_Core_Basics.ino
 * Modul: IskakINO Core & Utilities (Universal: AVR, ESP8266, ESP32)
 *
 * Demonstrasi fondasi utama sistem IskakINO:
 *  1. ArduFast        : I/O super cepat & Task Scheduler non-blocking (every/once).
 *  2. Timer           : IskakINO_Timer standalone countdown/interval timer.
 *  3. RingBuffer      : Antrean FIFO zero-heap circular buffer (IskakINO_RingBuffer).
 *  4. Diagnostic      : Monitoring kesehatan memori, RAM/Heap & reset reason (IskakINO_Diagnostic).
 *  5. BitUtils        : Endianness conversion, CRC8/16/32, & alignment-safe buffer I/O.
 *  6. Storage         : Penyimpanan struct persisten (EEPROM / Flash / LittleFS).
 *  7. JSON            : Zero-allocation JSON Builder & Type-Safe Reader.
 *
 * Kompatibel penuh: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

IskakINO_ArduFast fast;
IskakINO_Timer    heartbeatTimer;
IskakINO_RingBuffer<uint16_t, 8> dataQueue;

// Struct konfigurasi yang disimpan ke Flash/EEPROM
struct DeviceConfig {
    uint16_t deviceId;
    float    targetSetpoint;
    char     deviceName[16];
    uint32_t bootCount;
};

int configStorageAddr = -1;
DeviceConfig currentConfig;

void setup() {
    fast.begin(115200);
    fast.pinMode(LED_BUILTIN, OUTPUT);

    fast.log(F("========================================================="));
    fast.log(F("       IskakINO - Core Foundational Utilities Suite      "));
    fast.log(F("========================================================="));

    // 1. Hardware Diagnostic
    IskakINO_Diagnostic::printSummary();

    // 2. Storage Setup (Simpan & Muat Pengaturan)
    IskakStorage.begin("core_demo", true);
    configStorageAddr = IskakStorage.reserve(sizeof(DeviceConfig));

    if (IskakStorage.load(configStorageAddr, currentConfig)) {
        currentConfig.bootCount++;
        fast.logf(F("[Storage] Data ditemukan! Boot ke-%lu untuk alat: %s"),
                  (unsigned long)currentConfig.bootCount, currentConfig.deviceName);
    } else {
        fast.log(F("[Storage] Pengaturan belum ada. Menulis konfigurasi default..."));
        currentConfig.deviceId = 101;
        currentConfig.targetSetpoint = 27.5f;
        strncpy(currentConfig.deviceName, "IskakNode-01", sizeof(currentConfig.deviceName));
        currentConfig.bootCount = 1;
    }
    IskakStorage.save(configStorageAddr, currentConfig);

    // 3. BitUtils & CRC Demo
    const char* sampleText = "IskakINO-2026";
    uint32_t crcVal = IskakINO_BitUtils::crc32((const uint8_t*)sampleText, strlen(sampleText));
    uint32_t swapped = IskakINO_BitUtils::swap32(0x12345678);
    fast.logf(F("[BitUtils] CRC32('%s') = 0x%08lX | Swap32(0x12345678) = 0x%08lX"),
              sampleText, (unsigned long)crcVal, (unsigned long)swapped);

    // 4. RingBuffer Demo
    for (uint16_t i = 10; i <= 50; i += 10) {
        dataQueue.push(i);
    }
    fast.logf(F("[RingBuffer] Antrean terisi: %u item (Sisa slot: %u)"),
              dataQueue.count(), dataQueue.freeSpace());

    // 5. JSON Builder & Reader Demo
    IskakJSONBuilder builder;
    builder.beginObject();
    builder.add("node", currentConfig.deviceName);
    builder.add("id", (long)currentConfig.deviceId);
    builder.add("temp", currentConfig.targetSetpoint, 1);
    builder.add("boot", (long)currentConfig.bootCount);
    builder.endObject();

    fast.logf(F("[JSON] Output Serialized: %s"), builder.c_str());

    // Parse kembali
    IskakJSONReader reader(builder.c_str());
    if (reader.isValid()) {
        String nodeName = reader.getString("node");
        long nodeId = reader.getInt("id");
        fast.logf(F("[JSON] Parsed back -> node: %s, id: %ld"), nodeName.c_str(), nodeId);
    }

    // 6. Timer Standalone Setup
    heartbeatTimer.start(2500); // 2.5 detik
    fast.log(F("[System] Inisialisasi selesai. Scheduler berjalan...\n"));
}

void loop() {
    // Task 1: Kedip LED status tiap 500 ms (non-blocking)
    if (fast.every(500, 0)) {
        static bool ledState = false;
        ledState = !ledState;
        fast.digitalWrite(LED_BUILTIN, ledState);
    }

    // Task 2: Baca data dari RingBuffer tiap 1 detik
    if (fast.every(1000, 1)) {
        uint16_t val = 0;
        if (dataQueue.pop(val)) {
            fast.logf(F("[RingBuffer POP] Nilai: %u | Sisa elemen: %u"), val, dataQueue.count());
        }
    }

    // Task 3: Monitoring berkala lewat Timer standalone
    if (heartbeatTimer.hasExpired()) {
        fast.logf(F("[Heartbeat Timer] Free Heap: %lu bytes | Uptime: %lu s"),
                  (unsigned long)IskakINO_Diagnostic::getFreeHeap(), (unsigned long)(millis() / 1000));
        heartbeatTimer.start(3000); // Reset timer 3 detik
    }

    // Task 4: Eksekusi sekali setelah 5 detik boot
    if (fast.once(5000, 2)) {
        fast.log(F("[System] Milestone: Sistem telah stabil berjalan selama 5 detik."));
    }
}
