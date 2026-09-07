/*
 * src/websockets/IskakINO_WS_SHA1.h
 *
 * Engine SHA-1 (RFC 3174) & Base64 Encoder zero-dependency dan zero-malloc
 * untuk kalkulasi Sec-WebSocket-Accept handshake WebSocket (RFC 6455).
 *
 * Didesain khusus untuk embedded system (RAM super hemat & aman dari heap leak).
 *
 * Hanya aktif pada platform yang memiliki konektivitas WiFi (ESP32 / ESP8266).
 */

#ifndef ISKAKINO_WS_SHA1_H
#define ISKAKINO_WS_SHA1_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_HAS_WIFI)

#include <Arduino.h>

class IskakINO_WS_SHA1 {
public:
    static void hash(const uint8_t* data, size_t len, uint8_t hashResult[20]) {
        uint32_t h0 = 0x67452301;
        uint32_t h1 = 0xEFCDAB89;
        uint32_t h2 = 0x98BADCFE;
        uint32_t h3 = 0x10325476;
        uint32_t h4 = 0xC3D2E1F0;

        uint64_t totalBits = (uint64_t)len * 8;
        size_t paddedLen = ((len + 8) / 64 + 1) * 64;
        
        // Proses per blok 64 byte
        for (size_t block = 0; block < paddedLen; block += 64) {
            uint8_t chunk[64];
            for (size_t i = 0; i < 64; ++i) {
                size_t idx = block + i;
                if (idx < len) {
                    chunk[i] = data[idx];
                } else if (idx == len) {
                    chunk[i] = 0x80;
                } else if (idx >= paddedLen - 8) {
                    size_t bitIdx = 7 - (paddedLen - 1 - idx);
                    chunk[i] = (uint8_t)(totalBits >> (bitIdx * 8));
                } else {
                    chunk[i] = 0x00;
                }
            }

            uint32_t w[80];
            for (size_t i = 0; i < 16; ++i) {
                w[i] = ((uint32_t)chunk[i * 4] << 24) |
                       ((uint32_t)chunk[i * 4 + 1] << 16) |
                       ((uint32_t)chunk[i * 4 + 2] << 8) |
                       ((uint32_t)chunk[i * 4 + 3]);
            }
            for (size_t i = 16; i < 80; ++i) {
                uint32_t temp = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
                w[i] = (temp << 1) | (temp >> 31);
            }

            uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
            for (size_t i = 0; i < 80; ++i) {
                uint32_t f, k;
                if (i < 20) {
                    f = (b & c) | ((~b) & d);
                    k = 0x5A827999;
                } else if (i < 40) {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                } else if (i < 60) {
                    f = (b & c) | (b & d) | (c & d);
                    k = 0x8F1BBCDC;
                } else {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }

                uint32_t temp = ((a << 5) | (a >> 27)) + f + e + k + w[i];
                e = d;
                d = c;
                c = (b << 30) | (b >> 2);
                b = a;
                a = temp;
            }

            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
            h4 += e;
        }

        uint32_t h[5] = {h0, h1, h2, h3, h4};
        for (int i = 0; i < 5; ++i) {
            hashResult[i * 4]     = (uint8_t)(h[i] >> 24);
            hashResult[i * 4 + 1] = (uint8_t)(h[i] >> 16);
            hashResult[i * 4 + 2] = (uint8_t)(h[i] >> 8);
            hashResult[i * 4 + 3] = (uint8_t)(h[i]);
        }
    }

    static String base64Encode(const uint8_t* data, size_t len) {
        static const char b64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        String out = "";
        out.reserve(((len + 2) / 3) * 4);

        for (size_t i = 0; i < len; i += 3) {
            uint32_t octet_a = i < len ? data[i] : 0;
            uint32_t octet_b = (i + 1) < len ? data[i + 1] : 0;
            uint32_t octet_c = (i + 2) < len ? data[i + 2] : 0;

            uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

            out += b64Chars[(triple >> 18) & 0x3F];
            out += b64Chars[(triple >> 12) & 0x3F];
            out += (i + 1 < len) ? b64Chars[(triple >> 6) & 0x3F] : '=';
            out += (i + 2 < len) ? b64Chars[triple & 0x3F] : '=';
        }
        return out;
    }

    static String generateAcceptKey(const String& clientKey) {
        String combined = clientKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
        uint8_t hashResult[20];
        hash((const uint8_t*)combined.c_str(), combined.length(), hashResult);
        return base64Encode(hashResult, 20);
    }
};

#endif // defined(ISKAKINO_HAS_WIFI)
#endif // ISKAKINO_WS_SHA1_H
