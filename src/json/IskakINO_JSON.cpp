#include "IskakINO_JSON.h"

// ============================================================================
// ISKAK JSON BUILDER IMPLEMENTATION
// ============================================================================

IskakJSONBuilder::IskakJSONBuilder() {
    _buffer.reserve(128);
}

void IskakJSONBuilder::clear() {
    _buffer = "";
    _needComma = false;
}

void IskakJSONBuilder::appendCommaIfNeeded() {
    if (_needComma) {
        _buffer += ",";
    }
}

void IskakJSONBuilder::beginObject() {
    appendCommaIfNeeded();
    _buffer += "{";
    _needComma = false;
}

void IskakJSONBuilder::endObject() {
    _buffer += "}";
    _needComma = true;
}

void IskakJSONBuilder::beginArray(const char* key) {
    appendCommaIfNeeded();
    if (key && key[0] != '\0') {
        _buffer += "\"";
        _buffer += escape(key);
        _buffer += "\":[";
    } else {
        _buffer += "[";
    }
    _needComma = false;
}

void IskakJSONBuilder::endArray() {
    _buffer += "]";
    _needComma = true;
}

void IskakJSONBuilder::add(const char* key, const char* value) {
    if (!key) return;
    appendCommaIfNeeded();
    _buffer += "\"";
    _buffer += escape(key);
    _buffer += "\":\"";
    if (value) _buffer += escape(value);
    _buffer += "\"";
    _needComma = true;
}

void IskakJSONBuilder::add(const char* key, const String& value) {
    add(key, value.c_str());
}

void IskakJSONBuilder::add(const char* key, long value) {
    if (!key) return;
    appendCommaIfNeeded();
    _buffer += "\"";
    _buffer += escape(key);
    _buffer += "\":";
    _buffer += String(value);
    _needComma = true;
}

void IskakJSONBuilder::add(const char* key, float value, uint8_t decimals) {
    if (!key) return;
    appendCommaIfNeeded();
    _buffer += "\"";
    _buffer += escape(key);
    _buffer += "\":";
    _buffer += String(value, decimals);
    _needComma = true;
}

void IskakJSONBuilder::add(const char* key, bool value) {
    if (!key) return;
    appendCommaIfNeeded();
    _buffer += "\"";
    _buffer += escape(key);
    _buffer += "\":";
    _buffer += value ? "true" : "false";
    _needComma = true;
}

void IskakJSONBuilder::addRaw(const char* key, const char* rawJson) {
    if (!key || !rawJson) return;
    appendCommaIfNeeded();
    _buffer += "\"";
    _buffer += escape(key);
    _buffer += "\":";
    _buffer += rawJson;
    _needComma = true;
}

void IskakJSONBuilder::addValue(const char* value) {
    appendCommaIfNeeded();
    _buffer += "\"";
    if (value) _buffer += escape(value);
    _buffer += "\"";
    _needComma = true;
}

void IskakJSONBuilder::addValue(const String& value) {
    addValue(value.c_str());
}

void IskakJSONBuilder::addValue(long value) {
    appendCommaIfNeeded();
    _buffer += String(value);
    _needComma = true;
}

void IskakJSONBuilder::addValue(float value, uint8_t decimals) {
    appendCommaIfNeeded();
    _buffer += String(value, decimals);
    _needComma = true;
}

void IskakJSONBuilder::addValue(bool value) {
    appendCommaIfNeeded();
    _buffer += value ? "true" : "false";
    _needComma = true;
}

void IskakJSONBuilder::addRawValue(const char* rawJson) {
    if (!rawJson) return;
    appendCommaIfNeeded();
    _buffer += rawJson;
    _needComma = true;
}

String IskakJSONBuilder::escape(const String& raw) {
    String esc;
    esc.reserve(raw.length() + 8);
    for (size_t i = 0; i < raw.length(); i++) {
        char c = raw[i];
        switch (c) {
            case '\"': esc += "\\\""; break;
            case '\\': esc += "\\\\"; break;
            case '\b': esc += "\\b";  break;
            case '\f': esc += "\\f";  break;
            case '\n': esc += "\\n";  break;
            case '\r': esc += "\\r";  break;
            case '\t': esc += "\\t";  break;
            default:   esc += c;      break;
        }
    }
    return esc;
}

// ============================================================================
// ISKAK JSON READER IMPLEMENTATION (ZERO-COPY TOKENIZER)
// ============================================================================

IskakJSONReader::IskakJSONReader() {}

IskakJSONReader::IskakJSONReader(const String& json) {
    setSource(json);
}

IskakJSONReader::IskakJSONReader(const char* json) {
    setSource(json);
}

void IskakJSONReader::setSource(const String& json) {
    _source = trimWhitespace(json);
}

void IskakJSONReader::setSource(const char* json) {
    if (json) {
        _source = trimWhitespace(String(json));
    } else {
        _source = "";
    }
}

bool IskakJSONReader::isValid() const {
    if (_source.length() < 2) return false;
    char first = _source[0];
    char last = _source[_source.length() - 1];
    return (first == '{' && last == '}') || (first == '[' && last == ']');
}

String IskakJSONReader::trimWhitespace(const String& str) {
    int start = 0;
    int end = str.length() - 1;
    while (start <= end && ((uint8_t)str[start] <= 32)) start++;
    while (end >= start && ((uint8_t)str[end] <= 32)) end--;
    if (start > end) return "";
    return str.substring(start, end + 1);
}

int IskakJSONReader::findKey(const char* key, int startPos) const {
    if (!key || key[0] == '\0') return -1;
    String target = "\"" + String(key) + "\"";
    int pos = startPos;
    int len = _source.length();

    while (pos < len) {
        int found = _source.indexOf(target, pos);
        if (found < 0) return -1;

        // Pastikan bukan bagian dari string yang di-escape
        if (found > 0 && _source[found - 1] == '\\') {
            pos = found + target.length();
            continue;
        }

        // Cari tanda titik dua (colon ':') setelah key
        int colonPos = found + target.length();
        while (colonPos < len && ((uint8_t)_source[colonPos] <= 32)) colonPos++;

        if (colonPos < len && _source[colonPos] == ':') {
            // Nilai dimulai setelah colon
            int valStart = colonPos + 1;
            while (valStart < len && ((uint8_t)_source[valStart] <= 32)) valStart++;
            return valStart;
        }

        pos = found + target.length();
    }
    return -1;
}

String IskakJSONReader::extractValueAt(int valStart, char& outType) const {
    if (valStart < 0 || valStart >= (int)_source.length()) {
        outType = 'E'; // Error
        return "";
    }

    char first = _source[valStart];
    int len = _source.length();

    // 1. Tipe String
    if (first == '\"') {
        outType = 'S';
        int end = valStart + 1;
        while (end < len) {
            if (_source[end] == '\"' && _source[end - 1] != '\\') {
                break;
            }
            end++;
        }
        if (end < len) {
            return unescape(_source.substring(valStart + 1, end));
        }
        return "";
    }

    // 2. Tipe Objek Bersarang {...}
    if (first == '{') {
        outType = 'O';
        int depth = 0;
        bool inStr = false;
        for (int i = valStart; i < len; i++) {
            char c = _source[i];
            if (c == '\"' && (i == 0 || _source[i - 1] != '\\')) {
                inStr = !inStr;
            } else if (!inStr) {
                if (c == '{') depth++;
                else if (c == '}') {
                    depth--;
                    if (depth == 0) {
                        return _source.substring(valStart, i + 1);
                    }
                }
            }
        }
        return "";
    }

    // 3. Tipe Array Bersarang [...]
    if (first == '[') {
        outType = 'A';
        int depth = 0;
        bool inStr = false;
        for (int i = valStart; i < len; i++) {
            char c = _source[i];
            if (c == '\"' && (i == 0 || _source[i - 1] != '\\')) {
                inStr = !inStr;
            } else if (!inStr) {
                if (c == '[') depth++;
                else if (c == ']') {
                    depth--;
                    if (depth == 0) {
                        return _source.substring(valStart, i + 1);
                    }
                }
            }
        }
        return "";
    }

    // 4. Tipe Primitif (Angka, Boolean, Null)
    outType = 'P';
    int end = valStart;
    while (end < len) {
        char c = _source[end];
        if (c == ',' || c == '}' || c == ']' || ((uint8_t)c <= 32)) {
            break;
        }
        end++;
    }
    return _source.substring(valStart, end);
}

bool IskakJSONReader::hasKey(const char* key) const {
    return (findKey(key) >= 0);
}

String IskakJSONReader::getString(const char* key, const String& defaultVal) const {
    int idx = findKey(key);
    if (idx < 0) return defaultVal;
    char type;
    String val = extractValueAt(idx, type);
    if (type == 'E') return defaultVal;
    return val;
}

long IskakJSONReader::getInt(const char* key, long defaultVal) const {
    int idx = findKey(key);
    if (idx < 0) return defaultVal;
    char type;
    String val = extractValueAt(idx, type);
    if (type == 'E' || val.length() == 0) return defaultVal;
    return val.toInt();
}

float IskakJSONReader::getFloat(const char* key, float defaultVal) const {
    int idx = findKey(key);
    if (idx < 0) return defaultVal;
    char type;
    String val = extractValueAt(idx, type);
    if (type == 'E' || val.length() == 0) return defaultVal;
    return val.toFloat();
}

bool IskakJSONReader::getBool(const char* key, bool defaultVal) const {
    int idx = findKey(key);
    if (idx < 0) return defaultVal;
    char type;
    String val = extractValueAt(idx, type);
    if (val.equalsIgnoreCase("true") || val == "1") return true;
    if (val.equalsIgnoreCase("false") || val == "0") return false;
    return defaultVal;
}

String IskakJSONReader::getObject(const char* key) const {
    int idx = findKey(key);
    if (idx < 0) return "{}";
    char type;
    String val = extractValueAt(idx, type);
    if (type == 'O') return val;
    return "{}";
}

String IskakJSONReader::getArray(const char* key) const {
    int idx = findKey(key);
    if (idx < 0) return "[]";
    char type;
    String val = extractValueAt(idx, type);
    if (type == 'A') return val;
    return "[]";
}

size_t IskakJSONReader::getArrayLength(const char* arrayKey) const {
    String arrStr = arrayKey ? getArray(arrayKey) : _source;
    arrStr = trimWhitespace(arrStr);
    if (arrStr.length() < 2 || arrStr[0] != '[' || arrStr[arrStr.length() - 1] != ']') return 0;

    String inner = trimWhitespace(arrStr.substring(1, arrStr.length() - 1));
    if (inner.length() == 0) return 0;

    size_t count = 0;
    int depth = 0;
    bool inStr = false;

    for (size_t i = 0; i < inner.length(); i++) {
        char c = inner[i];
        if (c == '\"' && (i == 0 || inner[i - 1] != '\\')) {
            inStr = !inStr;
        } else if (!inStr) {
            if (c == '{' || c == '[') depth++;
            else if (c == '}' || c == ']') depth--;
            else if (c == ',' && depth == 0) count++;
        }
    }
    return count + 1;
}

String IskakJSONReader::getArrayItem(size_t index, const char* arrayKey) const {
    String arrStr = arrayKey ? getArray(arrayKey) : _source;
    arrStr = trimWhitespace(arrStr);
    if (arrStr.length() < 2 || arrStr[0] != '[') return "";

    String inner = trimWhitespace(arrStr.substring(1, arrStr.length() - 1));
    if (inner.length() == 0) return "";

    size_t curIdx = 0;
    int itemStart = 0;
    int depth = 0;
    bool inStr = false;

    for (size_t i = 0; i <= inner.length(); i++) {
        char c = (i < inner.length()) ? inner[i] : ',';
        if (i < inner.length() && c == '\"' && (i == 0 || inner[i - 1] != '\\')) {
            inStr = !inStr;
        } else if (!inStr) {
            if (i < inner.length() && (c == '{' || c == '[')) depth++;
            else if (i < inner.length() && (c == '}' || c == ']')) depth--;
            else if (c == ',' && depth == 0) {
                if (curIdx == index) {
                    String item = trimWhitespace(inner.substring(itemStart, i));
                    if (item.startsWith("\"") && item.endsWith("\"") && item.length() >= 2) {
                        return unescape(item.substring(1, item.length() - 1));
                    }
                    return item;
                }
                curIdx++;
                itemStart = i + 1;
            }
        }
    }
    return "";
}

#if defined(ESP32) || defined(ESP8266)
void IskakJSONReader::forEach(const char* arrayKey, std::function<void(size_t index, const String& item)> callback) const {
    if (!callback) return;
    size_t total = getArrayLength(arrayKey);
    for (size_t i = 0; i < total; i++) {
        String item = getArrayItem(i, arrayKey);
        callback(i, item);
    }
}
#endif

String IskakJSONReader::unescape(const String& raw) {
    String unesc;
    unesc.reserve(raw.length());
    size_t i = 0;
    while (i < raw.length()) {
        char c = raw[i];
        if (c == '\\' && i + 1 < raw.length()) {
            char next = raw[i + 1];
            switch (next) {
                case '\"': unesc += '\"'; i += 2; break;
                case '\\': unesc += '\\'; i += 2; break;
                case 'b':  unesc += '\b'; i += 2; break;
                case 'f':  unesc += '\f'; i += 2; break;
                case 'n':  unesc += '\n'; i += 2; break;
                case 'r':  unesc += '\r'; i += 2; break;
                case 't':  unesc += '\t'; i += 2; break;
                default:   unesc += next; i += 2; break;
            }
        } else {
            unesc += c;
            i++;
        }
    }
    return unesc;
}
