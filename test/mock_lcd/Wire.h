#ifndef MOCK_WIRE_H
#define MOCK_WIRE_H
#include <Arduino.h>
#include <cstdint>

// Mock minimal TwoWire — test driver mengontrol alamat mana yang "ACK"
// lewat ackAddresses (set of address yang dianggap ada perangkatnya).
class TwoWireMock {
public:
    uint8_t _lastTxAddr = 0;
    uint8_t _lastWrittenByte = 0;
    bool _ackAddress[128] = {false};
    int beginCalls = 0;
    unsigned long _clock = 0;

    void begin() { beginCalls++; }
    void setClock(unsigned long hz) { _clock = hz; }
    void beginTransmission(uint8_t addr) { _lastTxAddr = addr; }
    uint8_t endTransmission() { return _ackAddress[_lastTxAddr] ? 0 : 2; } // 0=sukses (mirip Wire asli)
    size_t write(uint8_t data) { _lastWrittenByte = data; return 1; }
};
extern TwoWireMock Wire;
#endif
