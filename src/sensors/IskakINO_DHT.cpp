#include "IskakINO_DHT.h"

IskakINO_DHT::IskakINO_DHT() {
    _logger.setDebug(true);
}

void IskakINO_DHT::begin(uint8_t pin, IskakDHTType type) {
    _pin = pin;
    _type = type;
    pinMode(_pin, INPUT_PULLUP);
    _lastReadMs = millis() - getMinIntervalMs();
    _logger.logf(F("[IskakINO DHT] Inisialisasi pada Pin %d (%s)"),
                 _pin, (_type == IskakDHTType::DHT11) ? "DHT11" : "DHT22/AM2302");
}

uint32_t IskakINO_DHT::getMinIntervalMs() const {
    return (_type == IskakDHTType::DHT11) ? 1000UL : 2000UL;
}

bool IskakINO_DHT::read() {
    if (_pin == 255) {
        _lastError = IskakINO_Result::INVALID_ARG;
        return false;
    }

    uint32_t currentMs = millis();
    if (currentMs - _lastReadMs < getMinIntervalMs()) {
        // Return cache pembacaan sebelumnya jika belum melewati interval minimum
        return (_lastError == IskakINO_Result::OK);
    }
    _lastReadMs = currentMs;

    uint8_t data[5] = {0, 0, 0, 0, 0};

    // 1. Kirim Start Signal ke Sensor
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    if (_type == IskakDHTType::DHT11) {
        delay(20); // DHT11 perlu pull-low minimal 18ms
    } else {
        delay(2);  // DHT22/AM2302 cukup ~1.5 - 10ms
    }
    digitalWrite(_pin, HIGH);
    delayMicroseconds(30);
    pinMode(_pin, INPUT_PULLUP);

    // 2. Tunggu respon dari sensor (Sensor pull low ~80us, lalu pull high ~80us)
    uint32_t timeout = 10000;
    while (digitalRead(_pin) == HIGH) {
        if (--timeout == 0) {
            _lastError = IskakINO_Result::TIMEOUT;
            return false;
        }
    }

    timeout = 10000;
    while (digitalRead(_pin) == LOW) {
        if (--timeout == 0) {
            _lastError = IskakINO_Result::TIMEOUT;
            return false;
        }
    }

    timeout = 10000;
    while (digitalRead(_pin) == HIGH) {
        if (--timeout == 0) {
            _lastError = IskakINO_Result::TIMEOUT;
            return false;
        }
    }

    // 3. Baca 40 bits (5 bytes)
    noInterrupts();
    for (int i = 0; i < 40; i++) {
        // Tunggu transisi LOW -> HIGH
        uint32_t lowCycles = 0;
        while (digitalRead(_pin) == LOW) {
            if (++lowCycles > 2000) {
                interrupts();
                _lastError = IskakINO_Result::TIMEOUT;
                return false;
            }
        }

        // Hitung durasi HIGH
        uint32_t highCycles = 0;
        while (digitalRead(_pin) == HIGH) {
            if (++highCycles > 2000) {
                interrupts();
                _lastError = IskakINO_Result::TIMEOUT;
                return false;
            }
        }

        // Jika HIGH lebih lama daripada LOW -> bit 1, selain itu bit 0
        if (highCycles > lowCycles) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    interrupts();

    // 4. Verifikasi Checksum
    uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    if (data[4] != checksum) {
        _lastError = IskakINO_Result::WRITE_FAILED; // Corrupt data
        _logger.log(F("[IskakINO DHT] Checksum error saat membaca data!"));
        return false;
    }

    // 5. Konversi Nilai Suhu & Kelembapan
    if (_type == IskakDHTType::DHT11) {
        _lastHumidity = (float)data[0];
        if (data[1] != 0) _lastHumidity += (float)data[1] * 0.1f;

        _lastTemperature = (float)data[2];
        if (data[3] != 0) _lastTemperature += (float)data[3] * 0.1f;
    } else {
        // DHT22 / AM2302 / DHT21
        _lastHumidity = (float)((data[0] << 8) | data[1]) * 0.1f;

        int16_t rawTemp = (int16_t)(((data[2] & 0x7F) << 8) | data[3]);
        _lastTemperature = (float)rawTemp * 0.1f;
        if (data[2] & 0x80) {
            _lastTemperature = -_lastTemperature;
        }
    }

    _lastError = IskakINO_Result::OK;
    return true;
}

float IskakINO_DHT::getTemperature(bool inFahrenheit) {
    read();
    if (inFahrenheit) {
        return _lastTemperature * 1.8f + 32.0f;
    }
    return _lastTemperature;
}

float IskakINO_DHT::getHumidity() {
    read();
    return _lastHumidity;
}

float IskakINO_DHT::getHeatIndex(bool inFahrenheit) {
    float t = getTemperature(inFahrenheit);
    float rh = getHumidity();
    return computeHeatIndex(t, rh, inFahrenheit);
}

float IskakINO_DHT::computeHeatIndex(float temperature, float percentHumidity, bool isFahrenheit) {
    float hi;
    if (!isFahrenheit) temperature = temperature * 1.8f + 32.0f;

    hi = 0.5f * (temperature + 61.0f + ((temperature - 68.0f) * 1.2f) + (percentHumidity * 0.094f));

    if (hi >= 80.0f) {
        hi = -42.379f + 2.04901523f * temperature + 10.14333127f * percentHumidity -
             0.22475541f * temperature * percentHumidity -
             0.00683783f * temperature * temperature -
             0.05481717f * percentHumidity * percentHumidity +
             0.00122874f * temperature * temperature * percentHumidity +
             0.00085282f * temperature * percentHumidity * percentHumidity -
             0.00000199f * temperature * temperature * percentHumidity * percentHumidity;

        if ((percentHumidity < 13.0f) && (temperature >= 80.0f) && (temperature <= 112.0f)) {
            hi -= ((13.0f - percentHumidity) * 0.25f) * sqrt((17.0f - abs(temperature - 95.0f)) * 0.05882f);
        } else if ((percentHumidity > 85.0f) && (temperature >= 80.0f) && (temperature <= 87.0f)) {
            hi += ((percentHumidity - 85.0f) * 0.1f) * ((87.0f - temperature) * 0.2f);
        }
    }

    return isFahrenheit ? hi : (hi - 32.0f) * 0.55555f;
}
