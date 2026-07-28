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
// ============================================================================
#if defined(__AVR__)
  #define ISKAKINO_PLATFORM_AVR 1
#elif defined(ESP32)
  #define ISKAKINO_PLATFORM_ESP32 1
#elif defined(ESP8266)
  #define ISKAKINO_PLATFORM_ESP8266 1
#elif defined(ARDUINO_ARCH_RP2040)
  #define ISKAKINO_PLATFORM_RP2040 1
#else
  #define ISKAKINO_PLATFORM_OTHER 1
#endif

// Kapabilitas turunan, dipakai modul seperti Storage/WifiPortal untuk
// memilih backend penyimpanan/WiFi yang tersedia tanpa menulis ulang
// rantai #if defined() masing-masing.
#if defined(ISKAKINO_PLATFORM_ESP32)
  #define ISKAKINO_HAS_WIFI       1
  #define ISKAKINO_HAS_PREFS      1
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #define ISKAKINO_HAS_WIFI       1
  #define ISKAKINO_HAS_LITTLEFS   1
#elif defined(ISKAKINO_PLATFORM_RP2040)
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
//                               toggle() memakai "AVR PINx write-toggle trick"
//                               (menulis 1 ke register PINx membalik output
//                               dalam 1 siklus clock, tanpa read-modify-write).
// ESP32                      : high()/low()/read()/toggle() memakai register
//                               write-1-to-set/clear (GPIO.out_w1ts/out_w1tc,
//                               GPIO.in, GPIO.out) untuk pin 0-31, dan bank
//                               register kedua (out1_w1ts/out1_w1tc/in1/out1)
//                               untuk pin >=32 (GPIO32-39 dst.) — layout
//                               standar soc/gpio_struct.h milik ESP-IDF.
//                               toggle() sengaja membaca register OUTPUT
//                               (bukan INPUT) agar tetap andal untuk pin
//                               yang murni dipakai sebagai output.
// ESP8266                    : high()/low() memakai register GPOS/GPOC
//                               (GPIO Output Set/Clear), dengan penanganan
//                               khusus untuk GPIO16 (register RTC terpisah).
// Platform lain               : fallback aman ke pinMode()/digitalWrite()/
//                               digitalRead() standar agar tetap kompatibel
//                               dan tetap benar secara fungsional.
//
// CATATAN: mode() tetap memakai pinMode() standar di semua platform. Ini
// karena konfigurasi arah pin biasanya hanya dipanggil sekali di setup()
// (bukan hot path), sedangkan high()/low()/toggle()/read() adalah operasi
// yang dipanggil berulang-ulang di loop() sehingga paling diuntungkan dari
// akses register langsung.
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
        if (P < 32) {
            GPIO.out_w1ts = (1UL << P);
        } else {
            GPIO.out1_w1ts.val = (1UL << (P - 32));
        }
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
        if (P < 32) {
            GPIO.out_w1tc = (1UL << P);
        } else {
            GPIO.out1_w1tc.val = (1UL << (P - 32));
        }
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
        // Trik hardware AVR: menulis 1 ke register PINx (bukan PORTx)
        // membalik bit output yang bersangkutan dalam 1 siklus clock,
        // tanpa perlu read-modify-write seperti pendekatan biasa.
        *(volatile uint8_t*)portInputRegister(digitalPinToPort(P)) = digitalPinToBitMask(P);
#elif defined(ISKAKINO_PLATFORM_ESP32)
        // Baca status dari register OUTPUT (GPIO.out), bukan register INPUT
        // (GPIO.in) seperti fallback generik — supaya tetap benar walau
        // input buffer pin dinonaktifkan/tidak stabil untuk pin yang murni
        // dipakai sebagai output.
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
#else
        if (read()) low(); else high();
#endif
    }

    inline bool read() __attribute__((always_inline)) {
#if defined(ISKAKINO_PLATFORM_AVR)
        return (*(volatile uint8_t*)portInputRegister(digitalPinToPort(P)) & digitalPinToBitMask(P)) != 0;
#elif defined(ISKAKINO_PLATFORM_ESP32)
        if (P < 32) {
            return (GPIO.in & (1UL << P)) != 0;
        } else {
            return (GPIO.in1.val & (1UL << (P - 32))) != 0;
        }
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
