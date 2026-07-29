#ifndef MOCK_STREAM_H
#define MOCK_STREAM_H
#include <Arduino.h>
#include <vector>
#include <cstdint>

// Mock minimal Arduino Stream, cukup untuk protokol DFPlayer Mini
// (available()/read()/write()). Test driver mengontrol apa yang "dibalas"
// modul lewat queueResponse()/queueRawByte(), dan bisa inspeksi apa yang
// DIKIRIM library lewat txLog (byte-byte yang di-write()).
class Stream {
public:
    std::vector<uint8_t> _rx; // data yang "diterima" dari modul (dibaca library)
    size_t _rxPos = 0;
    std::vector<uint8_t> txLog; // semua byte yang DIKIRIM library (untuk verifikasi)

    // Respons yang akan "dibalas" modul otomatis saat library mengirim
    // query 0x3F (dipanggil dari dalam write(), BUKAN diantre manual sebelum
    // isSDCardReady() dipanggil) — supaya urutannya persis meniru hardware
    // asli: flush buffer lama -> kirim query -> BARU modul membalas.
    // Kalau _autoReplyCmd == 0, tidak ada balasan otomatis (simulasi timeout).
    uint8_t _autoReplyCmd = 0;
    uint16_t _autoReplyParam = 0;
    std::vector<uint8_t> _autoReplyPrefixGarbage; // byte sampah sebelum frame valid (uji sinkronisasi)

    int available() { return (int)(_rx.size() - _rxPos); }
    void begin(unsigned long) {} // mock HardwareSerial::begin(baud), no-op
    int read() {
        if (_rxPos >= _rx.size()) return -1;
        return _rx[_rxPos++];
    }
    size_t write(const uint8_t* buf, size_t len) {
        for (size_t i = 0; i < len; i++) txLog.push_back(buf[i]);
        // Deteksi query 0x3F (byte index 3 = command) -> auto-balas
        if (len == 10 && buf[3] == 0x3F && _autoReplyCmd != 0) {
            for (uint8_t g : _autoReplyPrefixGarbage) _rx.push_back(g);
            queueValidFrame(_autoReplyCmd, _autoReplyParam);
        }
        return len;
    }

    // --- Helper test ---
    // Membangun & mengantre frame 10-byte respons DFPlayer yang VALID
    // (checksum benar), persis format yang dipakai sendRaw()/readResponse().
    void queueValidFrame(uint8_t cmd, uint16_t param) {
        uint8_t f[10];
        f[0] = 0x7E; f[1] = 0xFF; f[2] = 0x06; f[3] = cmd; f[4] = 0x00;
        f[5] = (uint8_t)(param >> 8); f[6] = (uint8_t)(param & 0xFF);
        uint16_t checksum = (uint16_t)(-(f[1]+f[2]+f[3]+f[4]+f[5]+f[6]));
        f[7] = (uint8_t)(checksum >> 8); f[8] = (uint8_t)(checksum & 0xFF);
        f[9] = 0xEF;
        for (int i = 0; i < 10; i++) _rx.push_back(f[i]);
    }
    void queueGarbageByte(uint8_t b) { _rx.push_back(b); }
    void clearRx() { _rx.clear(); _rxPos = 0; }
};

// Instance global Serial2 (tersedia di board nyata seperti ESP32) --
// dipakai contoh SmartVoice untuk komunikasi ke modul DFPlayer Mini.
extern Stream Serial2;
#endif
