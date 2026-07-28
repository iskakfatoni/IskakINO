#ifndef MOCK_EEPROM_H
#define MOCK_EEPROM_H
#include <Arduino.h>
#include <vector>
class EEPROMClass {
public:
    std::vector<uint8_t> _data;
    explicit EEPROMClass(size_t size = 1024) : _data(size, 0xFF) {}
    int length() const { return (int)_data.size(); }
    uint8_t read(int addr) { return (addr >= 0 && (size_t)addr < _data.size()) ? _data[addr] : 0xFF; }
    void write(int addr, uint8_t val) { if (addr >= 0 && (size_t)addr < _data.size()) _data[addr] = val; }
};
extern EEPROMClass EEPROM;
#endif
