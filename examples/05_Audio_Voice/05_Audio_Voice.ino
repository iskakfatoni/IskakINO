/*
 * 05_Audio_Voice.ino
 * Modul: IskakINO_SmartVoice & IskakINO_ArduFast (Universal)
 *
 * Demonstrasi pemutar audio MP3 pintar non-blocking (DFPlayer Mini / JQ6500):
 *  1. Inisialisasi komunikasi Serial ke modul MP3 dengan deteksi timeout aman.
 *  2. Pengecekan ketersediaan MicroSD card secara otomatis.
 *  3. Kontrol volume (0-30), EQ Preset, dan navigasi folder/lagu.
 *  4. Monitoring pin BUSY untuk mengetahui status pemutaran tanpa polling serial berat.
 *  5. Menu Serial Monitor untuk interaksi langsung memutar musik/efek suara.
 *
 * Kompatibel: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#include <IskakINO.h>

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
    #if defined(SOC_UART_NUM) && (SOC_UART_NUM < 3)
        #define VOICE_SERIAL Serial1
    #else
        #define VOICE_SERIAL Serial2
    #endif
#elif defined(ESP8266)
    #include <SoftwareSerial.h>
    SoftwareSerial voiceSoftSerial(D5, D6); // RX=D5 (GPIO14), TX=D6 (GPIO12)
    #define VOICE_SERIAL voiceSoftSerial
#else
    #include <SoftwareSerial.h>
    SoftwareSerial voiceSoftSerial(10, 11); // RX=10, TX=11 untuk Arduino AVR
    #define VOICE_SERIAL voiceSoftSerial
#endif

IskakINO_ArduFast   fast;
IskakINO_SmartVoice voice;

const uint8_t BUSY_PIN = 4;
uint8_t currentVolume = 20;

void printMenu() {
    fast.log(F("\n--- Menu Kontrol SmartVoice MP3 ---"));
    fast.log(F(" [1] Putar Track #1"));
    fast.log(F(" [2] Putar Track #2"));
    fast.log(F(" [P] Pause / Resume"));
    fast.log(F(" [S] Stop Pemutaran"));
    fast.log(F(" [+] Naikkan Volume"));
    fast.log(F(" [-] Turunkan Volume"));
    fast.log(F("-----------------------------------\n"));
}

void setup() {
    fast.begin(115200);
    VOICE_SERIAL.begin(9600); // DFPlayer Mini beroperasi pada baudrate 9600

    fast.log(F("========================================================="));
    fast.log(F("      IskakINO - SmartVoice MP3 Audio Showcase           "));
    fast.log(F("========================================================="));

    voice.setDebug(true);
    voice.begin(VOICE_SERIAL, BUSY_PIN);

    if (voice.isSDCardReady(600)) {
        fast.log(F("[Ready] Kartu MicroSD terdeteksi dan siap memutar suara."));
        voice.setVolume(currentVolume);
    } else {
        fast.logf(F("[Peringatan] MicroSD belum siap: %s (Cek koneksi kabel RX/TX)"),
                  IskakINO_ResultToString(voice.lastError()));
    }

    printMenu();
}

void loop() {
    // Baca perintah Serial dari pengguna
    if (Serial.available()) {
        char cmd = Serial.read();
        switch (cmd) {
            case '1':
                fast.log(F("[Audio] Memutar Track #1"));
                voice.play(1);
                break;
            case '2':
                fast.log(F("[Audio] Memutar Track #2"));
                voice.play(2);
                break;
            case 'p':
            case 'P':
                fast.log(F("[Audio] Pause / Resume"));
                voice.pause();
                break;
            case 's':
            case 'S':
                fast.log(F("[Audio] Stop"));
                voice.stop();
                break;
            case '+':
                if (currentVolume < 30) currentVolume++;
                voice.setVolume(currentVolume);
                fast.logf(F("[Audio] Volume: %u / 30"), currentVolume);
                break;
            case '-':
                if (currentVolume > 0) currentVolume--;
                voice.setVolume(currentVolume);
                fast.logf(F("[Audio] Volume: %u / 30"), currentVolume);
                break;
            case 'h':
            case 'H':
                printMenu();
                break;
        }
    }

    // Laporan status audio setiap 3 detik
    if (fast.every(3000, 0)) {
        fast.logf(F("[Status Audio] Sedang Memutar: %s | Volume: %u"),
                  voice.isPlaying() ? "YA" : "TIDAK", currentVolume);
    }
}
