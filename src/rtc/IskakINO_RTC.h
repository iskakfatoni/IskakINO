#ifndef ISKAKINO_RTC_H
#define ISKAKINO_RTC_H

#include <Arduino.h>
#include <Wire.h>

#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Result.h"
#include "core/IskakINO_Scheduler.h"

// Forward declaration untuk sinergi FastNTP
#if defined(ISKAKINO_HAS_WIFI)
class IskakINO_FastNTP;
#endif

enum class IskakRTCType : uint8_t {
    AUTO,
    DS3231,
    DS1307,
    PCF8563
};

struct IskakDateTime {
    uint16_t year;      // contoh: 2026
    uint8_t  month;     // 1 - 12
    uint8_t  day;       // 1 - 31
    uint8_t  hour;      // 0 - 23
    uint8_t  minute;    // 0 - 59
    uint8_t  second;    // 0 - 59
    uint8_t  dayOfWeek; // 1 = Minggu, 2 = Senin, ..., 7 = Sabtu

    IskakDateTime() : year(2026), month(1), day(1), hour(0), minute(0), second(0), dayOfWeek(5) {}
    IskakDateTime(uint16_t y, uint8_t m, uint8_t d, uint8_t h, uint8_t min, uint8_t s, uint8_t dow = 1)
        : year(y), month(m), day(d), hour(h), minute(min), second(s), dayOfWeek(dow) {}

    uint32_t toEpoch() const;
    static IskakDateTime fromEpoch(uint32_t epoch);

    // Formatter string
    String getTimeString(bool withSeconds = true) const;
    String getDateString(bool inIndonesian = true) const;
    String format(const char* fmt = "%Y-%m-%d %H:%M:%S") const;
    const char* getDayName(bool inIndonesian = true) const;
    const char* getMonthName(bool inIndonesian = true) const;
};

class IskakINO_RTC {
  public:
    IskakINO_RTC();
    virtual ~IskakINO_RTC() = default;

    // Inisialisasi bus I2C & deteksi chip
    bool begin(TwoWire& wire = Wire, IskakRTCType type = IskakRTCType::AUTO);

    // Status chip
    bool isRunning();
    bool lostPower();
    IskakRTCType chipType() const { return _chipType; }
    const char* chipName() const;

    // Pembacaan & Pengaturan Waktu
    IskakDateTime now();
    void setDateTime(const IskakDateTime& dt);
    void setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    void setEpoch(uint32_t epoch);
    uint32_t getEpoch();

    // Sensor Suhu internal (khusus DS3231)
    float getTemperature();

    // Sinergi Hybrid NTP (khusus board WiFi ESP32/ESP8266)
    #if defined(ISKAKINO_HAS_WIFI)
    void syncWithNTP(IskakINO_FastNTP& ntp, uint32_t intervalMs = 3600000UL);
    #endif

    // Loop & Scheduler
    void tick();

    void setDebug(bool debug) { _logger.setDebug(debug); }
    IskakINO_Result lastError() const { return _lastError; }

  private:
    TwoWire* _wire = &Wire;
    IskakINO_Logger _logger;
    IskakINO_Result _lastError = IskakINO_Result::OK;
    IskakRTCType _chipType = IskakRTCType::AUTO;
    uint8_t _i2cAddress = 0x68;

    #if defined(ISKAKINO_HAS_WIFI)
    IskakINO_FastNTP* _ntpSyncSource = nullptr;
    IskakINO_Scheduler _scheduler{1}; // id 0 = NTP sync check
    uint32_t _ntpSyncIntervalMs = 3600000UL;
    #endif

    // Register I2C helpers
    bool readRegisters(uint8_t reg, uint8_t* buffer, size_t length);
    bool writeRegisters(uint8_t reg, const uint8_t* buffer, size_t length);
    bool writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);

    bool detectChip();
    static uint8_t calculateDayOfWeek(uint16_t y, uint8_t m, uint8_t d);
};

#endif // ISKAKINO_RTC_H
