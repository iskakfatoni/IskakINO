#ifndef ISKAKINO_DS18B20_H
#define ISKAKINO_DS18B20_H

#include <Arduino.h>
#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Result.h"

class IskakINO_DS18B20 {
  public:
    IskakINO_DS18B20();
    virtual ~IskakINO_DS18B20() = default;

    void begin(uint8_t pin);

    // Membaca suhu dari sensor
    bool read();
    float getTemperatureC();
    float getTemperatureF();

    bool isConnected();
    bool isSuccess() const { return _lastError == IskakINO_Result::OK; }
    IskakINO_Result lastError() const { return _lastError; }
    void setDebug(bool debug) { _logger.setDebug(debug); }

  private:
    uint8_t _pin = 255;
    float   _lastTemperatureC = -999.0f;
    uint32_t _lastReadMs = 0;
    IskakINO_Logger _logger;
    IskakINO_Result _lastError = IskakINO_Result::OK;

    // 1-Wire Bit-Banging Native Protocol
    bool reset();
    void writeBit(uint8_t bit);
    uint8_t readBit();
    void writeByte(uint8_t byte);
    uint8_t readByte();
    static uint8_t crc8(const uint8_t* data, size_t length);
};

#endif // ISKAKINO_DS18B20_H
