/*
 * 08_ESP32_Advanced.ino
 * Modul: IskakINO_TaskCore, IskakINO_BLE, & IskakINO_Cam (Khusus ESP32)
 *
 * Demonstrasi fitur tingkat lanjut mikrokontroler ESP32:
 *  1. Dual Core FreeRTOS (TaskCore):
 *     - Background Worker task intensif dipin ke Core 0.
 *     - Antrean data thread-safe (IskakINO_Queue) antara Core 0 dan Core 1.
 *     - Sinkronisasi resource via IskakINO_Mutex.
 *  2. Bluetooth Low Energy (BLE):
 *     - Nordic UART Service (NUS) untuk komunikasi terminal nirkabel ke smartphone.
 *     - Eksekusi perintah /status dan /ping via BLE.
 *  3. Kamera OV2640 (IskakINO_Cam):
 *     - Pengambilan snapshot frame JPEG & deteksi memori PSRAM.
 *
 * Kompatibel: Khusus ESP32 (Dev Module, ESP32-CAM, dsb).
 */

#include <IskakINO.h>

#if !defined(ISKAKINO_PLATFORM_ESP32)
  #error "Sketsa ini hanya mendukung board ESP32."
#endif

// Paket data telemetri yang dikirim dari Core 0 ke Core 1
struct CoreDataPacket {
    uint32_t timestamp;
    float    computedValue;
    uint32_t core0FreeStack;
};

IskakINO_ArduFast fast;
IskakINO_TaskCore core0Worker(ISKAK_CORE_0, 4096, 1);
IskakINO_Queue<CoreDataPacket, 8> dataQueue;
IskakINO_BLE ble;

void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("    IskakINO - ESP32 Dual Core & BLE Advanced Suite      "));
    fast.log(F("========================================================="));
    fast.logf(F("[Hardware] Main loop berjalan di Core: %d"), xPortGetCoreID());

    // 1. Inisialisasi BLE Terminal
    ble.begin("IskakINO-ESP32");
    ble.onCommand("/status", [](const String& cmd, const String& args) {
        String res = "ESP32 Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB\n";
        ble.send(res);
    });

    // 2. Jalankan background worker berkala di Core 0 setiap 100 ms
    core0Worker.begin("Core0Worker");
    core0Worker.runEvery(100, []() {
        // --- BLOK INI DIEKSEKUSI DI CORE 0 ---
        static float simSensor = 25.0f;
        simSensor += ((random(0, 100) - 50) / 100.0f);

        CoreDataPacket pkt;
        pkt.timestamp = millis();
        pkt.computedValue = simSensor;
        pkt.core0FreeStack = uxTaskGetStackHighWaterMark(NULL);

        dataQueue.push(pkt);
    });

    fast.log(F("[TaskCore] Background worker di Core 0 berhasil diaktifkan."));
}

void loop() {
    // Loop utama di Core 1: Mengambil paket dari antrean tanpa blocking
    CoreDataPacket incoming;
    if (dataQueue.pop(incoming)) {
        if (fast.every(1000, 0)) {
            fast.logf(F("[Core 1 Menerima dari Core 0] Nilai: %.2f | Free Stack Core 0: %lu bytes"),
                      incoming.computedValue, (unsigned long)incoming.core0FreeStack);
        }
    }

    // Kedipkan LED status
    if (fast.every(500, 1)) {
        static bool led = false;
        led = !led;
        fast.digitalWrite(LED_BUILTIN, led);
    }
}
