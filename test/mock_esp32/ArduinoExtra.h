#ifndef MOCK_ARDUINO_EXTRA_H
#define MOCK_ARDUINO_EXTRA_H
#include <string>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <cctype>

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
    bool isEmpty() const { return s.empty(); }
    void clear() { s.clear(); }
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

    long toInt() const { return s.empty() ? 0 : std::strtol(s.c_str(), nullptr, 10); }
    float toFloat() const { return s.empty() ? 0.0f : std::strtof(s.c_str(), nullptr); }
    double toDouble() const { return s.empty() ? 0.0 : std::strtod(s.c_str(), nullptr); }

    bool startsWith(const String& prefix) const {
        return s.rfind(prefix.s, 0) == 0;
    }
    bool startsWith(const char* prefix) const {
        return prefix && s.rfind(prefix, 0) == 0;
    }
    bool endsWith(const String& suffix) const {
        if (suffix.s.length() > s.length()) return false;
        return s.compare(s.length() - suffix.s.length(), suffix.s.length(), suffix.s) == 0;
    }
    bool endsWith(const char* suffix) const {
        if (!suffix) return false;
        std::string sf(suffix);
        if (sf.length() > s.length()) return false;
        return s.compare(s.length() - sf.length(), sf.length(), sf) == 0;
    }

    bool equalsIgnoreCase(const String& other) const {
        if (s.length() != other.s.length()) return false;
        for (size_t i = 0; i < s.length(); i++) {
            if (std::tolower((unsigned char)s[i]) != std::tolower((unsigned char)other.s[i])) return false;
        }
        return true;
    }

    void toLowerCase() {
        for (char &c : s) c = (char)std::tolower((unsigned char)c);
    }
    void toUpperCase() {
        for (char &c : s) c = (char)std::toupper((unsigned char)c);
    }

    bool concat(const String& str) { s += str.s; return true; }
    bool concat(const char* cstr) { if (cstr) s += cstr; return true; }
    bool concat(char c) { s += c; return true; }

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
