#ifndef MOCK_UPDATE_H
#define MOCK_UPDATE_H
#include <Arduino.h>
#include "ArduinoExtra.h"
class UpdateClass {
public:
    bool begin(size_t) { return true; }
    size_t write(uint8_t*, size_t len) { return len; }
    bool end(bool) { return true; }
    bool hasError() { return false; }
    template <typename T> void printError(T&) {}
};
extern UpdateClass Update;
#endif
