#ifndef ISKAKINO_DHT_H
#define ISKAKINO_DHT_H

#include <Arduino.h>
#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Result.h"

enum class IskakDHTType : uint8_t {
    DHT11,
    DHT22,
    AM2302 = DHT22,
    DHT21,
    AM2301 = DHT21
};

class IskakINO_DHT {
  public:
    IskakINO_DHT();
    virtual ~IskakINO_DHT() = default;

    void begin(uint8_t pin, IskakDHTType type = IskakDHTType::DHT11);

    // Membaca data sensor (non-blocking jika dipanggil sebelum interval minimum)
    bool read();

    float getTemperature(bool inFahrenheit = false);
    float getHumidity();
    float getHeatIndex(bool inFahrenheit = false);

    bool isSuccess() const { return _lastError == IskakINO_Result::OK; }
    IskakINO_Result lastError() const { return _lastError; }
    void setDebug(bool debug) { _logger.setDebug(debug); }

  private:
    uint8_t _pin = 255;
    IskakDHTType _type = IskakDHTType::DHT11;
    float _lastTemperature = 0.0f;
    float _lastHumidity = 0.0f;
    uint32_t _lastReadMs = 0;
    IskakINO_Logger _logger;
    IskakINO_Result _lastError = IskakINO_Result::OK;

    uint32_t getMinIntervalMs() const;
    static float computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit);
};

#endif // ISKAKINO_DHT_H
