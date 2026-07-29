#ifndef MOCK_SOFTWARESERIAL_H
#define MOCK_SOFTWARESERIAL_H
#include "Stream.h"
// SoftwareSerial di board nyata juga turunan Stream (punya begin/available/
// read/write yang sama) -- untuk mock, cukup alias ke Stream yang sudah ada,
// tambah constructor (rxPin, txPin) yang diabaikan (tidak relevan utk test).
class SoftwareSerial : public Stream {
public:
    SoftwareSerial(uint8_t rxPin, uint8_t txPin) { (void)rxPin; (void)txPin; }
};
#endif
