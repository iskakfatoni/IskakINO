#ifndef MOCK_PREFERENCES_H
#define MOCK_PREFERENCES_H
#include <Arduino.h>
#include "ArduinoExtra.h"
#include <map>
#include <string>
#include <cstring>

class Preferences {
public:
    std::map<std::string, std::string> _blobs; // simulasi NVS in-memory (untuk putBytes/getBytes)

    bool begin(const char*, bool readOnly = false) { (void)readOnly; return true; }
    void end() {}
    void clear() { _blobs.clear(); }
    int getInt(const char*, int def) { return def; }
    void putInt(const char*, int) {}
    String getString(const char*, const String& def) { return def; }
    void putString(const char*, const String&) {}

    size_t putBytes(const char* key, const void* buf, size_t len) {
        _blobs[key] = std::string((const char*)buf, len);
        return len;
    }
    size_t getBytes(const char* key, void* buf, size_t maxLen) {
        auto it = _blobs.find(key);
        if (it == _blobs.end()) return 0;
        size_t n = it->second.size() < maxLen ? it->second.size() : maxLen;
        memcpy(buf, it->second.data(), n);
        return n;
    }
    bool isKey(const char* key) { return _blobs.count(key) > 0; }
    bool remove(const char* key) { return _blobs.erase(key) > 0; }
};
#endif
