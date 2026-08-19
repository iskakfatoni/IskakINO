/*
 * src/core/IskakINO_Logger.h
 * Logging printf-style terpusat, dengan flag debug per-instance.
 *
 * SUMBER: diekstrak dari log()/logFloat()/logf() di IskakINO_ArduFast v1.1.0.
 * Bedanya dengan versi ArduFast:
 *   - Dijadikan kelas mandiri (bukan menempel di IskakINO_ArduFast) supaya
 *     bisa dipakai modul lain (WifiPortal, FastNTP, Storage, SmartVoice)
 *     tanpa harus depend ke seluruh IskakINO_ArduFast.
 *   - Ditambah flag _debug (pola sama seperti IskakINO_Storage::setDebug())
 *     supaya modul yang punya mode "silent" tetap bisa pasang Logger tanpa
 *     otomatis ngeprint ke Serial kalau user tidak mengaktifkan debug.
 *   - Ditambah logResult() untuk mencetak IskakINO_Result secara konsisten.
 *   - Fitur Zero-Cost Release: Jika ISKAKINO_DISABLE_LOGGING didefinisikan,
 *     seluruh method logger di-inline sebagai no-op (0 byte Flash/RAM).
 */

#ifndef ISKAKINO_LOGGER_H
#define ISKAKINO_LOGGER_H

#include <Arduino.h>
#include <stdarg.h>
#include "IskakINO_Result.h"

// AVR gotcha: avr-libc <math.h> (ke-include transitif lewat Arduino.h)
// mendefinisikan `log`/`logf` sebagai MACRO saling terhubung
#ifdef log
#undef log
#endif
#ifdef logf
#undef logf
#endif

// Ukuran buffer stack untuk logf() (printf-style).
#ifndef ISKAKINO_LOGF_BUFFER_SIZE
#define ISKAKINO_LOGF_BUFFER_SIZE 64
#endif

#ifdef ISKAKINO_DISABLE_LOGGING

class IskakINO_Logger {
  public:
    IskakINO_Logger() {}
    inline void setDebug(bool) {}
    inline bool isDebug() const { return false; }
    inline void log(const __FlashStringHelper*) {}
    inline void log(const char*) {}
    inline void log(const String&) {}
    inline void log(const __FlashStringHelper*, long) {}
    inline void log(const char*, long) {}
    inline void logFloat(const __FlashStringHelper*, float, uint8_t = 2) {}
    inline void logf(const __FlashStringHelper*, ...) {}
    inline void logResult(const __FlashStringHelper*, IskakINO_Result) {}
};

#else

class IskakINO_Logger {
  private:
    bool _debug = false;

  public:
    IskakINO_Logger() {}

    // Aktif/nonaktifkan output ke Serial. Default nonaktif (silent).
    inline void setDebug(bool debugMode) { _debug = debugMode; }
    inline bool isDebug() const { return _debug; }

    inline void log(const __FlashStringHelper* msg) {
        if (!_debug) return;
        Serial.print(F("[LOG] "));
        Serial.println(msg);
    }

    inline void log(const char* msg) {
        if (!_debug || !msg) return;
        Serial.print(F("[LOG] "));
        Serial.println(msg);
    }

    inline void log(const String& msg) {
        if (!_debug) return;
        Serial.print(F("[LOG] "));
        Serial.println(msg);
    }

    inline void log(const __FlashStringHelper* msg, long val) {
        if (!_debug) return;
        Serial.print(F("[LOG] "));
        Serial.print(msg);
        Serial.print(F(": "));
        Serial.println(val);
    }

    inline void log(const char* msg, long val) {
        if (!_debug || !msg) return;
        Serial.print(F("[LOG] "));
        Serial.print(msg);
        Serial.print(F(": "));
        Serial.println(val);
    }

    inline void logFloat(const __FlashStringHelper* msg, float val, uint8_t decimals = 2) {
        if (!_debug) return;
        char buf[16];
        dtostrf(val, 0, decimals, buf);
        Serial.print(F("[LOG] "));
        Serial.print(msg);
        Serial.print(F(": "));
        Serial.println(buf);
    }

    inline void logf(const __FlashStringHelper* fmt, ...) {
        if (!_debug) return;
        char buf[ISKAKINO_LOGF_BUFFER_SIZE];
        va_list args;
        va_start(args, fmt);
#if defined(__AVR__)
        vsnprintf_P(buf, sizeof(buf), (const char*)fmt, args);
#else
        vsnprintf(buf, sizeof(buf), (const char*)fmt, args);
#endif
        va_end(args);
        Serial.print(F("[LOG] "));
        Serial.println(buf);
    }

    inline void logResult(const __FlashStringHelper* msg, IskakINO_Result result) {
        if (!_debug) return;
        Serial.print(F("[LOG] "));
        Serial.print(msg);
        Serial.print(F(": "));
        Serial.println(IskakINO_ResultToString(result));
    }
};

#endif

#endif
