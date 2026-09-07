/*
 * examples/26_DualCore_TaskManager/26_DualCore_TaskManager.ino
 *
 * Demonstrasi pemanfaatan Dual-Core FreeRTOS pada ESP32 menggunakan IskakINO_TaskCore.
 *
 * Fitur yang dicontohkan:
 *   1. IskakINO_TaskCore: Menjalankan task intensif di Core 0 secara non-blocking.
 *   2. IskakINO_Queue: Komunikasi antrean data thread-safe dari Core 0 ke Core 1.
 *   3. IskakINO_Mutex & IskakINO_LockGuard: Sinkronisasi variabel bersama tanpa race condition.
 *   4. Main Loop (Core 1): Melayani UI, serial monitor, dan tombol dengan latensi 0 ms.
 */

#include <IskakINO.h>

// Struktur data telemetri yang dikirim dari Core 0 ke Core 1
struct TelemetryPacket {
    uint32_t timestamp;
    float    filteredVoltage;
    float    computedPower;
    uint32_t freeStackCore0;
};

// 1. Antrean data inter-core thread-safe (Kapasitas: 10 paket)
IskakINO_Queue<TelemetryPacket, 10> telemetryQueue;

// 2. Mutex untuk melindungi resource bersama
IskakINO_Mutex dataMutex;
uint32_t totalProcessedFrames = 0;

// 3. Worker Task terdedikasi di Core 0 (Stack 4096 bytes, Prioritas 1)
IskakINO_TaskCore backgroundWorker(ISKAK_CORE_0, 4096, 1);

// Filter sinyal Kalman untuk simulasi komputasi di Core 0
IskakINO_Kalman1D kalman(0.01f, 0.1f, 1.0f);

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    Serial.println(F("\n========================================================"));
    Serial.println(F("   IskakINO Dual-Core FreeRTOS Task Manager Demo"));
    Serial.println(F("========================================================\n"));

    #if defined(ESP32)
    Serial.print(F("[INFO] Main setup() berjalan di Core ID: "));
    Serial.println(xPortGetCoreID());
    #endif

    // Inisialisasi dan jalankan task berkala di Core 0 setiap 50 ms
    backgroundWorker.begin("WorkerCore0");
    backgroundWorker.runEvery(50, []() {
        // --- BLOK INI DIEKSEKUSI DI CORE 0 ---
        static float rawVal = 12.0f;
        rawVal += ((random(0, 100) - 50) / 100.0f); // Simulasi noise ADC

        float smoothed = kalman.update(rawVal);
        float power = smoothed * 2.5f;

        // Kirim paket hasil komputasi ke antrean Core 1
        TelemetryPacket packet;
        packet.timestamp = millis();
        packet.filteredVoltage = smoothed;
        packet.computedPower = power;
        packet.freeStackCore0 = backgroundWorker.getFreeStack();

        telemetryQueue.push(packet);

        // Update counter dengan proteksi mutex thread-safe
        {
            IskakINO_LockGuard guard(dataMutex);
            totalProcessedFrames++;
        }
    });

    Serial.println(F("[OK] Worker task berhasil dimulai di Core 0!"));
    Serial.println(F("[OK] Core 1 siap melayani main loop() tanpa lag.\n"));
}

void loop() {
    // --- BLOK INI DIEKSEKUSI DI CORE 1 ---
    TelemetryPacket rxPacket;

    // Ambil data yang dikirim dari Core 0 jika tersedia
    if (telemetryQueue.pop(rxPacket)) {
        static uint32_t lastPrint = 0;
        if (millis() - lastPrint >= 500) { // Cetak setiap 500 ms
            lastPrint = millis();

            uint32_t framesCount = 0;
            {
                IskakINO_LockGuard guard(dataMutex);
                framesCount = totalProcessedFrames;
            }

            Serial.print(F("[Core 1 RX] Time: "));
            Serial.print(rxPacket.timestamp);
            Serial.print(F(" ms | Volt: "));
            Serial.print(rxPacket.filteredVoltage, 2);
            Serial.print(F(" V | Power: "));
            Serial.print(rxPacket.computedPower, 2);
            Serial.print(F(" W | Total Frames: "));
            Serial.print(framesCount);
            Serial.print(F(" | Stack Sisa Core 0: "));
            Serial.print(rxPacket.freeStackCore0);
            Serial.println(F(" bytes"));
        }
    }

    // Simulasi interaksi pengguna yang instan tanpa jeda
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'p' || c == 'P') {
            backgroundWorker.pause();
            Serial.println(F("\n>>> [COMMAND] Worker Core 0 di-PAUSE."));
        } else if (c == 'r' || c == 'R') {
            backgroundWorker.resume();
            Serial.println(F("\n>>> [COMMAND] Worker Core 0 di-RESUME."));
        } else if (c == 's' || c == 'S') {
            Serial.print(F("\n>>> [STATUS] Worker Core 0 Running: "));
            Serial.print(backgroundWorker.isRunning() ? F("YES") : F("NO"));
            Serial.print(F(" | Paused: "));
            Serial.println(backgroundWorker.isPaused() ? F("YES") : F("NO"));
        }
    }
}
