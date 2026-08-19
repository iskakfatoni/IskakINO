/*
 * 11_Buzzer_MelodyBeep.ino
 * Modul: IskakINO_Buzzer & IskakINO_ArduFast (Universal)
 *
 * Demonstrasi komprehensif driver Buzzer Non-Blocking:
 *  - Uji nada status sistem (Success, Error, Warning, Notification, Alarm).
 *  - Uji pemutaran melodi musik RTTTL (Nokia Tune, Super Mario, Star Wars).
 *  - Uji multi-tasking sejati (LED blink cepat & background math tanpa delay).
 *  - Menu interaktif Serial Monitor (bisa kirim '1'-'9', 's', 'm', 'h' kapan saja).
 *
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

// ============================================================================
// Konfigurasi Pin Buzzer & LED (Otomatis menyesuaikan target platform)
// ============================================================================
#if defined(ISKAKINO_PLATFORM_AVR)
  #define BUZZER_PIN    8   // Pin D8 di Arduino Uno / Nano
  #define STATUS_LED   13   // LED bawaan Arduino Uno (D13)
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #define BUZZER_PIN   14   // Pin D5 (GPIO14) di NodeMCU ESP8266
  #define STATUS_LED    2   // LED bawaan ESP8266 (D4 / GPIO2)
#elif defined(ISKAKINO_PLATFORM_ESP32)
  #define BUZZER_PIN   18   // Pin GPIO18 di ESP32 Dev Module
  #define STATUS_LED    2   // LED bawaan ESP32 (GPIO2)
#else
  #define BUZZER_PIN    8
  #define STATUS_LED    2
#endif

// Inisialisasi Objek ArduFast (Scheduler & Direct I/O) dan Buzzer (Passive)
IskakINO_ArduFast fast;
IskakINO_Buzzer buzzer(BUZZER_PIN, ISKAK_BUZZER_PASSIVE);

// FastPin untuk indikator LED visual
FastPin<STATUS_LED> led;

// ============================================================================
// Daftar Melodi Format RTTTL (Disimpan di Flash Memory PROGMEM)
// ============================================================================
// 1. Nokia Tune Klasik
const char MELODY_NOKIA[] PROGMEM =
    "NokiaTune:d=4,o=5,b=225:8e6,8d6,f#,g#,8c#6,8b,d,e,8b,8a,c#,e,2a";

// 2. Super Mario Bros Overworld Theme
const char MELODY_MARIO[] PROGMEM =
    "Mario:d=4,o=5,b=100:16e6,16e6,32p,8e6,16c6,8e6,8g6,8p,8g,8p,8c6,16p,8g,16p,8e,16p,8a,8b,16a#,8a,16g,16e6,16g6,8a6,16f6,8g6,8e6,16c6,16d6,8b";

// 3. Star Wars - The Imperial March (Darth Vader Theme)
const char MELODY_STARWARS[] PROGMEM =
    "Imperial:d=4,o=5,b=108:4a,4a,4a,8f,16c6,4a,8f,16c6,2a,4e6,4e6,4e6,8f6,16c6,4g#,8f,16c6,2a";

// Custom sequence arpeggio (C Major 7 Chord)
const uint16_t ARPEGGIO_NOTES[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_B4, NOTE_C5, NOTE_B4, NOTE_G4, NOTE_E4, NOTE_C4 };
const uint16_t ARPEGGIO_DURS[]  = { 100,     100,     100,     100,     250,     100,     100,     100,     300 };
const size_t   ARPEGGIO_COUNT   = sizeof(ARPEGGIO_NOTES) / sizeof(ARPEGGIO_NOTES[0]);

// ============================================================================
// State Machine Demonstrasi Otomatis
// ============================================================================
enum DemoPhase : uint8_t {
    PHASE_STARTUP = 0,
    PHASE_BEEP_TEST,
    PHASE_CHIMES_SUCCESS,
    PHASE_CHIMES_WARNING,
    PHASE_CHIMES_NOTIFICATION,
    PHASE_CHIMES_ERROR,
    PHASE_CHIMES_ALARM,
    PHASE_ARPEGGIO,
    PHASE_RTTTL_NOKIA,
    PHASE_RTTTL_MARIO,
    PHASE_RTTTL_STARWARS,
    PHASE_MULTITASKING_SHOWCASE,
    PHASE_IDLE_MENU
};

DemoPhase currentPhase = PHASE_STARTUP;
bool autoDemoActive = true;
uint32_t loopCounter = 0;
uint32_t lastReportMillis = 0;

// ============================================================================
// Prototipe Fungsi
// ============================================================================
void printInteractiveMenu();
void handleSerialInput();
void runAutoDemoSequencer();

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    fast.begin(115200);
    led.mode(OUTPUT);
    led.low();

    // Inisialisasi driver buzzer
    buzzer.begin();

    fast.log(F("========================================================="));
    fast.log(F("   IskakINO - Advanced Non-Blocking Buzzer Showcase      "));
    fast.log(F("========================================================="));
    fast.logf(F("[Info] Pin Buzzer : GPIO %d\n"), BUZZER_PIN);
    fast.logf(F("[Info] Pin Status : GPIO %d (LED)\n"), STATUS_LED);
    fast.logf(F("[Info] Platform   : %s\n"),
#if defined(ISKAKINO_PLATFORM_AVR)
             "Arduino AVR (Uno/Nano/Mega)"
#elif defined(ISKAKINO_PLATFORM_ESP8266)
             "ESP8266 (NodeMCU)"
#elif defined(ISKAKINO_PLATFORM_ESP32)
             "ESP32 (Dev Module)"
#else
             "Unknown Target"
#endif
    );

    printInteractiveMenu();

    // Bunyikan nada startup pembuka
    buzzer.playNotification();
    currentPhase = PHASE_STARTUP;
}

// ============================================================================
// LOOP UTAMA (100% Non-Blocking)
// ============================================================================
void loop() {
    // 1. WAJIB: Refresh state machine buzzer di setiap putaran loop
    buzzer.update();

    // 2. Indikator LED: Sinkron menyala saat buzzer sedang aktif berbunyi
    if (currentPhase != PHASE_MULTITASKING_SHOWCASE) {
        if (buzzer.isPlaying()) {
            led.high();
        } else {
            led.low();
        }
    }

    // 3. Baca input dari Serial Monitor kapan saja
    handleSerialInput();

    // 4. Jalankan rangkaian demo otomatis (jika aktif)
    if (autoDemoActive) {
        runAutoDemoSequencer();
    }

    // 5. Bukti Non-Blocking: Hitung siklus loop CPU di latar belakang
    loopCounter++;
    if (millis() - lastReportMillis >= 3000) {
        lastReportMillis = millis();
        // Cetak uptime dan throughput loop per detik
        fast.logf(F("[Heartbeat] Uptime: %lu ms | CPU Loop Throughput: %lu iterasi/3dtk | Audio: %s\n"),
                  (unsigned long)millis(),
                  (unsigned long)loopCounter,
                  buzzer.isPlaying() ? "PLAYING" : "IDLE");
        loopCounter = 0;
    }
}

// ============================================================================
// Logika Pengendali Demo Otomatis
// ============================================================================
void runAutoDemoSequencer() {
    // Jangan berpindah fase sebelum audio yang sedang dimainkan selesai
    if (buzzer.isPlaying()) return;

    switch (currentPhase) {
        case PHASE_STARTUP:
            if (fast.once(1200, 0)) {
                fast.reset(0);
                fast.log(F("\n>>> [1/10] Uji Single Beep (150ms @ 2.4kHz)..."));
                buzzer.beep(150, 2400);
                currentPhase = PHASE_BEEP_TEST;
            }
            break;

        case PHASE_BEEP_TEST:
            if (fast.once(1000, 0)) {
                fast.reset(0);
                fast.log(F(">>> [2/10] Uji Nada Status: SUCCESS Chime..."));
                buzzer.playSuccess();
                currentPhase = PHASE_CHIMES_SUCCESS;
            }
            break;

        case PHASE_CHIMES_SUCCESS:
            if (fast.once(1000, 0)) {
                fast.reset(0);
                fast.log(F(">>> [3/10] Uji Nada Status: WARNING Tone..."));
                buzzer.playWarning();
                currentPhase = PHASE_CHIMES_WARNING;
            }
            break;

        case PHASE_CHIMES_WARNING:
            if (fast.once(1000, 0)) {
                fast.reset(0);
                fast.log(F(">>> [4/10] Uji Nada Status: NOTIFICATION Sound..."));
                buzzer.playNotification();
                currentPhase = PHASE_CHIMES_NOTIFICATION;
            }
            break;

        case PHASE_CHIMES_NOTIFICATION:
            if (fast.once(1000, 0)) {
                fast.reset(0);
                fast.log(F(">>> [5/10] Uji Nada Status: ERROR Alert..."));
                buzzer.playError();
                currentPhase = PHASE_CHIMES_ERROR;
            }
            break;

        case PHASE_CHIMES_ERROR:
            if (fast.once(1200, 0)) {
                fast.reset(0);
                fast.log(F(">>> [6/10] Uji Nada Status: ALARM Siren (3 siklus)..."));
                buzzer.playAlarm(3);
                currentPhase = PHASE_CHIMES_ALARM;
            }
            break;

        case PHASE_CHIMES_ALARM:
            if (fast.once(1500, 0)) {
                fast.reset(0);
                fast.log(F(">>> [7/10] Uji Custom Arpeggio Sequence (C-E-G-B-C)..."));
                buzzer.playSequence(ARPEGGIO_NOTES, ARPEGGIO_DURS, ARPEGGIO_COUNT);
                currentPhase = PHASE_ARPEGGIO;
            }
            break;

        case PHASE_ARPEGGIO:
            if (fast.once(1500, 0)) {
                fast.reset(0);
                fast.log(F(">>> [8/10] Uji RTTTL Ringtone: NOKIA TUNE (PROGMEM)..."));
                buzzer.playRTTTL((const __FlashStringHelper*)MELODY_NOKIA);
                currentPhase = PHASE_RTTTL_NOKIA;
            }
            break;

        case PHASE_RTTTL_NOKIA:
            if (fast.once(2000, 0)) {
                fast.reset(0);
                fast.log(F(">>> [9/10] Uji RTTTL Ringtone: SUPER MARIO THEME..."));
                buzzer.playRTTTL((const __FlashStringHelper*)MELODY_MARIO);
                currentPhase = PHASE_RTTTL_MARIO;
            }
            break;

        case PHASE_RTTTL_MARIO:
            if (fast.once(2000, 0)) {
                fast.reset(0);
                fast.log(F(">>> [10/10] Uji RTTTL Ringtone: STAR WARS IMPERIAL MARCH..."));
                buzzer.playRTTTL((const __FlashStringHelper*)MELODY_STARWARS);
                currentPhase = PHASE_RTTTL_STARWARS;
            }
            break;

        case PHASE_RTTTL_STARWARS:
            if (fast.once(2000, 0)) {
                fast.reset(0);
                fast.log(F("\n========================================================="));
                fast.log(F("  Uji Khusus: Pembuktian Multi-Tasking Non-Blocking!    "));
                fast.log(F("  (Memutar melodi sambil LED berkedip cepat 50ms)        "));
                fast.log(F("========================================================="));
                buzzer.playRTTTL((const __FlashStringHelper*)MELODY_NOKIA);
                currentPhase = PHASE_MULTITASKING_SHOWCASE;
            }
            break;

        case PHASE_MULTITASKING_SHOWCASE:
            // Kedipkan LED dengan sangat cepat (setiap 60 ms) membuktikan CPU tidak terhenti
            if (fast.every(60, 1)) {
                led.toggle();
            }

            // Setelah lagu selesai, kembali ke mode standby
            if (!buzzer.isPlaying()) {
                led.low();
                fast.log(F("\n[Selesai] Seluruh rangkaian demonstrasi otomatis selesai!"));
                fast.log(F("[Info] Anda sekarang dapat mencoba tombol menu Serial di bawah ini.\n"));
                printInteractiveMenu();
                autoDemoActive = false;
                currentPhase = PHASE_IDLE_MENU;
            }
            break;

        case PHASE_IDLE_MENU:
        default:
            break;
    }
}

// ============================================================================
// Menu Bantuan & Kontrol Serial Interaktif
// ============================================================================
void printInteractiveMenu() {
    fast.log(F("---------------------------------------------------------"));
    fast.log(F("  Ketik karakter di Serial Monitor untuk memicu suara:   "));
    fast.log(F("  [1] Single Beep           [6] Alarm Siren (3x)         "));
    fast.log(F("  [2] Success Chime         [7] Melodi Nokia Tune        "));
    fast.log(F("  [3] Warning Tone          [8] Melodi Super Mario       "));
    fast.log(F("  [4] Notification Sound    [9] Melodi Star Wars         "));
    fast.log(F("  [5] Error Alert           [0] Arpeggio Chord           "));
    fast.log(F("  [s] STOP Audio            [m] Toggle Mute On/Off       "));
    fast.log(F("  [a] Ulangi Demo Otomatis  [h] Cetak Ulang Menu Ini     "));
    fast.log(F("---------------------------------------------------------"));
}

void handleSerialInput() {
    while (Serial.available() > 0) {
        char cmd = Serial.read();
        if (cmd == '\r' || cmd == '\n' || cmd == ' ') continue;

        fast.logf(F("[Serial Cmd] Menjalankan perintah: '%c'\n"), cmd);

        switch (cmd) {
            case '1':
                buzzer.beep(120, 2200);
                fast.log(F("-> Beep (120ms @ 2.2kHz)"));
                break;
            case '2':
                buzzer.playSuccess();
                fast.log(F("-> Play Success Chime"));
                break;
            case '3':
                buzzer.playWarning();
                fast.log(F("-> Play Warning Tone"));
                break;
            case '4':
                buzzer.playNotification();
                fast.log(F("-> Play Notification Tone"));
                break;
            case '5':
                buzzer.playError();
                fast.log(F("-> Play Error Alert"));
                break;
            case '6':
                buzzer.playAlarm(3);
                fast.log(F("-> Play Alarm Siren (3x)"));
                break;
            case '7':
                buzzer.playRTTTL((const __FlashStringHelper*)MELODY_NOKIA);
                fast.log(F("-> Play Melodi: Nokia Tune"));
                break;
            case '8':
                buzzer.playRTTTL((const __FlashStringHelper*)MELODY_MARIO);
                fast.log(F("-> Play Melodi: Super Mario"));
                break;
            case '9':
                buzzer.playRTTTL((const __FlashStringHelper*)MELODY_STARWARS);
                fast.log(F("-> Play Melodi: Star Wars"));
                break;
            case '0':
                buzzer.playSequence(ARPEGGIO_NOTES, ARPEGGIO_DURS, ARPEGGIO_COUNT);
                fast.log(F("-> Play Custom Arpeggio Sequence"));
                break;
            case 's':
            case 'S':
                buzzer.stop();
                led.low();
                fast.log(F("-> STOP: Suara dihentikan seketika."));
                break;
            case 'm':
            case 'M':
                buzzer.setMute(!buzzer.isMuted());
                fast.logf(F("-> Mute Status: %s\n"), buzzer.isMuted() ? "MUTED (Hening)" : "UNMUTED (Bersuara)");
                break;
            case 'a':
            case 'A':
                autoDemoActive = true;
                currentPhase = PHASE_STARTUP;
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
