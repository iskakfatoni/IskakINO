#ifndef MOCK_ARDUINO_EXTRA_H
#define MOCK_ARDUINO_EXTRA_H
#include <string>
#include <cstdlib>
#include <cstdint>

// --- Mock minimal kelas String milik Arduino, cukup untuk syntax-check ---
class String {
public:
    std::string s;
    String() {}
    String(const char* c) : s(c ? c : "") {}
    String(char c) : s(1, c) {}
    String(int v) : s(std::to_string(v)) {}
    String(unsigned int v) : s(std::to_string(v)) {}
    String(long v) : s(std::to_string(v)) {}
    String(unsigned long v) : s(std::to_string(v)) {}

    unsigned int length() const { return (unsigned int)s.length(); }
    const char* c_str() const { return s.c_str(); }
    void reserve(unsigned int n) { s.reserve(n); }
    void trim() {
        size_t a = s.find_first_not_of(" \t\r\n");
        size_t b = s.find_last_not_of(" \t\r\n");
        s = (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
    }
    int indexOf(char c) const { auto p = s.find(c); return p == std::string::npos ? -1 : (int)p; }
    int indexOf(const char* c) const { auto p = s.find(c); return p == std::string::npos ? -1 : (int)p; }
    String substring(int from) const { return String(s.substr(from).c_str()); }
    String substring(int from, int to) const { return String(s.substr(from, to - from).c_str()); }
    char operator[](unsigned int i) const { return s[i]; }

    String& operator+=(const String& o) { s += o.s; return *this; }
    String& operator+=(char c) { s += c; return *this; }
    friend String operator+(String a, const String& b) { a.s += b.s; return a; }
    friend String operator+(String a, const char* b) { a.s += b; return a; }
    friend String operator+(const char* a, String b) { return String(a) + b; }

    bool operator==(const char* o) const { return s == o; }
    bool operator==(const String& o) const { return s == o.s; }
    bool operator!=(const char* o) const { return s != o; }
};

// --- ESP object (restart/getFreeHeap/getFreeSketchSpace) ---
class ESPClass {
public:
    int restartCount = 0;
    void restart() { restartCount++; }
    uint32_t getFreeHeap() { return 100000; }
    uint32_t getFreeSketchSpace() { return 500000; }
};
extern ESPClass ESP;

inline void delay(unsigned long) {}
inline void delayMicroseconds(unsigned int) {}

// --- Dipakai FastNTP: word()/byte (tipe & fungsi bawaan Arduino core) ---
typedef uint8_t byte;
inline uint16_t word(uint8_t hi, uint8_t lo) { return ((uint16_t)hi << 8) | lo; }
#ifndef A0
#define A0 14
#endif


// --- Mock GPIO struct ala soc/gpio_struct.h, HANYA untuk memuaskan
// lookup nama non-dependent di badan template FastPin<P> (lihat
// core/IskakINO_Platform.h) supaya file ini bisa di-syntax-check.
// TIDAK memvalidasi kebenaran akses register ESP32 sungguhan — itu
// sudah di luar cakupan pilot WifiPortal ini (FastPin sendiri hanya
// bisa diuji sungguhan di board fisik, sama seperti precedent di
// ArduFast: lihat komentar di test/native_check/Arduino.h).
struct MockGpioBank { unsigned long val = 0; };
struct MockGpioStruct {
    unsigned long out = 0, in = 0, out_w1ts = 0, out_w1tc = 0;
    MockGpioBank out1, in1, out1_w1ts, out1_w1tc;
};
extern MockGpioStruct GPIO;

#endif

// --- Mock register ESP8266 (GPOS/GPOC/GPI/GP16O/GP16I), HANYA untuk
// memuaskan lookup nama non-dependent di FastPin<P> (sama alasan seperti
// MockGpioStruct untuk ESP32 di atas — bukan validasi register sungguhan).
#ifndef GPOS
extern unsigned long GPOS, GPOC, GPI, GP16O, GP16I;
#endif
