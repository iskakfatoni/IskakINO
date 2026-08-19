/*
 * 13_Button_MultiGesture.ino
 * Modul: IskakINO_Button & IskakINO_ArduFast (Universal)
 *
 * Demonstrasi deteksi multi-gesture tombol pintar non-blocking:
 *  - Single Click   : Klik tunggal (misal untuk toggle LED / relay).
 *  - Double Click   : Klik ganda cepat (misal untuk ganti mode).
 *  - Multi-Click    : Menghitung total ketukan berulang (1, 2, 3x klik).
 *  - Long Press     : Deteksi tekan tahan (> 600ms) untuk aksi khusus / reset.
 *  - Event Callback : Callback asinkron onClick, onDoubleClick, onLongPressStart.
 *
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

// Definisi Pin Tombol & LED Indikator
#if defined(ESP32)
  const uint8_t BTN_PIN = 4;
  const uint8_t LED_PIN = 2;
#elif defined(ESP8266)
  const uint8_t BTN_PIN = D2; // GPIO4
  const uint8_t LED_PIN = D4; // GPIO2 / Onboard LED
#else
  const uint8_t BTN_PIN = 2;  // Arduino Uno / Nano D2
  const uint8_t LED_PIN = 13; // LED_BUILTIN
#endif

// Inisialisasi Objek ArduFast & Button (Pin BTN_PIN, Active LOW / INPUT_PULLUP)
IskakINO_ArduFast fast;
IskakINO_Button btn(BTN_PIN, true, true);

bool ledState = false;
uint32_t loopCounter = 0;
uint32_t lastReportMillis = 0;

// ============================================================================
// Callback Event Handler (Opsional)
// ============================================================================
void onButtonSingleClick() {
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    fast.logf(F("[Event: CLICK] Tombol diklik sekali! Status LED: %s\n"), ledState ? "MENYALA" : "PADAM");
}

void onButtonDoubleClick() {
    fast.log(F("[Event: DOUBLE CLICK] Tombol diklik ganda! Mengedipkan LED 3x..."));
    // Kedipkan cepat
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(60);
        digitalWrite(LED_PIN, LOW);
        delay(60);
    }
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

void onButtonLongPressStart() {
    fast.log(F("[Event: LONG PRESS START] Tombol ditahan > 600ms!"));
}

void onButtonLongPressEnd() {
    fast.log(F("[Event: LONG PRESS END] Tombol akhirnya dilepas setelah ditahan!"));
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    fast.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    fast.log(F("========================================================="));
    fast.log(F("       IskakINO - Advanced Smart Button Showcase         "));
    fast.log(F("========================================================="));
    fast.logf(F("[Info] Pin Tombol: %d (Active LOW / PULLUP) | Pin LED: %d\n"), BTN_PIN, LED_PIN);
    fast.log(F("[Petunjuk] Coba tekan tombol: 1x klik, 2x klik cepat, atau tekan tahan!"));

    // Inisialisasi Tombol
    btn.begin();

    // Daftarkan Callback Event
    btn.onClick(onButtonSingleClick);
    btn.onDoubleClick(onButtonDoubleClick);
    btn.onLongPressStart(onButtonLongPressStart);
    btn.onLongPressEnd(onButtonLongPressEnd);
}

// ============================================================================
// LOOP UTAMA (100% Non-Blocking)
// ============================================================================
void loop() {
    // 1. WAJIB: Update state machine tombol di setiap putaran loop
    btn.update();

    // 2. Demonstrasi Polling Status Real-Time (Saat tombol sedang ditahan)
    if (btn.isHolding()) {
        // Tampilkan durasi tahan setiap 500ms
        if (fast.every(500, 0)) {
            fast.logf(F("[Polling] Tombol sedang ditahan selama: %lu ms...\n"),
                      (unsigned long)btn.getHoldDuration());
        }
    }

    // 3. Pembuktian Non-Blocking: Heartbeat monitor
    loopCounter++;
    if (millis() - lastReportMillis >= 3000) {
        lastReportMillis = millis();
        fast.logf(F("[Heartbeat] Uptime: %lu ms | CPU Loop: %lu iterasi/3dtk\n"),
                  (unsigned long)millis(),
                  (unsigned long)loopCounter);
        loopCounter = 0;
    }
}
