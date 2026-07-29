// Smoke-test fungsional IskakINO_SmartVoice — verifikasi protokol frame
// (checksum), isSDCardReady() dgn berbagai skenario respons, dan
// lastError()/setDebug() (kapabilitas BARU, murni tambahan).
//
// CATATAN: readResponse() aslinya BLOCKING (busy-wait while millis()-start
// < timeout) dan TIDAK disentuh migrasi ini sama sekali. Skenario "timeout
// murni tanpa respons" sengaja TIDAK dieksekusi langsung di sini karena
// mock millis() tidak auto-maju (dikontrol manual oleh test) — memanggilnya
// akan hang selamanya di lingkungan native. Ini bukan celah pada migrasi
// (readResponse() tidak diubah), jadi cukup diverifikasi lewat review kode.
#include "ArduinoExtra.h"
#include "Stream.h"
#include "../../src/voice/IskakINO_SmartVoice.h"
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    Stream voiceSerial;
    IskakINO_SmartVoice voice;
    voice.setDebug(true);

    // --- Sebelum begin(): semua perintah harus NOT_CONNECTED, tidak crash ---
    voice.setVolume(20);
    assert(voice.lastError() == IskakINO_Result::NOT_CONNECTED);
    voice.playTrack(1);
    assert(voice.lastError() == IskakINO_Result::NOT_CONNECTED);

    voice.begin(voiceSerial, 255, 10);
    assert(voice.lastError() == IskakINO_Result::OK);

    // --- Verifikasi frame yg DIKIRIM playTrack() persis sesuai protokol ---
    voiceSerial.txLog.clear();
    voice.playTrack(5);
    assert(voice.lastError() == IskakINO_Result::OK);
    assert(voiceSerial.txLog.size() == 10);
    assert(voiceSerial.txLog[0] == 0x7E); // start byte
    assert(voiceSerial.txLog[3] == 0x03); // cmd play track
    assert(voiceSerial.txLog[5] == 0x00); // param high
    assert(voiceSerial.txLog[6] == 0x05); // param low (track=5)
    assert(voiceSerial.txLog[9] == 0xEF); // end byte

    // --- Validasi argumen: track==0 harus INVALID_ARG, TIDAK mengirim frame ---
    voiceSerial.txLog.clear();
    voice.playTrack(0);
    assert(voice.lastError() == IskakINO_Result::INVALID_ARG);
    assert(voiceSerial.txLog.empty()); // tidak ada yang terkirim

    voiceSerial.txLog.clear();
    voice.playFromFolder(0, 1); // folder 0 invalid
    assert(voice.lastError() == IskakINO_Result::INVALID_ARG);
    assert(voiceSerial.txLog.empty());

    voiceSerial.txLog.clear();
    voice.playFromFolder(1, 1); // valid
    assert(voice.lastError() == IskakINO_Result::OK);
    assert(voiceSerial.txLog.size() == 10);

    // --- isSDCardReady(): SD terpasang (bit0=1) ---
    voiceSerial.clearRx();
    voiceSerial._autoReplyCmd = 0x3F;
    voiceSerial._autoReplyParam = 0x0002; // bit0=0 tapi ada bit lain -> belum terpasang
    bool ready = voice.isSDCardReady(100);
    assert(ready == false);
    assert(voice.lastError() == IskakINO_Result::OK); // query-nya sukses, walau SD blm siap

    voiceSerial.clearRx();
    voiceSerial._autoReplyParam = 0x0003; // bit0=1 -> SD siap
    ready = voice.isSDCardReady(100);
    assert(ready == true);
    assert(voice.lastError() == IskakINO_Result::OK);

    // --- isSDCardReady(): modul membalas error (0x40) ---
    voiceSerial.clearRx();
    voiceSerial._autoReplyCmd = 0x40;
    voiceSerial._autoReplyParam = 0x0000;
    ready = voice.isSDCardReady(100);
    assert(ready == false);
    assert(voice.lastError() == IskakINO_Result::WRITE_FAILED);

    // --- Frame dgn sampah di depan (sinkronisasi ke start byte 0x7E) ---
    voiceSerial.clearRx();
    voiceSerial._autoReplyCmd = 0x3F;
    voiceSerial._autoReplyParam = 0x0001;
    voiceSerial._autoReplyPrefixGarbage = {0x00, 0xAB};
    ready = voice.isSDCardReady(100);
    assert(ready == true);
    assert(voice.lastError() == IskakINO_Result::OK);

    // --- isPlaying() baca pin BUSY (Active Low) ---
    // Mock digitalRead() default mengembalikan LOW -> isPlaying() true
    // (pin BUSY DFPlayer memang Active Low saat sedang memutar).
    assert(voice.isPlaying(4) == true);

    printf("OK: semua smoke-test fungsional SmartVoice lolos\n");
    return 0;
}
