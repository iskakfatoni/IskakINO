#ifndef MOCK_LITTLEFS_H
#define MOCK_LITTLEFS_H
#include <Arduino.h>
#include "ArduinoExtra.h"
#include <map>
#include <string>
#include <sstream>
#include <vector>

class File {
public:
    bool _ok = false;
    bool _writing = false;
    std::string _key;
    std::ostringstream _writeBuf;
    std::istringstream _readBuf;
    std::string _readAll; // untuk read() biner (Storage pakai ini, bukan readStringUntil)

    operator bool() const { return _ok; }
    bool available() { return _ok && !_writing && _readBuf.peek() != EOF; }
    String readStringUntil(char delim) {
        std::string line;
        std::getline(_readBuf, line, delim);
        return String(line.c_str());
    }
    void print(const String& s) { if (_writing) _writeBuf << s.c_str(); }
    void print(char c) { if (_writing) _writeBuf << c; }
    void println(const String& s) { if (_writing) _writeBuf << s.c_str() << "\n"; }

    // Dipakai IskakINO_Storage (_writeSlot/_readSlot) — I/O biner mentah,
    // beda dari print()/println()/readStringUntil() yang dipakai WifiPortal.
    size_t write(const uint8_t* buf, size_t len) {
        if (!_writing) return 0;
        _writeBuf.write((const char*)buf, (std::streamsize)len);
        return len;
    }
    size_t read(uint8_t* buf, size_t len) {
        if (_writing) return 0;
        _readBuf.read((char*)buf, (std::streamsize)len);
        size_t got = (size_t)_readBuf.gcount();
        return got;
    }
    void close();
};

class Dir {
public:
    std::vector<std::string> _names;
    size_t _idx = (size_t)-1;
    bool next() { _idx++; return _idx < _names.size(); }
    String fileName() { return String(_names[_idx].c_str()); }
};

class LittleFSClass {
public:
    std::map<std::string, std::string> _files; // simulasi filesystem in-memory
    bool _failOpen = false; // set true dari test untuk mensimulasikan open() gagal

    bool begin() { return true; }
    bool exists(const char* path) { return _files.count(path) > 0; }
    File open(const char* path, const char* mode) {
        File f;
        if (_failOpen) { f._ok = false; return f; }
        std::string m(mode);
        f._key = path;
        if (m == "w") {
            f._ok = true;
            f._writing = true;
        } else { // "r"
            auto it = _files.find(path);
            if (it == _files.end()) { f._ok = false; return f; }
            f._ok = true;
            f._writing = false;
            f._readBuf.str(it->second);
        }
        return f;
    }
    bool remove(const char* path) { return _files.erase(path) > 0; }
    bool remove(const String& path) { return _files.erase(path.c_str()) > 0; }
    Dir openDir(const char*) {
        Dir d;
        for (auto& kv : _files) d._names.push_back(kv.first);
        return d;
    }
};
extern LittleFSClass LittleFS;

inline void File::close() {
    if (_writing && _ok) {
        LittleFS._files[_key] = _writeBuf.str();
    }
}
#endif
