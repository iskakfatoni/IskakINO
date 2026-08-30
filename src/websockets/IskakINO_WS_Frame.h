/*
 * src/websockets/IskakINO_WS_Frame.h
 *
 * Struktur data, enum event, opcode, dan helper pengkodean frame
 * protokol WebSocket standar (RFC 6455).
 *
 * Hanya aktif pada platform yang memiliki konektivitas WiFi (ESP32 / ESP8266).
 */

#ifndef ISKAKINO_WS_FRAME_H
#define ISKAKINO_WS_FRAME_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_HAS_WIFI)

#include <Arduino.h>

enum class IskakWSEvent : uint8_t {
    DISCONNECTED = 0,
    CONNECTED    = 1,
    TEXT         = 2,
    BINARY       = 3,
    PING         = 4,
    PONG         = 5,
    ERROR        = 6
};

enum class IskakWSClientState : uint8_t {
    DISCONNECTED   = 0,
    HANDSHAKE_WAIT = 1,
    CONNECTED      = 2
};

enum class IskakWSOpcode : uint8_t {
    CONTINUATION = 0x0,
    TEXT         = 0x1,
    BINARY       = 0x2,
    CLOSE        = 0x8,
    PING         = 0x9,
    PONG         = 0xA
};

// Callback type untuk penanganan event WebSocket
// signature: (uint8_t clientId, IskakWSEvent type, uint8_t* payload, size_t len)
typedef void (*IskakWSEventCallback)(uint8_t clientId, IskakWSEvent type, uint8_t* payload, size_t len);

class IskakINO_WS_Frame {
public:
    static size_t createHeader(uint8_t* headerBuf, IskakWSOpcode opcode, size_t payloadLen, bool mask = false, const uint8_t* maskKey = nullptr) {
        size_t hIdx = 0;
        // FIN bit (0x80) | Opcode
        headerBuf[hIdx++] = 0x80 | ((uint8_t)opcode & 0x0F);

        uint8_t maskBit = mask ? 0x80 : 0x00;

        if (payloadLen <= 125) {
            headerBuf[hIdx++] = maskBit | (uint8_t)payloadLen;
        } else if (payloadLen <= 65535) {
            headerBuf[hIdx++] = maskBit | 126;
            headerBuf[hIdx++] = (uint8_t)(payloadLen >> 8);
            headerBuf[hIdx++] = (uint8_t)(payloadLen & 0xFF);
        } else {
            headerBuf[hIdx++] = maskBit | 127;
            for (int i = 7; i >= 0; --i) {
                headerBuf[hIdx++] = (uint8_t)((uint64_t)payloadLen >> (i * 8));
            }
        }

        if (mask && maskKey) {
            for (int i = 0; i < 4; ++i) {
                headerBuf[hIdx++] = maskKey[i];
            }
        }

        return hIdx;
    }

    static void unmask(uint8_t* payload, size_t len, const uint8_t maskKey[4]) {
        for (size_t i = 0; i < len; ++i) {
            payload[i] ^= maskKey[i % 4];
        }
    }
};

#endif // defined(ISKAKINO_HAS_WIFI)
#endif // ISKAKINO_WS_FRAME_H
