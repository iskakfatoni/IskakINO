#ifndef MOCK_UDP_H
#define MOCK_UDP_H
#include <Arduino.h>
#include <cstring>
#include <cstdint>

// Mock minimal kelas dasar UDP milik Arduino, cukup untuk menguji
// state machine IskakINO_FastNTP tanpa jaringan sungguhan. Test driver
// mengontrol kapan "paket" datang lewat queuePacket()/queueNoPacket().
class UDP {
public:
    uint8_t _queuedPacket[64];
    int _queuedSize = 0;   // 0 = belum ada paket "masuk"
    int _beginPacketCalls = 0;
    int _writeCalls = 0;
    int _endPacketCalls = 0;
    const char* _lastHost = nullptr;

    bool begin(uint16_t) { return true; }

    void beginPacket(const char* host, uint16_t) { _lastHost = host; _beginPacketCalls++; }
    size_t write(const uint8_t*, size_t len) { _writeCalls++; return len; }
    void endPacket() { _endPacketCalls++; }

    int parsePacket() {
        int s = _queuedSize;
        return s;
    }
    int read(unsigned char* buf, size_t len) {
        size_t n = (size_t)_queuedSize < len ? (size_t)_queuedSize : len;
        memcpy(buf, _queuedPacket, n);
        _queuedSize = 0; // paket sudah "dibaca", kosongkan
        return (int)n;
    }

    // --- Helper test (bukan bagian API UDP asli) ---
    void queueNtpResponse(uint32_t secsSince1900, uint8_t stratum = 1) {
        memset(_queuedPacket, 0, 48);
        _queuedPacket[1] = stratum; // stratum==0 -> Kiss-of-Death
        _queuedPacket[40] = (secsSince1900 >> 24) & 0xFF;
        _queuedPacket[41] = (secsSince1900 >> 16) & 0xFF;
        _queuedPacket[42] = (secsSince1900 >> 8) & 0xFF;
        _queuedPacket[43] = secsSince1900 & 0xFF;
        _queuedSize = 48;
    }
    void queueNothing() { _queuedSize = 0; }
};
#endif
