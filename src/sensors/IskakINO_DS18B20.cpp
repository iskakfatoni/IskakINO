#include "IskakINO_DS18B20.h"

IskakINO_DS18B20::IskakINO_DS18B20() {
    _logger.setDebug(true);
}

void IskakINO_DS18B20::begin(uint8_t pin) {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);
    _logger.logf(F("[IskakINO DS18B20] Inisialisasi 1-Wire pada Pin %d"), _pin);
}

uint8_t IskakINO_DS18B20::crc8(const uint8_t* data, size_t length) {
    uint8_t crc = 0;
    while (length--) {
        uint8_t inbyte = *data++;
        for (uint8_t i = 8; i; i--) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}

bool IskakINO_DS18B20::reset() {
    noInterrupts();
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delayMicroseconds(480);
    pinMode(_pin, INPUT_PULLUP);
    delayMicroseconds(70);
    uint8_t presence = digitalRead(_pin);
    delayMicroseconds(410);
    interrupts();

    return (presence == LOW);
}

void IskakINO_DS18B20::writeBit(uint8_t bit) {
    if (bit & 1) {
        noInterrupts();
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
        delayMicroseconds(6);
        pinMode(_pin, INPUT_PULLUP);
        delayMicroseconds(64);
        interrupts();
    } else {
        noInterrupts();
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, LOW);
        delayMicroseconds(60);
        pinMode(_pin, INPUT_PULLUP);
        delayMicroseconds(10);
        interrupts();
    }
}

uint8_t IskakINO_DS18B20::readBit() {
    noInterrupts();
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delayMicroseconds(3);
    pinMode(_pin, INPUT_PULLUP);
    delayMicroseconds(10);
    uint8_t r = digitalRead(_pin);
    delayMicroseconds(53);
    interrupts();
    return r;
}

void IskakINO_DS18B20::writeByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        writeBit(byte & 1);
        byte >>= 1;
    }
}

uint8_t IskakINO_DS18B20::readByte() {
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (readBit()) byte |= (1 << i);
    }
    return byte;
}

bool IskakINO_DS18B20::isConnected() {
    return reset();
}

bool IskakINO_DS18B20::read() {
    if (_pin == 255) {
        _lastError = IskakINO_Result::INVALID_ARG;
        return false;
    }

    // 1. Reset dan mulai konversi suhu (0x44)
    if (!reset()) {
        _lastError = IskakINO_Result::NOT_FOUND;
        return false;
    }

    writeByte(0xCC); // SKIP ROM
    writeByte(0x44); // START CONVERT T

    // Tunggu konversi selesai (maks 750ms untuk 12-bit)
    uint32_t startMs = millis();
    while (readBit() == 0) {
        if (millis() - startMs > 800) {
            _lastError = IskakINO_Result::TIMEOUT;
            return false;
        }
        delay(5);
    }

    // 2. Reset dan baca scratchpad 9 bytes (0xBE)
    if (!reset()) {
        _lastError = IskakINO_Result::NOT_FOUND;
        return false;
    }

    writeByte(0xCC); // SKIP ROM
    writeByte(0xBE); // READ SCRATCHPAD

    uint8_t scratchpad[9];
    for (uint8_t i = 0; i < 9; i++) {
        scratchpad[i] = readByte();
    }

    // 3. Verifikasi CRC-8
    if (crc8(scratchpad, 8) != scratchpad[8]) {
        _lastError = IskakINO_Result::WRITE_FAILED;
        _logger.log(F("[IskakINO DS18B20] CRC-8 Scratchpad Error!"));
        return false;
    }

    // 4. Hitung suhu Celcius (12-bit = resolusi 0.0625 C)
    int16_t rawTemp = (int16_t)((scratchpad[1] << 8) | scratchpad[0]);
    _lastTemperatureC = (float)rawTemp * 0.0625f;

    _lastError = IskakINO_Result::OK;
    return true;
}

float IskakINO_DS18B20::getTemperatureC() {
    read();
    return _lastTemperatureC;
}

float IskakINO_DS18B20::getTemperatureF() {
    float c = getTemperatureC();
    return (c * 1.8f) + 32.0f;
}
