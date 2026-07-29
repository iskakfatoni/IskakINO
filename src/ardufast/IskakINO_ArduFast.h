//
#ifndef ISKAKINO_ARDUFAST_H
#define ISKAKINO_ARDUFAST_H

#include <Arduino.h>

#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Scheduler.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Version.h"

#ifndef ARDUFAST_LOGF_BUFFER_SIZE
#define ARDUFAST_LOGF_BUFFER_SIZE 64
#endif
#ifndef ISKAKINO_LOGF_BUFFER_SIZE
#define ISKAKINO_LOGF_BUFFER_SIZE ARDUFAST_LOGF_BUFFER_SIZE
#endif

#define ARDUFAST_VERSION_MAJOR 1
#define ARDUFAST_VERSION_MINOR 1
#define ARDUFAST_VERSION_PATCH 0
#define ARDUFAST_VERSION ((ARDUFAST_VERSION_MAJOR * 10000UL) + \
                           (ARDUFAST_VERSION_MINOR * 100UL) + \
                           (ARDUFAST_VERSION_PATCH))

class IskakINO_ArduFast {
private:
    IskakINO_Scheduler _scheduler;
    IskakINO_Logger _logger;

    IskakINO_ArduFast(const IskakINO_ArduFast&);
    IskakINO_ArduFast& operator=(const IskakINO_ArduFast&);

public:
    explicit IskakINO_ArduFast(uint8_t maxTasks = 10);

    void begin(unsigned long baud = 115200);

    // --- Task Manager ---
    bool every(unsigned long interval, uint8_t id);
    bool once(unsigned long delay_ms, uint8_t id);
    void reset(uint8_t id);
    void cancel(uint8_t id);

    // --- Analog ---
    int readNorm(uint8_t pin);
    int readStable(uint8_t pin, uint8_t samples = 16);
    int mapAnalog(uint8_t pin, int outMin, int outMax);
    float readEMA(uint8_t pin, float &state, float alpha = 0.1f);

    // --- Logging ---
    // 1. Pesan Teks Biasa (RAM / Flash)
    void log(const char* msg);
    void log(const __FlashStringHelper* msg);

    // 2. Pesan Teks + Nilai Long
    void log(const char* msg, long val);
    void log(const __FlashStringHelper* msg, long val);

    // 3. Pesan Teks + Nilai Float
    void logFloat(const char* msg, float val, uint8_t decimals = 2);
    void logFloat(const __FlashStringHelper* msg, float val, uint8_t decimals = 2);

    // 4. Formatted Printf-Style (logf)
    void logf(const char* fmt, ...);
    void logf(const __FlashStringHelper* fmt, ...);

    void setDebug(bool debugMode) { _logger.setDebug(debugMode); }

    // --- Wrapper IO Standar ---
    void pinMode(uint8_t pin, uint8_t mode);
    void digitalWrite(uint8_t pin, uint8_t val);
    int digitalRead(uint8_t pin);
};

#endif
