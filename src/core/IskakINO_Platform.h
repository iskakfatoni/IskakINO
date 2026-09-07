/*
 * src/core/IskakINO_Platform.h
 * Deteksi platform terpusat + akses register langsung (FastPin<P>).
 *
 * SUMBER: diekstrak & disatukan dari IskakINO_ArduFast v1.1.0
 * (sebelumnya makro #if defined(ESP32)/ESP8266 ini terduplikasi di
 * hampir setiap modul: ArduFast, Storage, WifiPortal). Modul lain dalam
 * IskakINO kini cukup include header ini dan pakai makro ISKAKINO_PLATFORM_*
 * / ISKAKINO_HAS_* di bawah, tanpa perlu menulis ulang deteksi platform.
 */

#ifndef ISKAKINO_PLATFORM_H
#define ISKAKINO_PLATFORM_H

#include <Arduino.h>

#if defined(ESP32)
  #include "soc/gpio_struct.h"
#endif

// ============================================================================
// Deteksi Platform — satu sumber kebenaran untuk seluruh library IskakINO.
// Hanya mendukung platform target: AVR, ESP32, dan ESP8266.
// ============================================================================
#if defined(__AVR__)
  #define ISKAKINO_PLATFORM_AVR 1
#elif defined(ESP32)
  #define ISKAKINO_PLATFORM_ESP32 1
#elif defined(ESP8266)
  #define ISKAKINO_PLATFORM_ESP8266 1
#else
  #define ISKAKINO_PLATFORM_OTHER 1
#endif

// Kapabilitas turunan, dipakai modul seperti Storage/WifiPortal untuk
// memilih backend penyimpanan/WiFi yang tersedia.
#if defined(ISKAKINO_PLATFORM_ESP32)
  #define ISKAKINO_HAS_WIFI       1
  #define ISKAKINO_HAS_PREFS      1
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #define ISKAKINO_HAS_WIFI       1
  #define ISKAKINO_HAS_LITTLEFS   1
#else
  #define ISKAKINO_HAS_EEPROM     1
#endif

// Penanganan khusus jika board tidak mendefinisikan LED_BUILTIN
// (sering terjadi di board ESP32 pihak ketiga).
#ifndef LED_BUILTIN
  #define LED_BUILTIN 2
#endif

// ============================================================================
// FastPin<P> — Direct Register I/O
// ----------------------------------------------------------------------------
// AVR (Uno/Nano/Mega dll.)   : high()/low()/toggle()/read() bekerja langsung
//                               di register PORTx/PINx via bit-manipulation.
//                               toggle() memakai "AVR PINx write-toggle trick".
// ESP32                      : high()/low()/read()/toggle() memakai register
//                               write-1-to-set/clear (.out_w1ts.val / .out_w1tc.val,
//                               .in.val / .out.val). Untuk pin >= 32 pada chip
//                               yang memiliki >32 pin (ESP32 classic/S2/S3),
//                               memakai bank register kedua (.out1_* / .in1).
// ESP8266                    : high()/low() memakai register GPOS/GPOC
//                               (GPIO Output Set/Clear), dengan penanganan
//                               khusus untuk GPIO16 (register RTC terpisah).
// Platform lain / Fallback   : fallback aman ke pinMode()/digitalWrite()/
//                               digitalRead() standar.
// ============================================================================
template <uint8_t P>
class FastPin {
public:
    inline void mode(uint8_t m) __attribute__((always_inline)) {
#if defined(ISKAKINO_PLATFORM_AVR)
        volatile uint8_t *ddr  = (volatile uint8_t*)portModeRegister(digitalPinToPort(P));
        volatile uint8_t *port = (volatile uint8_t*)portOutputRegister(digitalPinToPort(P));
        const uint8_t bit = digitalPinToBitMask(P);
        if (m == OUTPUT) {
            *ddr |= bit;
        } else {
            *ddr &= ~bit;
            if (m == INPUT_PULLUP) {
                *port |= bit;
            } else {
                *port &= ~bit;
            }
        }
#else
        pinMode(P, m);
#endif
    }

    inline void high() __attribute__((always_inline)) {
#if defined(ISKAKINO_PLATFORM_AVR)
        *(volatile uint8_t*)portOutputRegister(digitalPinToPort(P)) |= digitalPinToBitMask(P);
#elif defined(ISKAKINO_PLATFORM_ESP32)
  #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C2) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
        GPIO.out_w1ts = (1UL << P);
  #else
        if (P < 32) {
            GPIO.out_w1ts = (1UL << P);
        } else {
            GPIO.out1_w1ts.val = (1UL << (P - 32));
        }
  #endif
#elif defined(ISKAKINO_PLATFORM_ESP8266)
        if (P < 16) {
            GPOS = (1UL << P);
        } else if (P == 16) {
            GP16O |= 1;
        } else {
            digitalWrite(P, HIGH);
        }
#else
        digitalWrite(P, HIGH);
#endif
    }

    inline void low() __attribute__((always_inline)) {
#if defined(ISKAKINO_PLATFORM_AVR)
        *(volatile uint8_t*)portOutputRegister(digitalPinToPort(P)) &= ~digitalPinToBitMask(P);
#elif defined(ISKAKINO_PLATFORM_ESP32)
  #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C2) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
        GPIO.out_w1tc = (1UL << P);
  #else
        if (P < 32) {
            GPIO.out_w1tc = (1UL << P);
        } else {
            GPIO.out1_w1tc.val = (1UL << (P - 32));
        }
  #endif
#elif defined(ISKAKINO_PLATFORM_ESP8266)
        if (P < 16) {
            GPOC = (1UL << P);
        } else if (P == 16) {
            GP16O &= ~1;
        } else {
            digitalWrite(P, LOW);
        }
#else
        digitalWrite(P, LOW);
#endif
    }

    inline void toggle() __attribute__((always_inline)) {
#if defined(ISKAKINO_PLATFORM_AVR)
        *(volatile uint8_t*)portInputRegister(digitalPinToPort(P)) = digitalPinToBitMask(P);
#elif defined(ISKAKINO_PLATFORM_ESP32)
  #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C2) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
        if (GPIO.out & (1UL << P)) {
            GPIO.out_w1tc = (1UL << P);
        } else {
            GPIO.out_w1ts = (1UL << P);
        }
  #else
        if (P < 32) {
            if (GPIO.out & (1UL << P)) {
                GPIO.out_w1tc = (1UL << P);
            } else {
                GPIO.out_w1ts = (1UL << P);
            }
        } else {
            if (GPIO.out1.val & (1UL << (P - 32))) {
                GPIO.out1_w1tc.val = (1UL << (P - 32));
            } else {
                GPIO.out1_w1ts.val = (1UL << (P - 32));
            }
        }
  #endif
#else
        if (read()) low(); else high();
#endif
    }

    inline bool read() __attribute__((always_inline)) {
#if defined(ISKAKINO_PLATFORM_AVR)
        return (*(volatile uint8_t*)portInputRegister(digitalPinToPort(P)) & digitalPinToBitMask(P)) != 0;
#elif defined(ISKAKINO_PLATFORM_ESP32)
  #if defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32C2) || defined(CONFIG_IDF_TARGET_ESP32C6) || defined(CONFIG_IDF_TARGET_ESP32H2)
        return (GPIO.in & (1UL << P)) != 0;
  #else
        if (P < 32) {
            return (GPIO.in & (1UL << P)) != 0;
        } else {
            return (GPIO.in1.val & (1UL << (P - 32))) != 0;
        }
  #endif
#elif defined(ISKAKINO_PLATFORM_ESP8266)
        if (P < 16) {
            return (GPI & (1UL << P)) != 0;
        } else if (P == 16) {
            return (GP16I & 1) != 0;
        } else {
            return digitalRead(P);
        }
#else
        return digitalRead(P);
#endif
    }
};

#endif
