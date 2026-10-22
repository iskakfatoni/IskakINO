/*
 * src/core/IskakINO_BitUtils.h
 * Utilitas manipulasi bit, konversi Endianness (Big/Little Endian),
 * dan penghitung Checksum / CRC (CRC-8, CRC-16, CRC-32).
 *
 * Mencegah bug memory misalignment dan kesalahan pengurutan byte pada
 * protokol jaringan (WebSockets, MQTT) dan pembacaan sensor bus (I2C, 1-Wire).
 */

#ifndef ISKAKINO_BITUTILS_H
#define ISKAKINO_BITUTILS_H

#include <Arduino.h>

class IskakINO_BitUtils {
  public:
    // --- Byte Swapping (Inversi Endianness) ---

    static inline uint16_t swap16(uint16_t val) {
        return (uint16_t)((val << 8) | (val >> 8));
    }

    static inline uint32_t swap32(uint32_t val) {
        return (((val & 0x000000FFUL) << 24) |
                ((val & 0x0000FF00UL) << 8)  |
                ((val & 0x00FF0000UL) >> 8)  |
                ((val & 0xFF000000UL) >> 24));
    }

    static inline uint64_t swap64(uint64_t val) {
        return (((val & 0x00000000000000FFULL) << 56) |
                ((val & 0x000000000000FF00ULL) << 40) |
                ((val & 0x0000000000FF0000ULL) << 24) |
                ((val & 0x00000000FF000000ULL) << 8)  |
                ((val & 0x000000FF00000000ULL) >> 8)  |
                ((val & 0x0000FF0000000000ULL) >> 24) |
                ((val & 0x00FF000000000000ULL) >> 40) |
                ((val & 0xFF00000000000000ULL) >> 56));
    }

    // --- Pembacaan Data Aman Bebas Alignment Fault ---

    // Membaca 16-bit Big-Endian (Network Byte Order) dari buffer byte
    static inline uint16_t readBE16(const uint8_t* buf) {
        if (!buf) return 0;
        return (uint16_t)(((uint16_t)buf[0] << 8) | (uint16_t)buf[1]);
    }

    // Menulis 16-bit Big-Endian ke buffer byte
    static inline void writeBE16(uint8_t* buf, uint16_t val) {
        if (!buf) return;
        buf[0] = (uint8_t)(val >> 8);
        buf[1] = (uint8_t)(val & 0xFF);
    }

    // Membaca 32-bit Big-Endian dari buffer byte
    static inline uint32_t readBE32(const uint8_t* buf) {
        if (!buf) return 0;
        return (((uint32_t)buf[0] << 24) |
                ((uint32_t)buf[1] << 16) |
                ((uint32_t)buf[2] << 8)  |
                ((uint32_t)buf[3]));
    }

    // Menulis 32-bit Big-Endian ke buffer byte
    static inline void writeBE32(uint8_t* buf, uint32_t val) {
        if (!buf) return;
        buf[0] = (uint8_t)(val >> 24);
        buf[1] = (uint8_t)(val >> 16);
        buf[2] = (uint8_t)(val >> 8);
        buf[3] = (uint8_t)(val & 0xFF);
    }

    // Membaca 16-bit Little-Endian dari buffer byte
    static inline uint16_t readLE16(const uint8_t* buf) {
        if (!buf) return 0;
        return (uint16_t)(((uint16_t)buf[1] << 8) | (uint16_t)buf[0]);
    }

    // Menulis 16-bit Little-Endian ke buffer byte
    static inline void writeLE16(uint8_t* buf, uint16_t val) {
        if (!buf) return;
        buf[0] = (uint8_t)(val & 0xFF);
        buf[1] = (uint8_t)(val >> 8);
    }

    // Membaca 32-bit Little-Endian dari buffer byte
    static inline uint32_t readLE32(const uint8_t* buf) {
        if (!buf) return 0;
        return (((uint32_t)buf[3] << 24) |
                ((uint32_t)buf[2] << 16) |
                ((uint32_t)buf[1] << 8)  |
                ((uint32_t)buf[0]));
    }

    // Menulis 32-bit Little-Endian ke buffer byte
    static inline void writeLE32(uint8_t* buf, uint32_t val) {
        if (!buf) return;
        buf[0] = (uint8_t)(val & 0xFF);
        buf[1] = (uint8_t)(val >> 8);
        buf[2] = (uint8_t)(val >> 16);
        buf[3] = (uint8_t)(val >> 24);
    }

    // --- Checksum & Cyclic Redundancy Check (CRC) ---

    // Menghitung CRC-8 (Default polynomial 0x8C / Dallas-Maxim 1-Wire, misal DS18B20)
    static uint8_t crc8(const uint8_t* data, size_t length, uint8_t polynomial = 0x8C, uint8_t initial = 0x00) {
        uint8_t crc = initial;
        if (!data) return 0;
        for (size_t i = 0; i < length; i++) {
            uint8_t inbyte = data[i];
            for (uint8_t j = 0; j < 8; j++) {
                uint8_t mix = (crc ^ inbyte) & 0x01;
                crc >>= 1;
                if (mix) crc ^= polynomial;
                inbyte >>= 1;
            }
        }
        return crc;
    }

    // Menghitung CRC-16 (Default polynomial 0xA001 / Modbus IBM, initial 0xFFFF)
    static uint16_t crc16(const uint8_t* data, size_t length, uint16_t polynomial = 0xA001, uint16_t initial = 0xFFFF) {
        uint16_t crc = initial;
        if (!data) return 0;
        for (size_t i = 0; i < length; i++) {
            crc ^= (uint16_t)data[i];
            for (uint8_t j = 0; j < 8; j++) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ polynomial;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }

    // Menghitung CRC-32 (IEEE 802.3 Ethernet / ZIP standard)
    static uint32_t crc32(const uint8_t* data, size_t length, uint32_t initial = 0xFFFFFFFFUL) {
        uint32_t crc = initial;
        if (!data) return 0;
        for (size_t i = 0; i < length; i++) {
            crc ^= (uint32_t)data[i];
            for (uint8_t j = 0; j < 8; j++) {
                if (crc & 1) {
                    crc = (crc >> 1) ^ 0xEDB88320UL;
                } else {
                    crc >>= 1;
                }
            }
        }
        return ~crc;
    }
};

#endif // ISKAKINO_BITUTILS_H
