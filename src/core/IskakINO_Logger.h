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
 *
 * Setiap modul disarankan punya instance IskakINO_Logger sendiri (bukan
 * satu global tunggal) supaya debug bisa diaktifkan per-modul, mis.:
 *   IskakINO_Logger _log;
 *   _log.setDebug(true);   // hanya modul ini yang jadi cerewet di Serial
 */

#ifndef ISKAKINO_LOGGER_H
#define ISKAKINO_LOGGER_H

#include <Arduino.h>
#include <stdarg.h>
#include "IskakINO_Result.h"

// AVR gotcha: avr-libc <math.h> (ke-include transitif lewat Arduino.h)
// mendefinisikan `log`/`logf` sebagai MACRO saling terhubung, karena di AVR
// double==float (4 byte) jadi avr-libc cuma punya SATU implementasi nyata
// dan yang satunya lagi macro alias. TERKONFIRMASI lewat CI sungguhan
// arduino:avr:uno bahwa arahnya `#define logf log` (bukan sebaliknya
// seperti dugaan awal) -- makanya method logf() (variadic) di bawah diam-
// diam "dibajak" jadi log(), bentrok-ambigu dengan method log() yang sudah
// ada. Supaya aman terlepas dari arah macro-nya (bisa beda antar versi
// avr-libc), KEDUA nama di-#undef di sini -- fix terpusat di sini otomatis
// berlaku ke semua modul yang compose Logger (Storage, LCD, SmartVoice),
// bukan cuma ArduFast. Di platform lain (ESP32/ESP8266, double asli) macro
// ini biasanya tidak ada, jadi #ifdef di bawah aman jadi no-op.
#ifdef log
#undef log
#endif
#ifdef logf
#undef logf
#endif

// Ukuran buffer stack untuk logf() (printf-style). Bisa dikecilkan untuk
// board AVR yang RAM-nya sangat terbatas dengan mendefinisikan makro ini
// SEBELUM #include <IskakINO_Logger.h>, mis.:
//   #define ISKAKINO_LOGF_BUFFER_SIZE 32
//   #include <IskakINO_Logger.h>
#ifndef ISKAKINO_LOGF_BUFFER_SIZE
#define ISKAKINO_LOGF_BUFFER_SIZE 64
#endif

class IskakINO_Logger {
  private:
    bool _debug = false;

  public:
    IskakINO_Logger() {}

    // Aktif/nonaktifkan output ke Serial. Default nonaktif (silent) supaya
    // modul yang compose Logger ini tidak otomatis cerewet tanpa diminta.
    void setDebug(bool debugMode) { _debug = debugMode; }
    bool isDebug() const { return _debug; }

    void log(const __FlashStringHelper* msg) {
        if (!_debug) return;
        Serial.print(F("[LOG] "));
        Serial.println(msg);
    }

    void log(const __FlashStringHelper* msg, long val) {
        if (!_debug) return;
        Serial.print(F("[LOG] "));
        Serial.print(msg);
        Serial.print(F(": "));
        Serial.println(val);
    }

    // Pakai dtostrf() (tersedia di avr-libc & disediakan ulang oleh core
    // ESP8266/ESP32) supaya tidak bergantung pada dukungan %f di vsnprintf,
    // yang tidak tersedia secara default di AVR.
    void logFloat(const __FlashStringHelper* msg, float val, uint8_t decimals = 2) {
        if (!_debug) return;
        char buf[16];
        dtostrf(val, 0, decimals, buf);
        Serial.print(F("[LOG] "));
        Serial.print(msg);
        Serial.print(F(": "));
        Serial.println(buf);
    }

    // Printf-style logging. Format string HARUS __FlashStringHelper (F()).
    // CATATAN AVR: specifier %f TIDAK didukung oleh vsnprintf bawaan AVR
    // tanpa flag linker tambahan (-lprintf_flt) — untuk nilai desimal
    // pakai logFloat().
    void logf(const __FlashStringHelper* fmt, ...) {
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

    // Cetak IskakINO_Result secara konsisten, mis.:
    //   _log.logResult(F("Storage.save"), IskakINO_Result::WRITE_FAILED);
    //   -> "[LOG] Storage.save: WRITE_FAILED"
    void logResult(const __FlashStringHelper* msg, IskakINO_Result result) {
        if (!_debug) return;
        Serial.print(F("[LOG] "));
        Serial.print(msg);
        Serial.print(F(": "));
        Serial.println(IskakINO_ResultToString(result));
    }
};

#endif
