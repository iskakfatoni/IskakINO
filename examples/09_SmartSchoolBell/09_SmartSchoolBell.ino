/*
 * 09_SmartSchoolBell.ino
 * IskakINO Framework — Smart School Bell System (ESP32)
 *
 * Menggabungkan seluruh 6 modul IskakINO:
 *   - ArduFast   : FastPin I/O & Non-Blocking Scheduler
 *   - Storage    : Flash non-volatile schedule memory
 *   - LCD        : LiquidCrystal_I2C display driver
 *   - SmartVoice : DFPlayer Mini MP3 controller
 *   - WifiPortal : Web configuration portal & HTTP server
 *   - FastNTP    : SNTP clock synchronization & day tracker
 */

#include <IskakINO.h>
#include <WiFiUdp.h>
#include "BellWebPage.h"

#if defined(ARDUINO_ARCH_ESP32)
  #define VOICE_SERIAL Serial2
#else
  #include <SoftwareSerial.h>
  SoftwareSerial voiceSoftSerial(14, 12);
  #define VOICE_SERIAL voiceSoftSerial
#endif

// --- Pinout Hardware ---
#define PIN_RELAY_AMPLI   18
#define PIN_BUTTON_BELL   19
#define PIN_DFPLAYER_BUSY 4

// --- Objek Modul IskakINO ---
IskakINO_ArduFast   fast;
LiquidCrystal_I2C   lcd(16, 2);
IskakINO_SmartVoice voice;
IskakINO_WifiPortal portal;
WiFiUDP             ntpUdp;
IskakINO_FastNTP    ntp(ntpUdp, "pool.ntp.org");

// --- Profil Jadwal ---
enum BellProfile : uint8_t {
    PROFILE_AUTO     = 0,
    PROFILE_REGULER  = 1,
    PROFILE_JUMAT    = 2,
    PROFILE_UJIAN    = 3,
    PROFILE_RAMADHAN = 4,
    PROFILE_LIBUR    = 5
};

const char* profileNames[] = {
    "Otomatis", "Reguler", "Khusus Jumat", "Ujian", "Ramadhan", "Libur"
};

// --- Struktur Data Jadwal ---
struct BellSchedule {
    uint8_t hour;
    uint8_t minute;
    uint8_t profileMask;
    uint8_t days;
    uint8_t track;
    bool    enabled;
    char    label[16];
};

#define MAX_BELLS 16

struct BellConfig {
    uint8_t      activeProfile;
    uint8_t      count;
    uint8_t      volume;
    BellSchedule bells[MAX_BELLS];
};

BellConfig bellConfig;
int configStorageAddr;
int epochStorageAddr;

// --- Task Scheduler ID ---
#define TASK_CHECK_SCHEDULE 0
#define TASK_REFRESH_LCD    1

// --- State Machine Bel & Amplifier ---
enum BellState {
    BELL_IDLE,
    BELL_PRE_AMP,
    BELL_PLAYING,
    BELL_POST_AMP
};

BellState bellState = BELL_IDLE;
uint32_t stateTimer = 0;
uint8_t currentPlayTrack = 1;
int lastTriggeredMinute = -1;

#define DAYS_WEEKDAY 0x3E // Senin s/d Jumat
#define DAYS_SAT     0x7E // Senin s/d Sabtu
#define DAYS_ALL     0x7F // Setiap Hari

#define MASK_REGULER  (1 << 1)
#define MASK_JUMAT    (1 << 2)
#define MASK_UJIAN    (1 << 3)
#define MASK_RAMADHAN (1 << 4)

uint8_t getEffectiveProfile(int currentDay) {
    if (bellConfig.activeProfile != PROFILE_AUTO) {
        return bellConfig.activeProfile;
    }
    if (currentDay == 0) return PROFILE_LIBUR;
    if (currentDay == 5) return PROFILE_JUMAT;
    return PROFILE_REGULER;
}

void initDefaultSchedule() {
    bellConfig.activeProfile = PROFILE_AUTO;
    bellConfig.count = 8;
    bellConfig.volume = 25;

    // Profil Reguler
    bellConfig.bells[0] = {7, 0, MASK_REGULER, DAYS_WEEKDAY, 1, true, "Masuk Pagi"};
    bellConfig.bells[1] = {9, 45, MASK_REGULER, DAYS_WEEKDAY, 2, true, "Istirahat 1"};
    bellConfig.bells[2] = {10, 15, MASK_REGULER, DAYS_WEEKDAY, 3, true, "Masuk Kelas"};
    bellConfig.bells[3] = {14, 0, MASK_REGULER, DAYS_WEEKDAY, 4, true, "Pulang Sekolah"};

    // Profil Jumat
    bellConfig.bells[4] = {7, 0, MASK_JUMAT, (1 << 5), 1, true, "Masuk Jumat"};
    bellConfig.bells[5] = {11, 0, MASK_JUMAT, (1 << 5), 4, true, "Pulang Jumat"};

    // Profil Ujian
    bellConfig.bells[6] = {7, 30, MASK_UJIAN, DAYS_WEEKDAY, 1, true, "Ujian Sesi 1"};
    bellConfig.bells[7] = {11, 30, MASK_UJIAN, DAYS_WEEKDAY, 4, true, "Ujian Selesai"};

    IskakStorage.save(configStorageAddr, bellConfig);
}

void triggerBell(uint8_t trackNumber, const char* label) {
    if (bellState != BELL_IDLE) return;

    currentPlayTrack = trackNumber;
    bellState = BELL_PRE_AMP;
    stateTimer = millis();

    digitalWrite(PIN_RELAY_AMPLI, HIGH);

    lcd.clear();
    lcd.printCenter("BEL BERBUNYI!", 0);
    lcd.printCenter(label ? label : "Manual", 1);
}

void setupCustomWebRoutes() {
    IskakWebServer* server = portal.server();
    if (!server) return;

    // Sajikan Halaman Web Dashboard dari Flash Memory (PROGMEM)
    server->on("/bell", HTTP_GET, [server]() {
        server->send_P(200, "text/html", BELL_INDEX_HTML);
    });

    // API Endpoint: Ganti Mode Profil
    server->on("/bell/profile", HTTP_POST, [server]() {
        if (server->hasArg("profile")) {
            bellConfig.activeProfile = (uint8_t)server->arg("profile").toInt();
            IskakStorage.save(configStorageAddr, bellConfig);
        }
        server->sendHeader("Location", "/bell");
        server->send(303);
    });

    // API Endpoint: Trigger Bel Manual
    server->on("/bell/trigger", HTTP_GET, [server]() {
        uint8_t track = 1;
        if (server->hasArg("track")) {
            track = (uint8_t)server->arg("track").toInt();
        }
        triggerBell(track, "Web Trigger");
        server->sendHeader("Location", "/bell");
        server->send(303);
    });
}

void onSyncSukses(uint32_t utcEpoch) {
    uint32_t toSave = ntp.getUtcEpoch();
    IskakStorage.save(epochStorageAddr, toSave);
}

void setup() {
    Serial.begin(115200);
    fast.begin(115200);
    VOICE_SERIAL.begin(9600);

    pinMode(PIN_RELAY_AMPLI, OUTPUT);
    digitalWrite(PIN_RELAY_AMPLI, LOW);
    pinMode(PIN_BUTTON_BELL, INPUT_PULLUP);
    pinMode(PIN_DFPLAYER_BUSY, INPUT);

    IskakStorage.begin("schoolbell");
    epochStorageAddr  = IskakStorage.reserve(sizeof(uint32_t));
    configStorageAddr = IskakStorage.reserve(sizeof(BellConfig));

    if (!IskakStorage.load(configStorageAddr, bellConfig) || bellConfig.count == 0) {
        initDefaultSchedule();
    }

    lcd.begin();
    lcd.printCenter("IskakINO Bell", 0);
    lcd.printCenter("Inisialisasi...", 1);

    uint32_t savedEpoch;
    if (IskakStorage.load(epochStorageAddr, savedEpoch)) {
        ntp.setUtcEpoch(savedEpoch);
    }

    voice.begin(VOICE_SERIAL, PIN_DFPLAYER_BUSY);
    if (voice.isSDCardReady(500)) {
        voice.setVolume(bellConfig.volume);
    }

    portal.setBrandName("IskakINO School Bell");
    portal.setPortalTimeout(180);
    portal.beginAsync("IskakINO-SchoolBell");

    ntp.begin(25200, 0); // GMT+7 (WIB)
    ntp.onSync(onSyncSukses);
    ntp.setSyncInterval(3600000UL);

    setupCustomWebRoutes();

    lcd.clear();
    lcd.printCenter("Sistem Siap!", 0);
}

void loop() {
    portal.tick();
    ntp.update();
    lcd.update();

    // 1. State Machine Pengendali Relay & Audio (Non-Blocking)
    switch (bellState) {
        case BELL_PRE_AMP:
            if (millis() - stateTimer >= 2000) {
                voice.playTrack(currentPlayTrack);
                bellState = BELL_PLAYING;
                stateTimer = millis();
            }
            break;

        case BELL_PLAYING:
            if (millis() - stateTimer >= 3000) {
                if (!voice.isPlaying(PIN_DFPLAYER_BUSY)) {
                    bellState = BELL_POST_AMP;
                    stateTimer = millis();
                }
            }
            break;

        case BELL_POST_AMP:
            if (millis() - stateTimer >= 1000) {
                digitalWrite(PIN_RELAY_AMPLI, LOW);
                bellState = BELL_IDLE;
                lcd.clear();
            }
            break;

        case BELL_IDLE:
        default:
            break;
    }

    // 2. Evaluasi Jadwal Bel Sesuai Profil Aktif
    if (fast.every(1000, TASK_CHECK_SCHEDULE)) {
        if (ntp.isTimeSet()) {
            int currentHour   = ntp.getHours();
            int currentMinute = ntp.getMinutes();
            int currentSecond = ntp.getSeconds();
            int currentDay    = ntp.getDay();

            uint8_t effProfile = getEffectiveProfile(currentDay);

            if (effProfile != PROFILE_LIBUR && currentSecond == 0 && currentMinute != lastTriggeredMinute) {
                uint8_t profMask = (1 << effProfile);

                for (uint8_t i = 0; i < bellConfig.count; i++) {
                    if (bellConfig.bells[i].enabled) {
                        bool profMatches = (bellConfig.bells[i].profileMask & profMask) != 0;
                        bool dayMatches  = (bellConfig.bells[i].days & (1 << currentDay)) != 0;

                        if (profMatches && dayMatches &&
                            bellConfig.bells[i].hour == currentHour &&
                            bellConfig.bells[i].minute == currentMinute) {

                            lastTriggeredMinute = currentMinute;
                            triggerBell(bellConfig.bells[i].track, bellConfig.bells[i].label);
                            break;
                        }
                    }
                }
            }

            if (currentSecond != 0) {
                lastTriggeredMinute = -1;
            }
        }
    }

    // 3. Refresh LCD Tampilan Jam & Info Jadwal Terdekat
    if (bellState == BELL_IDLE && fast.every(1000, TASK_REFRESH_LCD)) {
        int curDay = ntp.getDay();
        uint8_t effProfile = getEffectiveProfile(curDay);

        lcd.setCursor(0, 0);
        if (!portal.isConnected()) {
            lcd.print(portal.state() == IskakPortalState::PORTAL ? F("Setup: 192.168.4.1") : F("Menyambung WiFi..."));
        } else if (!ntp.isTimeSet()) {
            lcd.print(F("Sync Waktu NTP.."));
        } else {
            String header = String(profileNames[effProfile]).substring(0, 3) + " " + ntp.getFormattedTime();
            lcd.print(header);
            lcd.print(F("       "));
        }

        lcd.setCursor(0, 1);
        if (ntp.isTimeSet() && effProfile != PROFILE_LIBUR) {
            int curH = ntp.getHours();
            int curM = ntp.getMinutes();
            int nextIdx = -1;
            int minDiff = 99999;
            uint8_t profMask = (1 << effProfile);

            for (uint8_t i = 0; i < bellConfig.count; i++) {
                if (bellConfig.bells[i].enabled &&
                   (bellConfig.bells[i].profileMask & profMask) &&
                   (bellConfig.bells[i].days & (1 << curDay))) {

                    int diff = (bellConfig.bells[i].hour * 60 + bellConfig.bells[i].minute) - (curH * 60 + curM);
                    if (diff > 0 && diff < minDiff) {
                        minDiff = diff;
                        nextIdx = i;
                    }
                }
            }

            if (nextIdx != -1) {
                char nextBuf[17];
                snprintf(nextBuf, sizeof(nextBuf), "> %02d:%02d %s",
                         bellConfig.bells[nextIdx].hour,
                         bellConfig.bells[nextIdx].minute,
                         bellConfig.bells[nextIdx].label);
                lcd.print(nextBuf);
            } else {
                lcd.print(F("Jadwal Selesai  "));
            }
        } else if (effProfile == PROFILE_LIBUR) {
            lcd.print(F("Mode Libur / Off"));
        } else {
            lcd.print(F("IskakINO SmartBell"));
        }
    }

    // 4. Tombol Fisik Manual Bel
    static bool lastBtnState = HIGH;
    bool btnState = digitalRead(PIN_BUTTON_BELL);
    if (btnState == LOW && lastBtnState == HIGH) {
        triggerBell(1, "Tombol Manual");
    }
    lastBtnState = btnState;
}
