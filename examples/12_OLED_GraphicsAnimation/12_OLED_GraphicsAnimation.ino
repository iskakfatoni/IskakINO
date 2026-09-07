/*
 * 12_OLED_GraphicsAnimation.ino
 * Modul: IskakINO_OLED & IskakINO_ArduFast (Universal)
 *
 * Demonstrasi komprehensif driver Layar Grafis OLED I2C (SSD1306 / SH1106):
 *  - Rendering teks ukuran normal (1x) & ganda (2x).
 *  - Animasi teks mesin ketik (Typewriter) & teks berjalan (Marquee Scroll).
 *  - Dashboard IoT grafis: Ikon sinyal WiFi dinamis, baterai, & progress bar.
 *  - Efek layar: Invert display, pengaturan kontras / kecerahan.
 *  - Menu interaktif Serial Monitor untuk kendali langsung.
 *
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

// Inisialisasi Objek ArduFast (Scheduler) dan OLED (128x64 pada alamat I2C 0x3C)
IskakINO_ArduFast fast;
IskakINO_OLED oled(128, 64, 0x3C);

// ============================================================================
// State Machine Demonstrasi Otomatis
// ============================================================================
enum OLEDDemoPhase : uint8_t {
    PHASE_SPLASH = 0,
    PHASE_TYPEWRITER,
    PHASE_TYPEWRITER_WAIT,
    PHASE_MARQUEE,
    PHASE_MARQUEE_WAIT,
    PHASE_DASHBOARD_INIT,
    PHASE_DASHBOARD_RUN,
    PHASE_EFFECTS_INVERT,
    PHASE_EFFECTS_CONTRAST,
    PHASE_STANDBY_MENU
};

OLEDDemoPhase currentPhase = PHASE_SPLASH;
bool autoDemoActive = true;

uint8_t progressValue = 0;
uint8_t wifiLevel = 0;
uint8_t batteryLevel = 0;
bool displayInverted = false;
uint32_t loopCounter = 0;
uint32_t lastReportMillis = 0;

// ============================================================================
// Prototipe Fungsi
// ============================================================================
void printInteractiveMenu();
void handleSerialInput();
void runAutoDemoSequencer();
void drawIoTDashboard(uint8_t wifi, uint8_t bat, uint8_t progress);

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("     IskakINO - Advanced Non-Blocking OLED Showcase      "));
    fast.log(F("========================================================="));

    // Inisialisasi komunikasi I2C & layar OLED
    if (!oled.begin()) {
        fast.log(F("[Peringatan] Layar OLED I2C tidak terdeteksi!"));
        fast.log(F("[Tips] Periksa sambungan kabel I2C SDA/SCL atau alamat (0x3C/0x3D)."));
    } else {
        fast.logf(F("[Ready] OLED terdeteksi pada alamat I2C: 0x%02X\n"), oled.getAddress());
    }

    printInteractiveMenu();

    // Tampilkan Splash Screen Awal
    oled.clear();
    oled.setTextSize(2);
    oled.printCenter("IskakINO", 1);
    oled.setTextSize(1);
    oled.printCenter("Unified OLED Engine", 4);
    oled.printCenter("v1.1.0 (Zero-RAM)", 6);

    currentPhase = PHASE_SPLASH;
    fast.reset(0);
}

// ============================================================================
// LOOP UTAMA (100% Non-Blocking)
// ============================================================================
void loop() {
    // 1. WAJIB: Refresh state machine animasi OLED di setiap putaran loop
    oled.update();

    // 2. Baca input dari Serial Monitor kapan saja
    handleSerialInput();

    // 3. Jalankan rangkaian demo otomatis (jika aktif)
    if (autoDemoActive) {
        runAutoDemoSequencer();
    }

    // 4. Pembuktian Non-Blocking: Heartbeat monitor
    loopCounter++;
    if (millis() - lastReportMillis >= 3000) {
        lastReportMillis = millis();
        fast.logf(F("[Heartbeat] Uptime: %lu ms | CPU Loop: %lu iterasi/3dtk | Fase: %d\n"),
                  (unsigned long)millis(),
                  (unsigned long)loopCounter,
                  currentPhase);
        loopCounter = 0;
    }
}

// ============================================================================
// Logika Pengendali Demo Otomatis
// ============================================================================
void runAutoDemoSequencer() {
    switch (currentPhase) {
        case PHASE_SPLASH:
            // Tahan splash screen selama 2.5 detik
            if (fast.once(2500, 0)) {
                fast.reset(0);
                fast.log(F(">>> [1/5] Memulai Efek Mesin Ketik (Typewriter)..."));
                oled.clear();
                oled.setTextSize(1);
                oled.printCenter("--- TYPEWRITER ---", 0);
                oled.drawHLine(0, 1, 128, 0x01);
                oled.typewriterStart(F("Halo, Dunia! IskakINO OLED kini aktif tanpa delay."), 0, 3, 60);
                currentPhase = PHASE_TYPEWRITER;
            }
            break;

        case PHASE_TYPEWRITER:
            if (!oled.isTypewriterActive()) {
                currentPhase = PHASE_TYPEWRITER_WAIT;
                fast.reset(0);
            }
            break;

        case PHASE_TYPEWRITER_WAIT:
            // Jeda 2 detik setelah teks selesai diketik
            if (fast.once(2000, 0)) {
                fast.reset(0);
                fast.log(F(">>> [2/5] Memulai Teks Berjalan Panjang (Marquee Scroll)..."));
                oled.clear();
                oled.printCenter("--- MARQUEE SCROLL ---", 0);
                oled.drawHLine(0, 1, 128, 0x01);
                oled.setCursor(0, 3);
                oled.print("Status: Transmisi Data");
                oled.scrollTextStart(F("IskakINO: ArduFast + Storage + LCD + OLED + Voice + Buzzer + Shield + WiFi + NTP!"), 5, 120);
                currentPhase = PHASE_MARQUEE;
            }
            break;

        case PHASE_MARQUEE:
            // Biarkan teks bergulir selama 7 detik
            if (fast.once(7000, 0)) {
                fast.reset(0);
                oled.scrollTextStop();
                fast.log(F(">>> [3/5] Memulai Dashboard IoT Grafis & Animasi Ikon..."));
                oled.clear();
                progressValue = 0;
                wifiLevel = 0;
                batteryLevel = 0;
                currentPhase = PHASE_DASHBOARD_INIT;
            }
            break;

        case PHASE_DASHBOARD_INIT:
            drawIoTDashboard(wifiLevel, batteryLevel, progressValue);
            currentPhase = PHASE_DASHBOARD_RUN;
            fast.reset(0);
            fast.reset(1);
            break;

        case PHASE_DASHBOARD_RUN:
            // Naikkan progress bar dan ganti ikon setiap 200 ms
            if (fast.every(180, 1)) {
                progressValue += 5;
                if (progressValue % 20 == 0) {
                    wifiLevel = (wifiLevel + 1) % 5;
                    batteryLevel = (batteryLevel + 1) % 6;
                }

                drawIoTDashboard(wifiLevel, batteryLevel, progressValue);

                if (progressValue >= 100) {
                    fast.reset(0);
                    currentPhase = PHASE_EFFECTS_INVERT;
                }
            }
            break;

        case PHASE_EFFECTS_INVERT:
            if (fast.once(1500, 0)) {
                fast.reset(0);
                fast.log(F(">>> [4/5] Uji Invert Display (Warna Terbalik)..."));
                oled.invertDisplay(true);
                currentPhase = PHASE_EFFECTS_CONTRAST;
            }
            break;

        case PHASE_EFFECTS_CONTRAST:
            if (fast.once(2000, 0)) {
                fast.reset(0);
                fast.log(F(">>> [5/5] Mengembalikan Display Normal..."));
                oled.invertDisplay(false);

                fast.log(F("\n[Selesai] Seluruh rangkaian demonstrasi otomatis selesai!"));
                fast.log(F("[Info] Anda sekarang dapat mencoba tombol menu Serial di bawah ini.\n"));
                printInteractiveMenu();
                autoDemoActive = false;
                currentPhase = PHASE_STANDBY_MENU;
            }
            break;

        case PHASE_STANDBY_MENU:
        default:
            break;
    }
}

// ============================================================================
// Helper Render Dashboard IoT
// ============================================================================
void drawIoTDashboard(uint8_t wifi, uint8_t bat, uint8_t progress) {
    // Baris 0: Header & Ikon Status
    oled.setCursor(0, 0);
    oled.print("IoT Node #1");

    oled.setCursor(80, 0);
    oled.print("W:");
    oled.drawWifiIcon(wifi, 95, 0);

    oled.drawBatteryIcon(bat, 114, 0);

    // Baris 1: Garis Pemisah Header
    oled.drawHLine(0, 1, 128, 0x01);

    // Baris 2: Nilai Sensor Simulasi
    oled.setCursor(0, 2);
    oled.print("Suhu : 28.5 C ");
    oled.drawIcon(OLED_ICON_THERMO, 100, 2, 8);

    oled.setCursor(0, 3);
    oled.print("Kelemb: 65 %  ");
    oled.drawIcon(OLED_ICON_CHECK, 100, 3, 8);

    // Baris 5: Label Progress & Baris 6: Bar Grafis
    oled.setCursor(0, 5);
    oled.print("Sync Data: ");
    oled.print(progress);
    oled.print("%   ");

    oled.drawProgressBar(progress, 6, 0, 127);
}

// ============================================================================
// Menu Bantuan & Kontrol Serial Interaktif
// ============================================================================
void printInteractiveMenu() {
    fast.log(F("---------------------------------------------------------"));
    fast.log(F("  Ketik karakter di Serial Monitor untuk kendali OLED:   "));
    fast.log(F("  [1] Tampilkan Splash Screen   [6] Toggle Invert Screen "));
    fast.log(F("  [2] Uji Typewriter Text       [7] Redupkan Layar       "));
    fast.log(F("  [3] Uji Marquee Scroll        [8] Terangkan Maksimal   "));
    fast.log(F("  [4] Tampilkan Dashboard IoT   [c] Bersihkan Layar      "));
    fast.log(F("  [5] Progress Bar 100%         [a] Ulangi Demo Otomatis "));
    fast.log(F("  [h] Cetak Ulang Menu Ini                               "));
    fast.log(F("---------------------------------------------------------"));
}

void handleSerialInput() {
    while (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == '\r' || cmd == '\n' || cmd == ' ') continue;

        fast.logf(F("[Serial Cmd] Menjalankan perintah: '%c'\n"), cmd);

        switch (cmd) {
            case '1':
                oled.clear();
                oled.setTextSize(2);
                oled.printCenter("IskakINO", 1);
                oled.setTextSize(1);
                oled.printCenter("Unified OLED Engine", 4);
                oled.printCenter("v1.1.0 (Zero-RAM)", 6);
                fast.log(F("-> Tampilan Splash Screen"));
                break;

            case '2':
                oled.clear();
                oled.setTextSize(1);
                oled.printCenter("--- TYPEWRITER ---", 0);
                oled.drawHLine(0, 1, 128, 0x01);
                oled.typewriterStart(F("Mengetik teks asinkron via IskakINO_OLED..."), 0, 3, 50);
                fast.log(F("-> Memulai animasi Typewriter"));
                break;

            case '3':
                oled.clear();
                oled.setTextSize(1);
                oled.printCenter("--- MARQUEE ---", 0);
                oled.drawHLine(0, 1, 128, 0x01);
                oled.setCursor(0, 3);
                oled.print("Teks Berjalan:");
                oled.scrollTextStart(F("IskakINO Framework: Modul OLED 100% Non-Blocking & Hemat RAM!"), 5, 100);
                fast.log(F("-> Memulai animasi Marquee Scroll"));
                break;

            case '4':
                oled.clear();
                drawIoTDashboard(4, 5, 85);
                fast.log(F("-> Tampilan Dashboard IoT Statis"));
                break;

            case '5':
                oled.clear();
                oled.printCenter("--- PROGRESS BAR ---", 0);
                oled.drawHLine(0, 1, 128, 0x01);
                oled.setCursor(0, 3);
                oled.print("Memproses: 100%");
                oled.drawProgressBar(100, 5, 0, 127);
                fast.log(F("-> Menggambar Progress Bar 100%"));
                break;

            case '6':
                displayInverted = !displayInverted;
                oled.invertDisplay(displayInverted);
                fast.logf(F("-> Invert Display: %s\n"), displayInverted ? "ON (Putih)" : "OFF (Hitam)");
                break;

            case '7':
                oled.setContrast(10); // Sangat redup
                fast.log(F("-> Kontras diset ke tingkat redup (10)"));
                break;

            case '8':
                oled.setContrast(255); // Terang maksimal
                fast.log(F("-> Kontras diset ke tingkat maksimal (255)"));
                break;

            case 'c':
            case 'C':
                oled.typewriterStop();
                oled.scrollTextStop();
                oled.clear();
                fast.log(F("-> Layar dibersihkan (Clear Display)"));
                break;

            case 'a':
            case 'A':
                autoDemoActive = true;
                currentPhase = PHASE_SPLASH;
                fast.reset(0);
                fast.log(F("-> Mengulang siklus demonstrasi otomatis dari awal..."));
                break;

            case 'h':
            case 'H':
            case '?':
                printInteractiveMenu();
                break;

            default:
                fast.log(F("[?] Perintah tidak dikenal. Ketik 'h' untuk melihat menu bantuan."));
                break;
        }
    }
}
