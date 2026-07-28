/*
 * 04_SmartVoice_PlayTrack.ino
 * Modul: IskakINO_SmartVoice (universal -- perlu satu Stream/Serial
 * tambahan untuk modul DFPlayer Mini, mis. Serial1/Serial2 di ESP32,
 * atau SoftwareSerial di AVR)
 *
 * Menunjukkan pemutaran track dari SD card, cek status SD, dan membaca
 * lastError() (IskakINO_Result) untuk tahu kenapa sebuah perintah
 * diabaikan (mis. track tidak valid, belum begin(), dst.).
 *
 * Rangkaian: DFPlayer Mini RX/TX ke Serial2 (ganti sesuai board Anda),
 * pin BUSY opsional untuk deteksi sedang memutar atau tidak.
 */

#include <IskakINO.h>

IskakINO_SmartVoice voice;

#define VOICE_SERIAL Serial2   // ganti ke SoftwareSerial kalau board tidak punya UART kedua
#define BUSY_PIN     4
#define BUTTON_PIN   5         // tombol aktif LOW, ke GND saat ditekan

void setup() {
    Serial.begin(115200);
    VOICE_SERIAL.begin(9600); // DFPlayer Mini defaultnya 9600 baud

    voice.setDebug(true);
    voice.begin(VOICE_SERIAL, BUSY_PIN);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    if (voice.isSDCardReady(500)) {
        Serial.println(F("SD Card terdeteksi, siap memutar."));
        voice.setVolume(20); // 0-30
    } else {
        // lastError() membedakan TIMEOUT (modul tidak merespons sama
        // sekali -- cek wiring/baud) dari WRITE_FAILED (modul merespons
        // tapi bilang error, biasanya SD tidak terpasang).
        Serial.print(F("SD Card belum siap, alasan: "));
        Serial.println(IskakINO_ResultToString(voice.lastError()));
    }
}

void loop() {
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(BUTTON_PIN);

    // Deteksi tombol baru saja ditekan (falling edge), tanpa delay() debounce
    if (buttonState == LOW && lastButtonState == HIGH) {
        voice.playTrack(1); // putar 0001.mp3 dari root SD card
        if (voice.lastError() != IskakINO_Result::OK) {
            Serial.print(F("playTrack gagal: "));
            Serial.println(IskakINO_ResultToString(voice.lastError()));
        }
    }
    lastButtonState = buttonState;

    // Pantau status sedang memutar lewat pin BUSY (tidak perlu polling modul)
    static bool wasPlaying = false;
    bool playing = voice.isPlaying(BUSY_PIN);
    if (playing != wasPlaying) {
        Serial.println(playing ? F("Mulai memutar...") : F("Selesai memutar."));
        wasPlaying = playing;
    }
}
