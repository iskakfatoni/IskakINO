/*
 * 16_ESP32Cam_SnapshotStream.ino
 * Modul: IskakINO_Cam & IskakINO_ArduFast (Khusus ESP32 / ESP32-CAM)
 *
 * Demonstrasi driver modul kamera ESP32 (ESP32-CAM AI-Thinker OV2640):
 *  - Inisialisasi instan tanpa konfigurasi 16 pin manual.
 *  - Deteksi ketersediaan modul PSRAM eksternal.
 *  - Pengambilan snapshot foto JPEG (Capture & Release frame buffer).
 *  - Kontrol lampu Flash LED onboard (GPIO 4) dengan pulse timer non-blocking.
 *  - Penyesuaian sensor dinamis (resolusi, flip vertikal, mirror horizontal).
 *  - Menu interaktif Serial Monitor.
 *
 * Catatan: Sketsa ini khusus untuk mikrokontroler keluarga ESP32 (misal ESP32-CAM).
 */

#include <IskakINO.h>

#if defined(ESP32)

// Inisialisasi Objek ArduFast & Kamera (Lampu Flash Blitz pada GPIO 4)
IskakINO_ArduFast fast;
IskakINO_Cam cam(4);

// Status sensor
bool vflipState = false;
bool hmirrorState = false;
uint32_t photoCounter = 0;
uint32_t loopCounter = 0;
uint32_t lastReportMillis = 0;

void printMenu();
void handleSerialInput();
void takePhoto();

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("       IskakINO - ESP32-CAM Snapshot & Control Demo      "));
    fast.log(F("========================================================="));

    // Inisialisasi Kamera ESP32-CAM AI-Thinker (Resolusi VGA 640x480, JPEG Quality 12)
    if (!cam.begin(CAM_MODEL_AI_THINKER, FRAMESIZE_VGA, 12)) {
        fast.log(F("[Error] Gagal menginisialisasi modul kamera!"));
        fast.log(F("[Tips] Pastikan modul kamera OV2640 terpasang rapat pada soket FPC."));
        return;
    }

    fast.log(F("[Ready] Modul kamera OV2640 berhasil diinisialisasi!"));
    fast.logf(F("[Hardware] PSRAM: %s\n"), cam.hasPSRAM() ? "TERDETEKSI (Mendukung Resolusi Tinggi)" : "TIDAK TERDETEKSI (Mode DRAM Standar)");

    printMenu();
    fast.reset(0);
}

// ============================================================================
// LOOP UTAMA (100% Non-Blocking)
// ============================================================================
void loop() {
    // 1. Update state machine kamera (misal timer pulse flash)
    cam.update();

    // 2. Baca perintah dari Serial Monitor kapan saja
    handleSerialInput();

    // 3. Demonstrasi Periodik: Ambil foto setiap 6 detik secara otomatis
    if (fast.every(6000, 0)) {
        fast.log(F(">>> [Periodic Auto Snapshot] Mengambil foto otomatis..."));
        takePhoto();
    }

    // 4. Pembuktian Non-Blocking: Heartbeat monitor
    loopCounter++;
    if (millis() - lastReportMillis >= 3000) {
        lastReportMillis = millis();
        fast.logf(F("[Heartbeat] Uptime: %lu ms | Total Foto Diambil: %lu | CPU Loop: %lu iterasi/3dtk\n"),
                  (unsigned long)millis(),
                  (unsigned long)photoCounter,
                  (unsigned long)loopCounter);
        loopCounter = 0;
    }
}

// ============================================================================
// Fungsi Pengambilan Foto (Capture & Release)
// ============================================================================
void takePhoto() {
    if (!cam.isInitialized()) {
        fast.log(F("[Error] Kamera belum siap!"));
        return;
    }

    // Kedipkan blitz flash sejenak (100ms)
    cam.flashPulse(100);

    // Tangkap 1 frame buffer
    camera_fb_t* fb = cam.capture();
    if (!fb) {
        fast.log(F("[Error] Gagal menangkap frame gambar!"));
        return;
    }

    photoCounter++;
    fast.logf(F("[Capture Sukses #%lu] Ukuran JPEG: %u bytes | Dimensi: %ux%u px | Alamat Buffer: %p\n"),
              (unsigned long)photoCounter,
              (unsigned int)fb->len,
              (unsigned int)fb->width,
              (unsigned int)fb->height,
              (void*)fb->buf);

    // WAJIB: Selalu kembalikan buffer frame setelah selesai agar memori tidak bocor!
    cam.release(fb);
}

// ============================================================================
// Menu Bantuan & Kontrol Serial
// ============================================================================
void printMenu() {
    fast.log(F("---------------------------------------------------------"));
    fast.log(F("  Ketik karakter di Serial Monitor untuk kendali Kamera: "));
    fast.log(F("  [s] Ambil Foto Snapshot        [f] Flash Blitz 150ms   "));
    fast.log(F("  [1] Set Resolusi QVGA (320x240)[2] Set Resolusi VGA   "));
    fast.log(F("  [3] Set Resolusi SVGA (800x600)[4] Set Resolusi HD    "));
    fast.log(F("  [v] Toggle Flip Vertikal       [h] Toggle Mirror Horiz "));
    fast.log(F("  [m] Tampilkan Menu Ini                                 "));
    fast.log(F("---------------------------------------------------------"));
}

void handleSerialInput() {
    while (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == '\r' || cmd == '\n' || cmd == ' ') continue;

        fast.logf(F("[Serial Cmd] Menjalankan perintah: '%c'\n"), cmd);

        switch (cmd) {
            case 's':
            case 'S':
                takePhoto();
                break;

            case 'f':
            case 'F':
                cam.flashPulse(150);
                fast.log(F("-> Lampu Flash dipicu selama 150ms"));
                break;

            case '1':
                cam.setResolution(FRAMESIZE_QVGA);
                fast.log(F("-> Resolusi diubah ke: QVGA (320x240)"));
                break;

            case '2':
                cam.setResolution(FRAMESIZE_VGA);
                fast.log(F("-> Resolusi diubah ke: VGA (640x480)"));
                break;

            case '3':
                cam.setResolution(FRAMESIZE_SVGA);
                fast.log(F("-> Resolusi diubah ke: SVGA (800x600)"));
                break;

            case '4':
                cam.setResolution(FRAMESIZE_HD);
                fast.log(F("-> Resolusi diubah ke: HD (1280x720)"));
                break;

            case 'v':
            case 'V':
                vflipState = !vflipState;
                cam.setVFlip(vflipState);
                fast.logf(F("-> Vertical Flip: %s\n"), vflipState ? "ON" : "OFF");
                break;

            case 'h':
            case 'H':
                hmirrorState = !hmirrorState;
                cam.setHMirror(hmirrorState);
                fast.logf(F("-> Horizontal Mirror: %s\n"), hmirrorState ? "ON" : "OFF");
                break;

            case 'm':
            case 'M':
            case '?':
                printMenu();
                break;

            default:
                fast.log(F("[?] Perintah tidak dikenal. Ketik 'm' untuk menu bantuan."));
                break;
        }
    }
}

#else

// Fallback jika dikompilasi di luar board ESP32
void setup() {
    Serial.begin(115200);
    Serial.println(F("[Peringatan] Contoh ini dirancang khusus untuk platform ESP32 (mis. ESP32-CAM)."));
}

void loop() {
}

#endif // defined(ESP32)
