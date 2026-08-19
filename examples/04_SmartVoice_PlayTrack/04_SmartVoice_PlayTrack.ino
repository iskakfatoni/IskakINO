/*
 * 04_SmartVoice_PlayTrack.ino
 * Modul: IskakINO_SmartVoice & IskakINO_ArduFast (Universal)
 *
 * Menunjukkan:
 *   1. Komunikasi non-blocking ke DFPlayer Mini MP3
 *   2. Membaca status kartu MicroSD dan umpan balik error via IskakINO_Result
 *   3. Pembacaan tombol dengan debouncing stabil via ArduFast
 *   4. Logging status pemutaran audio dengan ArduFast logger
 */

#include <IskakINO.h>

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    #if defined(SOC_UART_NUM) && (SOC_UART_NUM < 3)
        #define VOICE_SERIAL Serial1
    #else
        #define VOICE_SERIAL Serial2
    #endif
#else
    #include <SoftwareSerial.h>
    SoftwareSerial voiceSoftSerial(10, 11); // RX=10, TX=11 untuk Arduino AVR
    #define VOICE_SERIAL voiceSoftSerial
#endif

IskakINO_ArduFast   fast;
IskakINO_SmartVoice voice;

#define BUSY_PIN     4
#define BUTTON_PIN   5 // Tombol aktif LOW (ke GND saat ditekan)

void setup() {
    fast.begin(115200);
    VOICE_SERIAL.begin(9600); // DFPlayer Mini beroperasi di baudrate 9600

    fast.log(F("========================================"));
    fast.log(F("  IskakINO - SmartVoice MP3 Showcase    "));
    fast.log(F("========================================"));

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    voice.setDebug(true);
    voice.begin(VOICE_SERIAL, BUSY_PIN);

    if (voice.isSDCardReady(500)) {
        fast.log(F("[Ready] SD Card terdeteksi, modul siap memutar audio."));
        voice.setVolume(22); // Volume rentang 0 s/d 30
    } else {
        fast.logf(F("[Error] SD Card belum siap: %s (Periksa kartu MicroSD/kabel RX-TX)"),
                  IskakINO_ResultToString(voice.lastError()));
    }
}

void loop() {
    static bool lastBtnPressed = false;

    // Baca tombol dengan filter kestabilan ArduFast (anti-noise)
    bool isPressed = (fast.readStable(BUTTON_PIN, 8) == LOW);

    // Deteksi transisi penekanan tombol (Falling Edge)
    if (isPressed && !lastBtnPressed) {
        fast.log(F("[Action] Tombol ditekan -> Memutar Track 0001.mp3..."));
        voice.playTrack(1);

        if (voice.lastError() != IskakINO_Result::OK) {
            fast.logf(F("[Error] Gagal memutar: %s"), IskakINO_ResultToString(voice.lastError()));
        }
    }
    lastBtnPressed = isPressed;

    // Pantau status pemutaran via pin BUSY secara berkala (setiap 100 ms)
    if (fast.every(100, 0)) {
        static bool wasPlaying = false;
        bool playing = voice.isPlaying(BUSY_PIN);
        if (playing != wasPlaying) {
            fast.log(playing ? F("[Status] Sedang memutar suara...") : F("[Status] Pemutaran selesai."));
            wasPlaying = playing;
        }
    }
}
