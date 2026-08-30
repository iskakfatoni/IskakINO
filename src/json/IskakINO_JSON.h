#ifndef ISKAKINO_JSON_H
#define ISKAKINO_JSON_H

#include <Arduino.h>
#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Result.h"

#if defined(ESP32) || defined(ESP8266)
#include <functional>
#endif

class IskakJSONBuilder {
  public:
    IskakJSONBuilder();
    virtual ~IskakJSONBuilder() = default;

    void clear();
    void beginObject();
    void endObject();
    void beginArray(const char* key = nullptr);
    void endArray();

    // Penambahan Key-Value untuk Object
    void add(const char* key, const char* value);
    void add(const char* key, const String& value);
    void add(const char* key, long value);
    void add(const char* key, int value) { add(key, (long)value); }
    void add(const char* key, uint32_t value) { add(key, (long)value); }
    void add(const char* key, float value, unsigned int decimals = 2);
    void add(const char* key, double value, unsigned int decimals = 2) { add(key, (float)value, decimals); }
    void add(const char* key, bool value);
    void addRaw(const char* key, const char* rawJson);
    void addRaw(const char* key, const String& rawJson) { addRaw(key, rawJson.c_str()); }

    // Penambahan Elemen untuk Array
    void addValue(const char* value);
    void addValue(const String& value);
    void addValue(long value);
    void addValue(int value) { addValue((long)value); }
    void addValue(float value, unsigned int decimals = 2);
    void addValue(bool value);
    void addRawValue(const char* rawJson);

    String toString() const { return _buffer; }
    size_t length() const { return _buffer.length(); }
    const char* c_str() const { return _buffer.c_str(); }
    void writeTo(Print& out) const { out.print(_buffer); }

    static String escape(const String& raw);

  private:
    String _buffer;
    bool   _needComma = false;
    void   appendCommaIfNeeded();
};

class IskakJSONReader {
  public:
    IskakJSONReader();
    IskakJSONReader(const String& json);
    IskakJSONReader(const char* json);
    virtual ~IskakJSONReader() = default;

    void setSource(const String& json);
    void setSource(const char* json);

    bool isValid() const;
    bool hasKey(const char* key) const;

    String getString(const char* key, const String& defaultVal = "") const;
    long   getInt(const char* key, long defaultVal = 0) const;
    float  getFloat(const char* key, float defaultVal = 0.0f) const;
    bool   getBool(const char* key, bool defaultVal = false) const;
    String getObject(const char* key) const;
    String getArray(const char* key) const;

    // Helper untuk Elemen Array
    size_t getArrayLength(const char* arrayKey = nullptr) const;
    String getArrayItem(size_t index, const char* arrayKey = nullptr) const;

    #if defined(ESP32) || defined(ESP8266)
    void forEach(const char* arrayKey, std::function<void(size_t index, const String& item)> callback) const;
    #endif

    static String unescape(const String& raw);

  private:
    String _source;
    int findKey(const char* key, int startPos = 0) const;
    String extractValueAt(int valStart, char& outType) const;
    static String trimWhitespace(const String& str);
};

// Aliases untuk konsistensi penamaan ekosistem IskakINO
typedef IskakJSONBuilder IskakINO_JSONBuilder;
typedef IskakJSONReader  IskakINO_JSONReader;
typedef IskakJSONBuilder IskakINO_JSON;

#endif // ISKAKINO_JSON_H
