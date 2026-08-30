/*
 * src/prayertimes/IskakINO_PrayerTimesModule.h
 *
 * Adapter IskakINO_PrayerTimes untuk IskakINO_Kernel.
 * Universal untuk semua platform (AVR, ESP32, ESP8266).
 */

#ifndef ISKAKINO_PRAYERTIMES_MODULE_H
#define ISKAKINO_PRAYERTIMES_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_PrayerTimes.h"

class IskakINO_PrayerTimesModule : public IskakINO_Module {
private:
    IskakINO_PrayerTimes& _pt;
    float _lat;
    float _lng;
    float _tz;

public:
    explicit IskakINO_PrayerTimesModule(IskakINO_PrayerTimes& pt, float latitude = -6.2088f, float longitude = 106.8456f, float timezone = 7.0f)
        : _pt(pt), _lat(latitude), _lng(longitude), _tz(timezone) {}

    void begin() override {
        _pt.setLocation(_lat, _lng, _tz);
    }

    void update() override {
        // No periodic polling needed by default (computed on demand or date change)
    }

    const char* moduleName() const override {
        return "PrayerTimes";
    }
};

#endif // ISKAKINO_PRAYERTIMES_MODULE_H
