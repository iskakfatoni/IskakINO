/*
 * 14_Relay_PulseBlink.ino
 * Modul: IskakINO_Relay & IskakINO_ArduFast (Universal)
 *
 * Demonstrasi driver Relay & Aktuator Pintar non-blocking:
 *  - Kontrol manual ON / OFF / TOGGLE.
 *  - Auto-Off Pulse Timer: pulse(2500) menyala 2.5 detik lalu mati otomatis.
 *  - Blink Cadence: blink(300, 200, 4) berkedip ritmik 4 siklus.
 *  - Proteksi Switching Chatter: setMinSwitchInterval().
 *  - Menu interaktif Serial Monitor untuk kendali langsung aktuator.
 *
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

// Definisi Pin Relay
#if defined(ESP32)
  const uint8_t RELAY_PIN = 19;
#elif defined(ESP8266)
  const uint8_t RELAY_PIN = D5; // GPIO14
#else
  const uint8_t RELAY_PIN = 7;  // Arduino Uno / Nano D7
#endif

// Inisialisasi Objek ArduFast & Relay (Active LOW optocoupler)
IskakINO_ArduFast fast;
IskakINO_Relay relay(RELAY_PIN, ISKAK_RELAY_ACTIVE_LOW);

// State machine demo otomatis
enum RelayDemoPhase : uint8_t {
    PHASE_INTRO = 0,
    PHASE_TEST_ON,
    PHASE_TEST_OFF,
    PHASE_TEST_PULSE,
    PHASE_WAIT_PULSE,
    PHASE_TEST_BLINK,
    PHASE_WAIT_BLINK,
    PHASE_STANDBY_MENU
};

RelayDemoPhase currentPhase = PHASE_INTRO;
bool autoDemoActive = true;
uint32_t loopCounter = 0;
uint32_t lastReportMillis = 0;

void printMenu();
void handleSerialInput();
void runAutoDemo();

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("       IskakINO - Smart Relay & Actuator Showcase        "));
    fast.log(F("========================================================="));
    fast.logf(F("[Info] Pin Relay: %d (Active LOW - Glitch-Free Boot)\n"), RELAY_PIN);

    // Inisialisasi Relay (Otomatis OFF saat booting)
    relay.begin();

    // Set proteksi jeda perpindahan minimal 150ms
    relay.setMinSwitchInterval(150);

    printMenu();
    currentPhase = PHASE_INTRO;
    fast.reset(0);
}

// ============================================================================
// LOOP UTAMA (100% Non-Blocking)
// ============================================================================
void loop() {
    // 1. WAJIB: Update timer pulse & blink relay di setiap loop
    relay.update();

    // 2. Baca perintah dari Serial Monitor kapan saja
    handleSerialInput();

    // 3. Jalankan demo otomatis (jika aktif)
    if (autoDemoActive) {
        runAutoDemo();
    }

    // 4. Pembuktian Non-Blocking: Heartbeat monitor
    loopCounter++;
    if (millis() - lastReportMillis >= 3000) {
        lastReportMillis = millis();
        fast.logf(F("[Heartbeat] Uptime: %lu ms | Relay State: %s | CPU Loop: %lu iterasi/3dtk\n"),
                  (unsigned long)millis(),
                  relay.isOn() ? "ON" : "OFF",
                  (unsigned long)loopCounter);
        loopCounter = 0;
    }
}

// ============================================================================
// Logika Demo Otomatis
// ============================================================================
void runAutoDemo() {
    switch (currentPhase) {
        case PHASE_INTRO:
            if (fast.once(1500, 0)) {
                fast.reset(0);
                fast.log(F(">>> [1/3] Menguji Manual ON (Relay Menyala)..."));
                relay.on();
                currentPhase = PHASE_TEST_ON;
            }
            break;

        case PHASE_TEST_ON:
            if (fast.once(2000, 0)) {
                fast.reset(0);
                fast.log(F(">>> [1/3] Menguji Manual OFF (Relay Padam)..."));
                relay.off();
                currentPhase = PHASE_TEST_OFF;
            }
            break;

        case PHASE_TEST_OFF:
            if (fast.once(1500, 0)) {
                fast.reset(0);
                fast.log(F(">>> [2/3] Menguji Auto-Off Pulse Timer (Menyala 2.5 Detik lalu Mati Sendiri)..."));
                relay.pulse(2500);
                currentPhase = PHASE_WAIT_PULSE;
            }
            break;

        case PHASE_WAIT_PULSE:
            if (!relay.isPulsing()) {
                fast.log(F(">> Pulse Selesai: Relay padam secara otomatis tanpa delay!"));
                fast.reset(0);
                currentPhase = PHASE_TEST_BLINK;
            }
            break;

        case PHASE_TEST_BLINK:
            if (fast.once(1500, 0)) {
                fast.reset(0);
                fast.log(F(">>> [3/3] Menguji Blink Cadence (Berkedip 4 kali: ON 250ms, OFF 250ms)..."));
                relay.blink(250, 250, 4);
                currentPhase = PHASE_WAIT_BLINK;
            }
            break;

        case PHASE_WAIT_BLINK:
            if (!relay.isBlinking()) {
                fast.log(F(">> Blink Cadence Selesai!"));
                fast.log(F("\n[Selesai] Seluruh rangkaian demonstrasi relay otomatis selesai!"));
                fast.log(F("[Info] Anda sekarang dapat mencoba menu Serial Monitor di bawah.\n"));
                printMenu();
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
// Menu Bantuan & Kontrol Serial
// ============================================================================
void printMenu() {
    fast.log(F("---------------------------------------------------------"));
    fast.log(F("  Ketik karakter di Serial Monitor untuk kendali Relay:  "));
    fast.log(F("  [1] Nyalakan Relay (ON)        [p] Picu Pulse 2 Detik  "));
    fast.log(F("  [0] Matikan Relay (OFF)        [b] Blink 3x (300/200ms)"));
    fast.log(F("  [t] Balikkan Status (Toggle)   [s] Hentikan Semua (Stop)"));
    fast.log(F("  [a] Ulangi Demo Otomatis       [h] Tampilkan Menu Ini  "));
    fast.log(F("---------------------------------------------------------"));
}

void handleSerialInput() {
    while (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == '\r' || cmd == '\n' || cmd == ' ') continue;

        fast.logf(F("[Serial Cmd] Menjalankan perintah: '%c'\n"), cmd);

        switch (cmd) {
            case '1':
                relay.on();
                fast.log(F("-> Relay: ON"));
                break;

            case '0':
                relay.off();
                fast.log(F("-> Relay: OFF"));
                break;

            case 't':
            case 'T':
                relay.toggle();
                fast.logf(F("-> Relay: %s\n"), relay.isOn() ? "ON" : "OFF");
                break;

            case 'p':
            case 'P':
                fast.log(F("-> Memulai Pulse Timer 2000ms..."));
                relay.pulse(2000);
                break;

            case 'b':
            case 'B':
                fast.log(F("-> Memulai Blink Cadence 3 siklus (300ms ON / 200ms OFF)..."));
                relay.blink(300, 200, 3);
                break;

            case 's':
            case 'S':
                relay.stop();
                fast.log(F("-> Relay Dihentikan & Dipadamkan (Stop)"));
                break;

            case 'a':
            case 'A':
                autoDemoActive = true;
                currentPhase = PHASE_INTRO;
                fast.reset(0);
                fast.log(F("-> Mengulang demo otomatis dari awal..."));
                break;

            case 'h':
            case 'H':
            case '?':
                printMenu();
                break;

            default:
                fast.log(F("[?] Perintah tidak dikenal. Ketik 'h' untuk menu bantuan."));
                break;
        }
    }
}
