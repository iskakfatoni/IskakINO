/*
 * 02_Basic_IO_Actuators.ino
 * Modul: IskakINO_Button, IskakINO_Relay, & IskakINO_Buzzer (Universal)
 *
 * Demonstrasi terpadu kendali tombol multi-gesture dan aktuator non-blocking:
 *  1. Button Multi-Gesture : Deteksi Single Click, Double Click, dan Long Press (>600ms).
 *  2. Smart Relay          : Toggle instan, Pulse auto-off (2s), dan Cadence Blink ritmik.
 *  3. Non-Blocking Buzzer  : Feedback audio nada status (beep, notification, alarm).
 *  4. Interaksi Terpadu    :
 *     - Single Click -> Toggle Relay + Audio Beep
 *     - Double Click -> Relay Pulse 2 Detik + Nada Notifikasi
 *     - Long Press   -> Relay Blink 4x Cadence + Sirine Alarm
 *  5. Serial Menu Control  : Uji aktuator secara manual via Serial Monitor.
 *
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

// Definisi Pin untuk berbagai board (hindari konflik macro BUZZER_PIN pada BasicIOShield)
#if defined(ESP32)
  const uint8_t PIN_ACT_BTN    = 4;
  const uint8_t PIN_ACT_RELAY  = 19;
  const uint8_t PIN_ACT_BUZZER = 18;
#elif defined(ESP8266)
  const uint8_t PIN_ACT_BTN    = D2; // GPIO4
  const uint8_t PIN_ACT_RELAY  = D5; // GPIO14
  const uint8_t PIN_ACT_BUZZER = D6; // GPIO12
#else
  const uint8_t PIN_ACT_BTN    = 2;  // Arduino Uno D2
  const uint8_t PIN_ACT_RELAY  = 7;  // Arduino Uno D7
  const uint8_t PIN_ACT_BUZZER = 8;  // Arduino Uno D8
#endif

IskakINO_ArduFast fast;
IskakINO_Button   btn(PIN_ACT_BTN, true, true);            // Active LOW, Internal Pullup
IskakINO_Relay    relay(PIN_ACT_RELAY, ISKAK_RELAY_ACTIVE_LOW);
IskakINO_Buzzer   buzzer(PIN_ACT_BUZZER, ISKAK_BUZZER_PASSIVE);

// Dukungan opsional EMS Basic I/O Shield jika dikompilasi pada AVR Uno
#if defined(ISKAKINO_PLATFORM_AVR) || defined(__AVR__)
IskakINO_BasicIOShield shield;
bool hasShield = false;
#endif

void printHelpMenu() {
    fast.log(F("\n--- Menu Interaktif Serial ---"));
    fast.log(F(" [1] Toggle Relay"));
    fast.log(F(" [2] Pulse Relay (2.0 detik)"));
    fast.log(F(" [3] Blink Relay (300ms on, 200ms off, 4x)"));
    fast.log(F(" [4] Mainkan Audio Feedback Beep"));
    fast.log(F(" [5] Mainkan Nada Notifikasi"));
    fast.log(F(" [6] Mainkan Sirine Alarm"));
    fast.log(F("-------------------------------\n"));
}

void onButtonClick() {
    relay.toggle();
    buzzer.beep(50, 2000); // 50ms, 2000Hz
    fast.logf(F("[Tombol] Single Click -> Relay: %s"), relay.isOn() ? "ON" : "OFF");
}

void onButtonDoubleClick() {
    relay.pulse(2000); // Nyala 2 detik lalu otomatis mati
    buzzer.playNotification();
    fast.log(F("[Tombol] Double Click -> Relay Pulse 2 Detik!"));
}

void onButtonLongPress() {
    relay.blink(250, 150, 4); // Kedip ritmik 4 siklus
    buzzer.playAlarm(3);
    fast.log(F("[Tombol] Long Press -> Relay Blink Cadence 4x + Alarm!"));
}

void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("   IskakINO - Button, Relay & Buzzer Actuators Suite     "));
    fast.log(F("========================================================="));
    fast.logf(F("[Konfigurasi Pin] Button: %d, Relay: %d, Buzzer: %d\n"), PIN_ACT_BTN, PIN_ACT_RELAY, PIN_ACT_BUZZER);

    // Hubungkan callback gestur tombol
    btn.onClick(onButtonClick);
    btn.onDoubleClick(onButtonDoubleClick);
    btn.onLongPressStart(onButtonLongPress);

    // Inisialisasi aktuator
    relay.off();
    buzzer.beep(80, 1800); // Nada boot

#if defined(ISKAKINO_PLATFORM_AVR) || defined(__AVR__)
    // Jika dijalankan di Arduino AVR, inisialisasi Basic I/O Shield
    shield.begin();
    hasShield = true;
    shield.setDisplay(26); // Tampilkan angka 26 di 7-segment
#endif

    printHelpMenu();
}

void loop() {
    // WAJIB: Polling loop setiap aktuator & sensor tombol
    btn.update();
    relay.update();
    buzzer.update();

#if defined(ISKAKINO_PLATFORM_AVR) || defined(__AVR__)
    if (hasShield) {
        shield.update(); // Multiplexing 7-segment non-blocking
    }
#endif

    // Baca input Serial untuk uji kontrol manual
    if (Serial.available()) {
        char ch = Serial.read();
        switch (ch) {
            case '1':
                relay.toggle();
                buzzer.beep(50, 2000);
                fast.logf(F("[Serial] Toggle Relay -> %s"), relay.isOn() ? "ON" : "OFF");
                break;
            case '2':
                relay.pulse(2000);
                buzzer.playNotification();
                fast.log(F("[Serial] Relay Pulse 2000ms"));
                break;
            case '3':
                relay.blink(300, 200, 4);
                buzzer.playAlarm(3);
                fast.log(F("[Serial] Relay Blink 4x"));
                break;
            case '4':
                buzzer.beep(60, 2200);
                break;
            case '5':
                buzzer.playNotification();
                break;
            case '6':
                buzzer.playAlarm(3);
                break;
            case 'h':
            case 'H':
                printHelpMenu();
                break;
        }
    }

    // Task laporan status berkala tiap 5 detik
    if (fast.every(5000, 0)) {
        fast.logf(F("[Status] Relay: %s | Buzzer Aktif: %s | Uptime: %lu s"),
                  relay.isOn() ? "ON" : "OFF",
                  buzzer.isPlaying() ? "YA" : "TIDAK",
                  (unsigned long)(millis() / 1000));
    }
}
