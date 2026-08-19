/*
 * src/oled/IskakINO_OLED.cpp
 * Implementasi driver Layar Grafis OLED I2C (SSD1306 & SH1106) untuk IskakINO.
 */

#include "IskakINO_OLED.h"
#include <stdarg.h>
#include <string.h>

#define OLED_CMD_MODE  0x00
#define OLED_DATA_MODE 0x40

IskakINO_OLED::IskakINO_OLED(uint8_t width, uint8_t height, uint8_t i2cAddress)
    : _width(width), _height(height), _pages(height / 8), _address(i2cAddress),
      _wire(&Wire), _connected(false), _chip(OLED_CHIP_AUTO),
      _cursorCol(0), _cursorRow(0), _textSize(1),
      _typewriterActive(false), _twTargetCol(0), _twTargetRow(0), _twIndex(0), _twIntervalMs(60), _twTimerMark(0),
      _scrollActive(false), _scrollRow(0), _scrollOffset(0), _scrollIntervalMs(150), _scrollTimerMark(0) {
    _animText[0] = '\0';
}

bool IskakINO_OLED::begin(TwoWire &wirePort) {
    _wire = &wirePort;
    _wire->begin();

    // Auto-scan jika alamat bernilai 0 atau cek alamat yang diberikan
    if (_address == 0) {
        // Coba 0x3C dulu lalu 0x3D
        _wire->beginTransmission(0x3C);
        if (_wire->endTransmission() == 0) {
            _address = 0x3C;
            _connected = true;
        } else {
            _wire->beginTransmission(0x3D);
            if (_wire->endTransmission() == 0) {
                _address = 0x3D;
                _connected = true;
            } else {
                _connected = false;
                return false;
            }
        }
    } else {
        _wire->beginTransmission(_address);
        _connected = (_wire->endTransmission() == 0);
        if (!_connected) {
            // Coba alternatif 0x3C / 0x3D jika alamat yang dimasukkan gagal
            uint8_t alt = (_address == 0x3C) ? 0x3D : 0x3C;
            _wire->beginTransmission(alt);
            if (_wire->endTransmission() == 0) {
                _address = alt;
                _connected = true;
            } else {
                return false;
            }
        }
    }

    _pages = _height / 8;
    if (_pages == 0) _pages = 4; // Fallback untuk 32px

    _initDisplay();
    clear();
    return true;
}

bool IskakINO_OLED::begin(uint8_t width, uint8_t height, uint8_t i2cAddress, TwoWire &wirePort) {
    _width = width;
    _height = height;
    _address = i2cAddress;
    return begin(wirePort);
}

bool IskakINO_OLED::isConnected() const {
    return _connected;
}

uint8_t IskakINO_OLED::getAddress() const {
    return _address;
}

void IskakINO_OLED::setChipType(IskakOLEDChip chip) {
    _chip = chip;
}

void IskakINO_OLED::_sendCommand(uint8_t cmd) {
    if (!_connected) return;
    _wire->beginTransmission(_address);
    _wire->write(OLED_CMD_MODE);
    _wire->write(cmd);
    _wire->endTransmission();
}

void IskakINO_OLED::_sendCommand2(uint8_t cmd1, uint8_t cmd2) {
    if (!_connected) return;
    _wire->beginTransmission(_address);
    _wire->write(OLED_CMD_MODE);
    _wire->write(cmd1);
    _wire->write(cmd2);
    _wire->endTransmission();
}

void IskakINO_OLED::_sendCommand3(uint8_t cmd1, uint8_t cmd2, uint8_t cmd3) {
    if (!_connected) return;
    _wire->beginTransmission(_address);
    _wire->write(OLED_CMD_MODE);
    _wire->write(cmd1);
    _wire->write(cmd2);
    _wire->write(cmd3);
    _wire->endTransmission();
}

void IskakINO_OLED::_sendData(uint8_t data) {
    if (!_connected) return;
    _wire->beginTransmission(_address);
    _wire->write(OLED_DATA_MODE);
    _wire->write(data);
    _wire->endTransmission();
}

void IskakINO_OLED::_sendDataChunk(const uint8_t* data, size_t len, bool isProgmem) {
    if (!_connected || !data || len == 0) return;

    size_t i = 0;
    while (i < len) {
        size_t chunk = (len - i > 16) ? 16 : (len - i);
        _wire->beginTransmission(_address);
        _wire->write(OLED_DATA_MODE);
        for (size_t k = 0; k < chunk; k++) {
            uint8_t b = isProgmem ? pgm_read_byte(data + i + k) : *(data + i + k);
            _wire->write(b);
        }
        _wire->endTransmission();
        i += chunk;
    }
}

void IskakINO_OLED::_setPageAndCol(uint8_t col, uint8_t page) {
    if (page >= _pages) page = _pages - 1;
    if (col >= _width) col = _width - 1;

    uint8_t colOffset = (_chip == OLED_CHIP_SH1106) ? (col + 2) : col;

    _sendCommand(0xB0 | (page & 0x07)); // Set Page Address (B0 ~ B7)
    _sendCommand(0x00 | (colOffset & 0x0F)); // Lower Column Address
    _sendCommand(0x10 | ((colOffset >> 4) & 0x0F)); // Higher Column Address
}

void IskakINO_OLED::_initDisplay() {
    _sendCommand(0xAE); // Display OFF

    _sendCommand2(0xD5, 0x80); // Set Display Clock Divide Ratio
    _sendCommand2(0xA8, _height - 1); // Set Multiplex Ratio (63 untuk 64px, 31 untuk 32px)
    _sendCommand2(0xD3, 0x00); // Set Display Offset
    _sendCommand(0x40 | 0x00); // Set Display Start Line (0)

    if (_chip == OLED_CHIP_SH1106) {
        _sendCommand2(0xAD, 0x8B); // Enable DC-DC charge pump SH1106
    } else {
        _sendCommand2(0x8D, 0x14); // Enable Charge Pump SSD1306 (7.5V)
    }

    _sendCommand2(0x20, 0x02); // Memory Addressing Mode: Page Addressing Mode
    _sendCommand(0xA1); // Set Segment Re-map (A1 = normal orientation)
    _sendCommand(0xC8); // Set COM Output Scan Direction (C8 = normal orientation)

    if (_height == 32) {
        _sendCommand2(0xDA, 0x02); // COM Pins config 32px
    } else {
        _sendCommand2(0xDA, 0x12); // COM Pins config 64px
    }

    _sendCommand2(0x81, 0xCF); // Contrast Control
    _sendCommand2(0xD9, 0xF1); // Pre-charge Period
    _sendCommand2(0xDB, 0x40); // VCOMH Deselect Level
    _sendCommand(0xA4); // Entire Display Resume from RAM
    _sendCommand(0xA6); // Normal Display (Non-inverted)
    _sendCommand(0xAF); // Display ON
}

void IskakINO_OLED::clear() {
    for (uint8_t p = 0; p < _pages; p++) {
        clearRow(p);
    }
    setCursor(0, 0);
}

void IskakINO_OLED::clearRow(uint8_t row) {
    if (row >= _pages) return;
    _setPageAndCol(0, row);

    // Kirim 128 bytes 0x00 dalam chunk 16 byte
    for (uint8_t i = 0; i < 8; i++) {
        _wire->beginTransmission(_address);
        _wire->write(OLED_DATA_MODE);
        for (uint8_t k = 0; k < 16; k++) {
            _wire->write(0x00);
        }
        _wire->endTransmission();
    }
}

void IskakINO_OLED::clearArea(uint8_t startCol, uint8_t row, uint8_t widthCols) {
    if (row >= _pages || startCol >= _width) return;
    if (startCol + widthCols > _width) widthCols = _width - startCol;

    _setPageAndCol(startCol, row);
    uint8_t sent = 0;
    while (sent < widthCols) {
        uint8_t chunk = (widthCols - sent > 16) ? 16 : (widthCols - sent);
        _wire->beginTransmission(_address);
        _wire->write(OLED_DATA_MODE);
        for (uint8_t k = 0; k < chunk; k++) {
            _wire->write(0x00);
        }
        _wire->endTransmission();
        sent += chunk;
    }
}

void IskakINO_OLED::setCursor(uint8_t col, uint8_t row) {
    _cursorCol = (col < _width) ? col : _width - 1;
    _cursorRow = (row < _pages) ? row : _pages - 1;
}

void IskakINO_OLED::setTextSize(uint8_t size) {
    _textSize = (size >= 2) ? 2 : 1;
}

void IskakINO_OLED::invertDisplay(bool invert) {
    _sendCommand(invert ? 0xA7 : 0xA6);
}

void IskakINO_OLED::setContrast(uint8_t contrast) {
    _sendCommand2(0x81, contrast);
}

void IskakINO_OLED::displayOn() {
    _sendCommand(0xAF);
}

void IskakINO_OLED::displayOff() {
    _sendCommand(0xAE);
}

void IskakINO_OLED::_drawChar5x7(uint8_t c) {
    if (c < 32 || c > 126) c = ' ';
    uint8_t fontIdx = c - 32;

    _setPageAndCol(_cursorCol, _cursorRow);

    _wire->beginTransmission(_address);
    _wire->write(OLED_DATA_MODE);
    for (uint8_t col = 0; col < 5; col++) {
        uint8_t b = pgm_read_byte(&(OLED_FONT_5X7[fontIdx][col]));
        _wire->write(b);
    }
    _wire->write(0x00); // 1 pixel horizontal space gap
    _wire->endTransmission();

    _cursorCol += 6;
    if (_cursorCol + 6 > _width) {
        _cursorCol = 0;
        _cursorRow++;
        if (_cursorRow >= _pages) _cursorRow = 0;
    }
}

void IskakINO_OLED::_drawChar2x(uint8_t c) {
    if (c < 32 || c > 126) c = ' ';
    uint8_t fontIdx = c - 32;

    // Expand font 5x7 menjadi 10x14 pixel (memakai 2 pages vertikal)
    // Page 1 (Bagian Atas - 4 bit bawah di-stretch jadi 8 bit)
    if (_cursorRow < _pages) {
        _setPageAndCol(_cursorCol, _cursorRow);
        _wire->beginTransmission(_address);
        _wire->write(OLED_DATA_MODE);
        for (uint8_t col = 0; col < 5; col++) {
            uint8_t orig = pgm_read_byte(&(OLED_FONT_5X7[fontIdx][col]));
            uint8_t top = 0;
            if (orig & 0x01) top |= 0x03;
            if (orig & 0x02) top |= 0x0C;
            if (orig & 0x04) top |= 0x30;
            if (orig & 0x08) top |= 0xC0;
            _wire->write(top);
            _wire->write(top); // 2x horizontal width
        }
        _wire->write(0x00);
        _wire->write(0x00);
        _wire->endTransmission();
    }

    // Page 2 (Bagian Bawah - 4 bit atas di-stretch jadi 8 bit)
    if (_cursorRow + 1 < _pages) {
        _setPageAndCol(_cursorCol, _cursorRow + 1);
        _wire->beginTransmission(_address);
        _wire->write(OLED_DATA_MODE);
        for (uint8_t col = 0; col < 5; col++) {
            uint8_t orig = pgm_read_byte(&(OLED_FONT_5X7[fontIdx][col]));
            uint8_t bot = 0;
            if (orig & 0x10) bot |= 0x03;
            if (orig & 0x20) bot |= 0x0C;
            if (orig & 0x40) bot |= 0x30;
            if (orig & 0x80) bot |= 0xC0;
            _wire->write(bot);
            _wire->write(bot);
        }
        _wire->write(0x00);
        _wire->write(0x00);
        _wire->endTransmission();
    }

    _cursorCol += 12;
    if (_cursorCol + 12 > _width) {
        _cursorCol = 0;
        _cursorRow += 2;
        if (_cursorRow >= _pages) _cursorRow = 0;
    }
}

size_t IskakINO_OLED::write(uint8_t c) {
    if (c == '\n') {
        _cursorCol = 0;
        _cursorRow += (_textSize == 2) ? 2 : 1;
        if (_cursorRow >= _pages) _cursorRow = 0;
        return 1;
    }
    if (c == '\r') {
        _cursorCol = 0;
        return 1;
    }

    if (_textSize == 2) {
        _drawChar2x(c);
    } else {
        _drawChar5x7(c);
    }
    return 1;
}

void IskakINO_OLED::printCenter(const char* text, uint8_t row) {
    if (!text || row >= _pages) return;
    size_t len = strlen(text);
    uint8_t charWidth = (_textSize == 2) ? 12 : 6;
    uint16_t textPx = len * charWidth;
    uint8_t startCol = (textPx < _width) ? (_width - textPx) / 2 : 0;

    clearRow(row);
    if (_textSize == 2 && row + 1 < _pages) clearRow(row + 1);

    setCursor(startCol, row);
    print(text);
}

void IskakINO_OLED::printCenter(const __FlashStringHelper* text, uint8_t row) {
    if (!text || row >= _pages) return;
    size_t len = strlen_P((const char*)text);
    uint8_t charWidth = (_textSize == 2) ? 12 : 6;
    uint16_t textPx = len * charWidth;
    uint8_t startCol = (textPx < _width) ? (_width - textPx) / 2 : 0;

    clearRow(row);
    if (_textSize == 2 && row + 1 < _pages) clearRow(row + 1);

    setCursor(startCol, row);
    print(text);
}

void IskakINO_OLED::printRight(const char* text, uint8_t row) {
    if (!text || row >= _pages) return;
    size_t len = strlen(text);
    uint8_t charWidth = (_textSize == 2) ? 12 : 6;
    uint16_t textPx = len * charWidth;
    uint8_t startCol = (textPx < _width) ? (_width - textPx) : 0;

    setCursor(startCol, row);
    print(text);
}

void IskakINO_OLED::printRight(const __FlashStringHelper* text, uint8_t row) {
    if (!text || row >= _pages) return;
    size_t len = strlen_P((const char*)text);
    uint8_t charWidth = (_textSize == 2) ? 12 : 6;
    uint16_t textPx = len * charWidth;
    uint8_t startCol = (textPx < _width) ? (_width - textPx) : 0;

    setCursor(startCol, row);
    print(text);
}

void IskakINO_OLED::printf(const char* fmt, ...) {
    char buf[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    print(buf);
}

void IskakINO_OLED::drawHLine(uint8_t startCol, uint8_t row, uint8_t width, uint8_t pattern) {
    if (row >= _pages || startCol >= _width) return;
    if (startCol + width > _width) width = _width - startCol;

    _setPageAndCol(startCol, row);
    uint8_t sent = 0;
    while (sent < width) {
        uint8_t chunk = (width - sent > 16) ? 16 : (width - sent);
        _wire->beginTransmission(_address);
        _wire->write(OLED_DATA_MODE);
        for (uint8_t k = 0; k < chunk; k++) {
            _wire->write(pattern);
        }
        _wire->endTransmission();
        sent += chunk;
    }
}

void IskakINO_OLED::drawProgressBar(uint8_t percent, uint8_t row, uint8_t startCol, uint8_t endCol) {
    if (row >= _pages || startCol >= endCol || endCol >= _width) return;
    if (percent > 100) percent = 100;

    uint8_t barWidth = endCol - startCol;
    uint8_t filledWidth = (uint16_t(barWidth - 2) * percent) / 100;

    _setPageAndCol(startCol, row);
    _wire->beginTransmission(_address);
    _wire->write(OLED_DATA_MODE);
    _wire->write(0x7E); // Garis batas kiri [
    _wire->endTransmission();

    // Isi bar
    _setPageAndCol(startCol + 1, row);
    for (uint8_t i = 0; i < barWidth - 2; i++) {
        _sendData((i < filledWidth) ? 0x7E : 0x42);
    }

    _setPageAndCol(endCol, row);
    _wire->beginTransmission(_address);
    _wire->write(OLED_DATA_MODE);
    _wire->write(0x7E); // Garis batas kanan ]
    _wire->endTransmission();
}

void IskakINO_OLED::drawIcon(const uint8_t* iconProgmem, uint8_t col, uint8_t row, uint8_t width) {
    if (!iconProgmem || row >= _pages || col >= _width) return;
    if (col + width > _width) width = _width - col;

    _setPageAndCol(col, row);
    _sendDataChunk(iconProgmem, width, true);
}

void IskakINO_OLED::drawWifiIcon(uint8_t level, uint8_t col, uint8_t row) {
    if (level > 4) level = 4;
    drawIcon(OLED_ICON_WIFI[level], col, row, 8);
}

void IskakINO_OLED::drawBatteryIcon(uint8_t level, uint8_t col, uint8_t row) {
    if (level > 5) level = 5;
    drawIcon(OLED_ICON_BATTERY[level], col, row, 12);
}

void IskakINO_OLED::drawBitmap(uint8_t col, uint8_t row, const uint8_t* bitmap, uint8_t w, uint8_t h_pages, bool isProgmem) {
    if (!bitmap || row >= _pages || col >= _width) return;

    for (uint8_t p = 0; p < h_pages; p++) {
        if (row + p >= _pages) break;
        _setPageAndCol(col, row + p);
        _sendDataChunk(bitmap + (p * w), w, isProgmem);
    }
}

// --- Animasi: Typewriter ---
void IskakINO_OLED::typewriterStart(const char* text, uint8_t col, uint8_t row, uint16_t intervalMs) {
    if (!text) return;
    strncpy(_animText, text, MAX_ANIM_TEXT_LEN);
    _animText[MAX_ANIM_TEXT_LEN] = '\0';

    _twTargetCol = col;
    _twTargetRow = row;
    _twIndex = 0;
    _twIntervalMs = intervalMs;
    _typewriterActive = true;
    _twTimerMark = millis();

    clearRow(row);
    setCursor(col, row);
}

void IskakINO_OLED::typewriterStart(const __FlashStringHelper* text, uint8_t col, uint8_t row, uint16_t intervalMs) {
    if (!text) return;
    strncpy_P(_animText, (const char*)text, MAX_ANIM_TEXT_LEN);
    _animText[MAX_ANIM_TEXT_LEN] = '\0';

    _twTargetCol = col;
    _twTargetRow = row;
    _twIndex = 0;
    _twIntervalMs = intervalMs;
    _typewriterActive = true;
    _twTimerMark = millis();

    clearRow(row);
    setCursor(col, row);
}

void IskakINO_OLED::typewriterStop() {
    _typewriterActive = false;
}

bool IskakINO_OLED::isTypewriterActive() const {
    return _typewriterActive;
}

// --- Animasi: Marquee Scroll ---
void IskakINO_OLED::scrollTextStart(const char* text, uint8_t row, uint16_t intervalMs) {
    if (!text) return;
    strncpy(_animText, text, MAX_ANIM_TEXT_LEN);
    _animText[MAX_ANIM_TEXT_LEN] = '\0';

    _scrollRow = row;
    _scrollOffset = 0;
    _scrollIntervalMs = intervalMs;
    _scrollActive = true;
    _scrollTimerMark = millis();

    clearRow(row);
}

void IskakINO_OLED::scrollTextStart(const __FlashStringHelper* text, uint8_t row, uint16_t intervalMs) {
    if (!text) return;
    strncpy_P(_animText, (const char*)text, MAX_ANIM_TEXT_LEN);
    _animText[MAX_ANIM_TEXT_LEN] = '\0';

    _scrollRow = row;
    _scrollOffset = 0;
    _scrollIntervalMs = intervalMs;
    _scrollActive = true;
    _scrollTimerMark = millis();

    clearRow(row);
}

void IskakINO_OLED::scrollTextStop() {
    _scrollActive = false;
}

bool IskakINO_OLED::isScrollActive() const {
    return _scrollActive;
}

void IskakINO_OLED::update() {
    uint32_t now = millis();

    // 1. Update Typewriter
    if (_typewriterActive) {
        if (now - _twTimerMark >= _twIntervalMs) {
            _twTimerMark = now;
            if (_animText[_twIndex] != '\0') {
                write((uint8_t)_animText[_twIndex]);
                _twIndex++;
            } else {
                _typewriterActive = false;
            }
        }
    }

    // 2. Update Marquee Scroll
    if (_scrollActive) {
        if (now - _scrollTimerMark >= _scrollIntervalMs) {
            _scrollTimerMark = now;
            size_t len = strlen(_animText);
            const uint8_t MAX_VISIBLE_CHARS = 21; // 128 / 6 = 21 karakter

            char windowBuf[MAX_VISIBLE_CHARS + 1];
            for (uint8_t i = 0; i < MAX_VISIBLE_CHARS; i++) {
                uint16_t charIdx = _scrollOffset + i;
                if (charIdx < len) {
                    windowBuf[i] = _animText[charIdx];
                } else if (charIdx < len + 4) {
                    windowBuf[i] = ' '; // Jeda antar pengulangan
                } else {
                    uint16_t wrapIdx = (charIdx - (len + 4)) % len;
                    windowBuf[i] = _animText[wrapIdx];
                }
            }
            windowBuf[MAX_VISIBLE_CHARS] = '\0';

            setCursor(0, _scrollRow);
            print(windowBuf);

            _scrollOffset++;
            if (_scrollOffset >= len + 4) {
                _scrollOffset = 0;
            }
        }
    }
}
