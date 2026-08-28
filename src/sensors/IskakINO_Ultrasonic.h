#ifndef ISKAKINO_ULTRASONIC_H
#define ISKAKINO_ULTRASONIC_H

#include <Arduino.h>
#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Result.h"
#include "../filter/IskakINO_Filter.h"

class IskakINO_Ultrasonic {
  public:
    IskakINO_Ultrasonic();
    virtual ~IskakINO_Ultrasonic() = default;

    void begin(uint8_t trigPin, uint8_t echoPin, uint16_t maxDistanceCm = 400);

    // Pengukuran jarak
    float getDistanceCm(bool filtered = true);
    float getDistanceMm(bool filtered = true);
    float getDistanceInch(bool filtered = true);

    // Filter Control
    void resetFilter();

    bool isSuccess() const { return _lastError == IskakINO_Result::OK; }
    IskakINO_Result lastError() const { return _lastError; }
    void setDebug(bool debug) { _logger.setDebug(debug); }

  private:
    uint8_t  _trigPin = 255;
    uint8_t  _echoPin = 255;
    uint16_t _maxDistanceCm = 400;
    uint32_t _timeoutUs = 25000;
    float    _lastDistanceCm = 0.0f;

    IskakINO_MedianFilter<float, 5> _medianFilter;
    IskakINO_Logger _logger;
    IskakINO_Result _lastError = IskakINO_Result::OK;

    float measureRawCm();
};

#endif // ISKAKINO_ULTRASONIC_H
