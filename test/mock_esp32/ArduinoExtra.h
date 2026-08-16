#ifndef MOCK_ARDUINO_EXTRA_H
#define MOCK_ARDUINO_EXTRA_H

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

// Forward declaration untuk emulasi Flash string
class __FlashStringHelper;

// --- Mock kelas String milik Arduino (lengkap & robust untuk test/syntax
// check) ---
class String {
public:
  std::string s;

  // Konstruktor
  String() {}
  String(const char *c) : s(c ? c : "") {}
  String(char c) : s(1, c) {}
  String(unsigned char c) : s(1, (char)c) {}
  String(int v) : s(std::to_string(v)) {}
  String(unsigned int v) : s(std::to_string(v)) {}
  String(long v) : s(std::to_string(v)) {}
  String(unsigned long v) : s(std::to_string(v)) {}
  String(bool v) : s(v ? "1" : "0") {}
  String(const __FlashStringHelper *f)
      : s(f ? reinterpret_cast<const char *>(f) : "") {}
  String(float v, unsigned char decimalPlaces = 2) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*f", (int)decimalPlaces, (double)v);
    s = buf;
  }
  String(double v, unsigned char decimalPlaces = 2) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*f", (int)decimalPlaces, v);
    s = buf;
  }

  // Informasi dasar
  unsigned int length() const { return (unsigned int)s.length(); }
  bool isEmpty() const { return s.empty(); }
  void clear() { s.clear(); }
  const char *c_str() const { return s.c_str(); }
  void reserve(unsigned int n) { s.reserve(n); }

  // Mutasi string
  void trim() {
    size_t a = s.find_first_not_of(" \t\r\n");
    size_t b = s.find_last_not_of(" \t\r\n");
    s = (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
  }

  // Index & pencarian (aman terhadap null & out-of-bounds)
  int indexOf(char c, unsigned int fromIndex = 0) const {
    if (fromIndex >= s.length())
      return -1;
    auto p = s.find(c, fromIndex);
    return p == std::string::npos ? -1 : (int)p;
  }
  int indexOf(const char *c, unsigned int fromIndex = 0) const {
    if (!c || fromIndex >= s.length())
      return -1;
    auto p = s.find(c, fromIndex);
    return p == std::string::npos ? -1 : (int)p;
  }
  int indexOf(const String &str, unsigned int fromIndex = 0) const {
    return indexOf(str.c_str(), fromIndex);
  }

  int lastIndexOf(char c, unsigned int fromIndex = ~0U) const {
    if (s.empty())
      return -1;
    size_t pos =
        (fromIndex >= s.length()) ? std::string::npos : (size_t)fromIndex;
    auto p = s.rfind(c, pos);
    return p == std::string::npos ? -1 : (int)p;
  }
  int lastIndexOf(const char *c, unsigned int fromIndex = ~0U) const {
    if (!c || s.empty())
      return -1;
    size_t pos =
        (fromIndex >= s.length()) ? std::string::npos : (size_t)fromIndex;
    auto p = s.rfind(c, pos);
    return p == std::string::npos ? -1 : (int)p;
  }
  int lastIndexOf(const String &str, unsigned int fromIndex = ~0U) const {
    return lastIndexOf(str.c_str(), fromIndex);
  }

  // Substring aman (tanpa melempar out_of_range)
  String substring(int from) const {
    if (from < 0)
      from = 0;
    if ((size_t)from >= s.length())
      return String("");
    return String(s.substr(from).c_str());
  }
  String substring(int from, int to) const {
    if (from < 0)
      from = 0;
    if (to < 0)
      to = 0;
    if (from > to)
      std::swap(from, to);
    if ((size_t)from >= s.length())
      return String("");
    if ((size_t)to > s.length())
      to = (int)s.length();
    return String(s.substr(from, to - from).c_str());
  }

  // Akses karakter
  char charAt(unsigned int index) const {
    return index < s.length() ? s[index] : '\0';
  }
  void setCharAt(unsigned int index, char c) {
    if (index < s.length())
      s[index] = c;
  }
  char &operator[](unsigned int i) { return s[i]; }
  char operator[](unsigned int i) const { return s[i]; }

  // Konversi nilai
  long toInt() const {
    return s.empty() ? 0 : std::strtol(s.c_str(), nullptr, 10);
  }
  float toFloat() const {
    return s.empty() ? 0.0f : std::strtof(s.c_str(), nullptr);
  }
  double toDouble() const {
    return s.empty() ? 0.0 : std::strtod(s.c_str(), nullptr);
  }

  // Pengecekan prefix & suffix
  bool startsWith(const String &prefix) const {
    return s.rfind(prefix.s, 0) == 0;
  }
  bool startsWith(const char *prefix) const {
    return prefix && s.rfind(prefix, 0) == 0;
  }
  bool endsWith(const String &suffix) const {
    if (suffix.s.length() > s.length())
      return false;
    return s.compare(s.length() - suffix.s.length(), suffix.s.length(),
                     suffix.s) == 0;
  }
  bool endsWith(const char *suffix) const {
    if (!suffix)
      return false;
    std::string sf(suffix);
    if (sf.length() > s.length())
      return false;
    return s.compare(s.length() - sf.length(), sf.length(), sf) == 0;
  }

  // Case handling & comparison
  bool equalsIgnoreCase(const String &other) const {
    if (s.length() != other.s.length())
      return false;
    for (size_t i = 0; i < s.length(); i++) {
      if (std::tolower((unsigned char)s[i]) !=
          std::tolower((unsigned char)other.s[i]))
        return false;
    }
    return true;
  }
  bool equals(const String &other) const { return s == other.s; }
  bool equals(const char *other) const {
    return other ? (s == other) : s.empty();
  }
  int compareTo(const String &other) const { return s.compare(other.s); }

  void toLowerCase() {
    for (char &c : s)
      c = (char)std::tolower((unsigned char)c);
  }
  void toUpperCase() {
    for (char &c : s)
      c = (char)std::toupper((unsigned char)c);
  }

  // Manipulasi lanjutan: replace, remove, buffer copy
  void replace(char find, char repl) {
    for (char &c : s) {
      if (c == find)
        c = repl;
    }
  }
  void replace(const String &find, const String &repl) {
    if (find.isEmpty())
      return;
    size_t pos = 0;
    while ((pos = s.find(find.s, pos)) != std::string::npos) {
      s.replace(pos, find.s.length(), repl.s);
      pos += repl.s.length();
    }
  }
  void replace(const char *find, const char *repl) {
    if (!find || !repl)
      return;
    replace(String(find), String(repl));
  }

  void remove(unsigned int index, unsigned int count = ~0U) {
    if (index >= s.length())
      return;
    if (count == ~0U || index + count > s.length()) {
      s.erase(index);
    } else {
      s.erase(index, count);
    }
  }

  void toCharArray(char *buf, unsigned int bufsize,
                   unsigned int index = 0) const {
    if (!buf || bufsize == 0)
      return;
    if (index >= s.length()) {
      buf[0] = '\0';
      return;
    }
    unsigned int copyLen =
        (unsigned int)std::min((size_t)(bufsize - 1), s.length() - index);
    std::memcpy(buf, s.data() + index, copyLen);
    buf[copyLen] = '\0';
  }

  void getBytes(unsigned char *buf, unsigned int bufsize,
                unsigned int index = 0) const {
    toCharArray(reinterpret_cast<char *>(buf), bufsize, index);
  }

  // Penggabungan (concat & operator+=)
  bool concat(const String &str) {
    s += str.s;
    return true;
  }
  bool concat(const char *cstr) {
    if (cstr)
      s += cstr;
    return true;
  }
  bool concat(char c) {
    s += c;
    return true;
  }
  bool concat(int num) {
    s += std::to_string(num);
    return true;
  }
  bool concat(unsigned int num) {
    s += std::to_string(num);
    return true;
  }
  bool concat(long num) {
    s += std::to_string(num);
    return true;
  }
  bool concat(unsigned long num) {
    s += std::to_string(num);
    return true;
  }

  String &operator+=(const String &o) {
    s += o.s;
    return *this;
  }
  String &operator+=(const char *o) {
    if (o)
      s += o;
    return *this;
  }
  String &operator+=(char c) {
    s += c;
    return *this;
  }
  String &operator+=(int v) {
    s += std::to_string(v);
    return *this;
  }
  String &operator+=(unsigned int v) {
    s += std::to_string(v);
    return *this;
  }
  String &operator+=(long v) {
    s += std::to_string(v);
    return *this;
  }
  String &operator+=(unsigned long v) {
    s += std::to_string(v);
    return *this;
  }
  String &operator+=(float v) {
    *this += String(v);
    return *this;
  }
  String &operator+=(double v) {
    *this += String(v);
    return *this;
  }

  // Operator biner +
  friend String operator+(String a, const String &b) {
    a.s += b.s;
    return a;
  }
  friend String operator+(String a, const char *b) {
    if (b)
      a.s += b;
    return a;
  }
  friend String operator+(const char *a, const String &b) {
    return String(a ? a : "") + b;
  }
  friend String operator+(String a, char b) {
    a.s += b;
    return a;
  }
  friend String operator+(char a, const String &b) { return String(a) + b; }
  friend String operator+(String a, int b) {
    a.s += std::to_string(b);
    return a;
  }
  friend String operator+(int a, const String &b) { return String(a) + b; }

  // Operator perbandingan
  bool operator==(const char *o) const { return o ? (s == o) : s.empty(); }
  bool operator==(const String &o) const { return s == o.s; }
  bool operator!=(const char *o) const { return o ? (s != o) : !s.empty(); }
  bool operator!=(const String &o) const { return s != o.s; }

  friend bool operator==(const char *a, const String &b) { return b == a; }
  friend bool operator!=(const char *a, const String &b) { return b != a; }

  bool operator<(const String &o) const { return s < o.s; }
  bool operator>(const String &o) const { return s > o.s; }
  bool operator<=(const String &o) const { return s <= o.s; }
  bool operator>=(const String &o) const { return s >= o.s; }
};

// --- ESP object (mock kaya fitur untuk ESP32/ESP8266) ---
class ESPClass {
public:
  int restartCount = 0;
  void restart() { restartCount++; }

  uint32_t getFreeHeap() const { return 100000; }
  uint32_t getHeapSize() const { return 320000; }
  uint32_t getMinFreeHeap() const { return 80000; }
  uint32_t getMaxAllocHeap() const { return 60000; }
  uint32_t getFreeSketchSpace() const { return 500000; }
  uint32_t getFlashChipSize() const { return 4194304; }
  uint32_t getFlashChipSpeed() const { return 80000000; }

  const char *getChipModel() const { return "ESP32-D0WDQ6"; }
  uint8_t getChipRevision() const { return 1; }
  uint32_t getCpuFreqMHz() const { return 240; }
  const char *getSdkVersion() const { return "v4.4-mock"; }
  uint64_t getEfuseMac() const { return 0x112233445566ULL; }

  void deepSleep(uint64_t time_us = 0) { (void)time_us; }
  void feedWatchdog() {}
  void wdtFeed() {}
  void wdtDisable() {}
  void wdtEnable(uint32_t = 0) {}
};
extern ESPClass ESP;

// --- Fungsi utilitas global bawaan core Arduino/ESP ---
inline void delay(unsigned long) {}
inline void delayMicroseconds(unsigned int) {}
inline void yield() {}

// --- Tipe & fungsi pembantu Arduino core ---
typedef uint8_t byte;
typedef bool boolean;

inline uint16_t word(uint8_t hi, uint8_t lo) {
  return ((uint16_t)hi << 8) | lo;
}
inline uint16_t word(uint16_t w) { return w; }

// --- Pin Analog & Spesifik ESP ---
#ifndef A0
#define A0 14
#endif
#ifndef A1
#define A1 15
#endif
#ifndef A2
#define A2 16
#endif
#ifndef A3
#define A3 17
#endif
#ifndef A4
#define A4 18
#endif
#ifndef A5
#define A5 19
#endif
#ifndef A6
#define A6 20
#endif
#ifndef A7
#define A7 21
#endif
#ifndef DAC1
#define DAC1 25
#endif
#ifndef DAC2
#define DAC2 26
#endif

// --- Mock GPIO struct ala soc/gpio_struct.h (ESP32) ---
struct MockGpioBank {
  unsigned long val = 0;
};
struct MockGpioStruct {
  unsigned long out = 0, in = 0, out_w1ts = 0, out_w1tc = 0;
  unsigned long enable = 0, enable_w1ts = 0, enable_w1tc = 0;
  unsigned long status = 0, status_w1ts = 0, status_w1tc = 0;
  MockGpioBank out1, in1, out1_w1ts, out1_w1tc;
  MockGpioBank enable1, enable1_w1ts, enable1_w1tc;
  MockGpioBank status1, status1_w1ts, status1_w1tc;
};
extern MockGpioStruct GPIO;

// --- Mock register ESP8266 (GPOS/GPOC/GPI/GP16O/GP16I) ---
#ifndef GPOS
extern unsigned long GPOS, GPOC, GPI, GP16O, GP16I;
#endif

#endif // MOCK_ARDUINO_EXTRA_H
