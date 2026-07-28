// test/mock/Arduino.h
//
// Mock minimal Arduino core untuk menguji LOGIKA platform-independent
// IskakINO_ArduFast (task manager, analog wrapper, logging) secara native
// di host CI — tanpa toolchain AVR/ESP8266/ESP32 dan tanpa hardware fisik.
//
// TIDAK dipakai untuk menguji FastPin<P> (register access), karena itu
// murni platform-specific dan hanya bermakna diuji di board sungguhan
// (lihat 12_ESP32_HighPin_Test.ino sebagai jaring pengaman compile-time
// untuk jalur register ESP32).
#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <cstdint>
#include <cstdio>
#include <cstring>

using std::vsnprintf;
using std::snprintf;

// --- Konstanta digital ---
#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2
#define HIGH 1
#define LOW 0
#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

// --- Jam palsu yang bisa dikendalikan dari test ---
extern unsigned long _mock_millis_value;
inline unsigned long millis() { return _mock_millis_value; }

// --- Stub digital/analog I/O ---
// Tidak mensimulasikan state pin sungguhan — cukup aman untuk compile dan
// untuk menguji logika non-register (task manager, analog wrapper).
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t) {}
inline uint8_t digitalRead(uint8_t) { return LOW; }

extern int _mock_analog_value;
inline int analogRead(uint8_t) { return _mock_analog_value; }

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// --- Emulasi Flash string (host tidak punya PROGMEM) ---
class __FlashStringHelper;
#define F(str) (reinterpret_cast<const __FlashStringHelper*>(str))
#define PROGMEM
inline char* dtostrf(double val, signed char width, unsigned char prec, char* sbuf) {
    snprintf(sbuf, 32, "%*.*f", width, prec, val);
    return sbuf;
}

// --- Serial mock: dicetak ke stdout supaya kelihatan di log CI ---
class MockSerial {
public:
    void begin(unsigned long) {}
    void print(const __FlashStringHelper* s)  { std::printf("%s", reinterpret_cast<const char*>(s)); }
    void print(const char* s)                 { std::printf("%s", s); }
    void println(const __FlashStringHelper* s){ std::printf("%s\n", reinterpret_cast<const char*>(s)); }
    void println(const char* s)               { std::printf("%s\n", s); }
    void println(long v)                      { std::printf("%ld\n", v); }
    void println(int v)                       { std::printf("%d\n", v); }
    void println(bool v)                      { std::printf("%d\n", v ? 1 : 0); }
    // Dipakai untuk Serial.println(val, HEX) — mis. di IskakINO LCD.
    void println(unsigned int v, int base) {
        if (base == 16) std::printf("%X\n", v);
        else std::printf("%d\n", v);
    }
    // Overload String ditambahkan supaya mock ini juga bisa dipakai untuk
    // syntax-check modul lain (mis. WifiPortal) yang memakai Arduino String.
    // Forward-declare via template SFINAE-friendly overload eksplisit
    // (bukan template umum) supaya tidak "merebut" panggilan dengan
    // const char[]/int yang seharusnya jatuh ke overload non-template lain.
    template <typename T> void print(const T& s)   { printImpl(s, 0); }
    template <typename T> void println(const T& s) { printlnImpl(s, 0); }
  private:
    // Overload dgn parameter dummy int: versi yang butuh .c_str() menang
    // hanya kalau tipe argumen benar2 punya method itu (SFINAE via decltype).
    template <typename T> auto printImpl(const T& s, int) -> decltype(s.c_str(), void()) {
        std::printf("%s", s.c_str());
    }
    void printImpl(...) {}
    template <typename T> auto printlnImpl(const T& s, int) -> decltype(s.c_str(), void()) {
        std::printf("%s\n", s.c_str());
    }
    void printlnImpl(...) {}
  public:};
extern MockSerial Serial;

#endif
