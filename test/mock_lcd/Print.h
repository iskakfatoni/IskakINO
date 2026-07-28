#ifndef MOCK_PRINT_H
#define MOCK_PRINT_H
#include <Arduino.h>
#include <cstdio>
#include <cstring>

#define DEC 10
#define HEX 16

class Print {
public:
    virtual size_t write(uint8_t) = 0;
    virtual ~Print() {}

    size_t write(const char* s) {
        size_t n = 0;
        while (*s) { write((uint8_t)*s++); n++; }
        return n;
    }

    size_t print(const char* s) { return write(s); }
    size_t print(char c) { return write((uint8_t)c); }
    size_t print(const String& s) { return write(s.c_str()); }
    size_t print(uint8_t v, int base) {
        char buf[8];
        if (base == HEX) std::snprintf(buf, sizeof(buf), "%X", v);
        else std::snprintf(buf, sizeof(buf), "%d", v);
        return write(buf);
    }
    size_t print(int v) { char buf[16]; std::snprintf(buf, sizeof(buf), "%d", v); return write(buf); }
    // Overload F()-string, standar di Print asli Arduino.
    size_t print(const __FlashStringHelper* s) { return write((const char*)s); }
    size_t println(const __FlashStringHelper* s) { size_t n = write((const char*)s); write((uint8_t)'\n'); return n + 1; }

    size_t println(const char* s) { size_t n = write(s); write((uint8_t)'\n'); return n + 1; }
};
#endif
