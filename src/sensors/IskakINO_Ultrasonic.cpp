#include "IskakINO_Ultrasonic.h"

IskakINO_Ultrasonic::IskakINO_Ultrasonic() {
    _logger.setDebug(true);
}

void IskakINO_Ultrasonic::begin(uint8_t trigPin, uint8_t echoPin, uint16_t maxDistanceCm) {
    _trigPin = trigPin;
    _echoPin = echoPin;
    _maxDistanceCm = maxDistanceCm;

    // Timeout mikrodetik = (2 * maxDistanceCm / 0.0343)
    _timeoutUs = (uint32_t)((_maxDistanceCm * 58.2f) + 1000.0f);

    pinMode(_trigPin, OUTPUT);
    digitalWrite(_trigPin, LOW);
    pinMode(_echoPin, INPUT);

    _medianFilter.reset();
    _logger.logf(F("[IskakINO Ultrasonic] Inisialisasi Trig Pin: %d | Echo Pin: %d (Maks: %d cm)"),
                 _trigPin, _echoPin, _maxDistanceCm);
}

void IskakINO_Ultrasonic::resetFilter() {
    _medianFilter.reset();
}

float IskakINO_Ultrasonic::measureRawCm() {
    if (_trigPin == 255 || _echoPin == 255) {
        _lastError = IskakINO_Result::INVALID_ARG;
        return 0.0f;
    }

    // Trigger Pulse 10us
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(4);
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    // Ukur durasi pulse HIGH pada Echo Pin
    unsigned long duration = pulseIn(_echoPin, HIGH, _timeoutUs);

    if (duration == 0) {
        _lastError = IskakINO_Result::TIMEOUT;
        return (float)_maxDistanceCm; // Out of range
    }

    // Jarak (cm) = durasi / 58.2
    float distance = (float)duration / 58.2f;
    if (distance > _maxDistanceCm) distance = (float)_maxDistanceCm;

    _lastError = IskakINO_Result::OK;
    return distance;
}

float IskakINO_Ultrasonic::getDistanceCm(bool filtered) {
    float raw = measureRawCm();
    if (_lastError != IskakINO_Result::OK && raw >= (float)_maxDistanceCm) {
        return (float)_maxDistanceCm;
    }

    if (filtered) {
        _lastDistanceCm = _medianFilter.update(raw);
    } else {
        _lastDistanceCm = raw;
    }
    return _lastDistanceCm;
}

float IskakINO_Ultrasonic::getDistanceMm(bool filtered) {
    return getDistanceCm(filtered) * 10.0f;
}

float IskakINO_Ultrasonic::getDistanceInch(bool filtered) {
    return getDistanceCm(filtered) * 0.393701f;
}
