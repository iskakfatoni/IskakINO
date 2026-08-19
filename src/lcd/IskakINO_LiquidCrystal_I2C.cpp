#include "IskakINO_LiquidCrystal_I2C.h"
#include <Arduino.h>
#include <Wire.h>

// ID slot pada _scheduler
#define ISKAKINO_LCD_SCHED_BACKLIGHT   0
#define ISKAKINO_LCD_SCHED_TYPEWRITER  1
#define ISKAKINO_LCD_SCHED_BANNER      2

// Definisi static member
bool LiquidCrystal_I2C::_wireInitialized = false;

// Fix regresi v1.1.0: dipanggil user setelah Wire.begin(SDA, SCL) manual
void LiquidCrystal_I2C::useExternalWireBegin() {
    _wireInitialized = true;
}

/* =========================================================
   Constructor
========================================================= */
LiquidCrystal_I2C::LiquidCrystal_I2C(uint8_t cols, uint8_t rows)
    : _cols(cols),
      _rows(rows),
      _addr(0x00),
      _backlight(true),
      _initialized(false),
      _displayfunction(LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS),
      _displaycontrol(0),
      _displaymode(0),
      _twIndex(0),
      _twRow(0),
      _twIntervalMs(100),
      _twActive(false),
      _twIsScrollMode(false),
      _backlightTimeoutMs(0),
      _bannerPages(nullptr),
      _bannerCallback(nullptr),
      _bannerPageCount(0),
      _bannerCurrentPage(0),
      _bannerIntervalMs(3000),
      _bannerActive(false),
      _bannerPaused(false),
      _progressBarReady(false)
{
    _logger.setDebug(LCD_ENABLE_SERIAL_DEBUG ? true : false);
}

/* =========================================================
   Address control
========================================================= */
void LiquidCrystal_I2C::setAddress(uint8_t addr) {
    _addr = addr;

    if (_logger.isDebug()) {
        Serial.print(F("[LCD] Manual I2C address set: 0x"));
        Serial.println(_addr, HEX);
    }

    if (_initialized) {
        _initialized = false;
        begin();
    }
}

uint8_t LiquidCrystal_I2C::getAddress() const {
    return _addr;
}

void LiquidCrystal_I2C::createChar(uint8_t location, const uint8_t charmap[]) {
    location &= 0x7;
    _command(LCD_SETCGRAMADDR | (location << 3));
    for (uint8_t i = 0; i < 8; i++) {
        write(charmap[i]);
    }
}

/* =========================================================
   Init
========================================================= */
void LiquidCrystal_I2C::begin() {
    if (_initialized) return;

    if (!_wireInitialized) {
        Wire.begin();
        Wire.setClock(100000);
        _wireInitialized = true;
    }

    _scanAddress();
    if (_addr == 0x00) return;

    if (_rows > 1) {
        _displayfunction |= LCD_2LINE;
    }

    delay(50);
    _write4bits(0x03 << 4);
    delayMicroseconds(4500);
    _write4bits(0x03 << 4);
    delayMicroseconds(4500);
    _write4bits(0x03 << 4);
    delayMicroseconds(150);
    _write4bits(0x02 << 4);

    _command(LCD_FUNCTIONSET | _displayfunction);

    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();
    clear();

    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    _command(LCD_ENTRYMODESET | _displaymode);
    home();

    _initialized = true;
    _scheduler.reset(ISKAKINO_LCD_SCHED_BACKLIGHT);

#ifndef ISKAKINO_NO_SPLASH
    _showSplashScreen();
#endif
}

void LiquidCrystal_I2C::clear() {
    _command(LCD_CLEARDISPLAY);
    delayMicroseconds(2000);
}

void LiquidCrystal_I2C::home() {
    _command(LCD_RETURNHOME);
    delayMicroseconds(2000);
}

void LiquidCrystal_I2C::setCursor(uint8_t col, uint8_t row) {
    static const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row >= _rows) row = _rows - 1;
    _command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LiquidCrystal_I2C::display() {
    _displaycontrol |= LCD_DISPLAYON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::noDisplay() {
    _displaycontrol &= ~LCD_DISPLAYON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::cursor() {
    _displaycontrol |= LCD_CURSORON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::noCursor() {
    _displaycontrol &= ~LCD_CURSORON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::blink() {
    _displaycontrol |= LCD_BLINKON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::noBlink() {
    _displaycontrol &= ~LCD_BLINKON;
    _command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LiquidCrystal_I2C::backlight() {
    _backlight = true;
    _scheduler.reset(ISKAKINO_LCD_SCHED_BACKLIGHT);
    _expanderWrite(0);
}

void LiquidCrystal_I2C::noBacklight() {
    _backlight = false;
    _expanderWrite(0);
}

void LiquidCrystal_I2C::setBacklightTimeout(unsigned long timeoutMs) {
    _backlightTimeoutMs = timeoutMs;
    _scheduler.reset(ISKAKINO_LCD_SCHED_BACKLIGHT);
}

void LiquidCrystal_I2C::scrollDisplayLeft() {
    _command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LiquidCrystal_I2C::scrollDisplayRight() {
    _command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void LiquidCrystal_I2C::leftToRight() {
    _displaymode |= LCD_ENTRYLEFT;
    _command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal_I2C::rightToLeft() {
    _displaymode &= ~LCD_ENTRYLEFT;
    _command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal_I2C::autoscroll() {
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    _command(LCD_ENTRYMODESET | _displaymode);
}

void LiquidCrystal_I2C::noAutoscroll() {
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    _command(LCD_ENTRYMODESET | _displaymode);
}

size_t LiquidCrystal_I2C::write(uint8_t value) {
    _scheduler.reset(ISKAKINO_LCD_SCHED_BACKLIGHT);
    _send(value, Rs);
    return 1;
}

/* =========================================================
   Low-level command handling
========================================================= */
void LiquidCrystal_I2C::_command(uint8_t value) {
    _send(value, 0);
}

void LiquidCrystal_I2C::_send(uint8_t value, uint8_t mode) {
    if (_addr == 0x00) return;

    uint8_t highnib = value & 0xF0;
    uint8_t lownib = (value << 4) & 0xF0;
    _write4bits(highnib | mode);
    _write4bits(lownib | mode);
}

void LiquidCrystal_I2C::_write4bits(uint8_t value) {
    _expanderWrite(value);
    _pulseEnable(value);
}

void LiquidCrystal_I2C::_pulseEnable(uint8_t data) {
    _expanderWrite(data | En);
    delayMicroseconds(1);
    _expanderWrite(data & ~En);
    delayMicroseconds(50);
}

void LiquidCrystal_I2C::_expanderWrite(uint8_t data) {
    if (_addr == 0x00) return;

    Wire.beginTransmission(_addr);
    Wire.write(data | (_backlight ? LCD_BACKLIGHT : LCD_NOBACKLIGHT));
    Wire.endTransmission();
}

/* =========================================================
   I2C Address Auto Scan
========================================================= */
void LiquidCrystal_I2C::_scanAddress() {
    if (_addr != 0x00) return;

    for (uint8_t addr = 0x20; addr <= 0x27; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            _addr = addr;
            if (_logger.isDebug()) {
                Serial.print(F("[LCD] Found at 0x"));
                Serial.println(_addr, HEX);
            }
            return;
        }
    }

    for (uint8_t addr = 0x38; addr <= 0x3F; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            _addr = addr;
            if (_logger.isDebug()) {
                Serial.print(F("[LCD] Found at 0x"));
                Serial.println(_addr, HEX);
            }
            return;
        }
    }

    _logger.log(F("[LCD] I2C device NOT found"));
}

/* =========================================================
   printCenter — overload const char* & String
========================================================= */
void LiquidCrystal_I2C::_printCenterImpl(const char* text, int row) {
    int len = strlen(text);
    int pos = (_cols - len) / 2;
    if (pos < 0) pos = 0;

    setCursor(pos, row);
    print(text);
}

void LiquidCrystal_I2C::printCenter(const char* text, int row) {
    _printCenterImpl(text, row);
}

void LiquidCrystal_I2C::printCenter(const String& text, int row) {
    _printCenterImpl(text.c_str(), row);
}

/* =========================================================
   typewriter() versi BLOCKING
========================================================= */
void LiquidCrystal_I2C::_typewriterBlockingImpl(const char* text, int row, int delayTime) {
    setCursor(0, row);
    size_t len = strlen(text);
    for (size_t i = 0; i < len; i++) {
        if (i < (size_t)_cols) {
            print(text[i]);
            delay(delayTime);
        }
    }
}

void LiquidCrystal_I2C::typewriter(const char* text, int row, int delayTime) {
    _typewriterBlockingImpl(text, row, delayTime);
}

void LiquidCrystal_I2C::typewriter(const String& text, int row, int delayTime) {
    _typewriterBlockingImpl(text.c_str(), row, delayTime);
}

/* =========================================================
   typewriter NON-BLOCKING
========================================================= */
void LiquidCrystal_I2C::typewriterStart(const char* text, int row, int delayTime) {
    _twBuffer = text;
    _twIndex = 0;
    _twRow = row;
    _twIntervalMs = (uint16_t)delayTime;
    _scheduler.reset(ISKAKINO_LCD_SCHED_TYPEWRITER);
    _twActive = true;
    _twIsScrollMode = false;

    setCursor(0, row);
    for (uint8_t i = 0; i < _cols; i++) print(' ');
    setCursor(0, row);
}

void LiquidCrystal_I2C::typewriterStart(const String& text, int row, int delayTime) {
    typewriterStart(text.c_str(), row, delayTime);
}

void LiquidCrystal_I2C::typewriterStop() {
    _twActive = false;
}

bool LiquidCrystal_I2C::isTypewriterActive() const {
    return _twActive && !_twIsScrollMode;
}

/* =========================================================
   Auto horizontal scroll text (NON-BLOCKING)
========================================================= */
void LiquidCrystal_I2C::scrollTextStart(const char* text, int row, uint16_t intervalMs) {
    _twBuffer = text;
    _twBuffer += "   ";

    _twIndex = 0;
    _twRow = row;
    _twIntervalMs = intervalMs;
    _scheduler.reset(ISKAKINO_LCD_SCHED_TYPEWRITER);
    _twActive = true;
    _twIsScrollMode = true;
}

void LiquidCrystal_I2C::scrollTextStart(const String& text, int row, uint16_t intervalMs) {
    scrollTextStart(text.c_str(), row, intervalMs);
}

void LiquidCrystal_I2C::scrollTextStop() {
    _twActive = false;
}

bool LiquidCrystal_I2C::isScrollActive() const {
    return _twActive && _twIsScrollMode;
}

/* =========================================================
   Dynamic Banner / Page Flipper
========================================================= */
void LiquidCrystal_I2C::bannerStart(const LCDPage* pages, uint8_t pageCount, uint16_t flipIntervalMs) {
    if (pages == nullptr || pageCount == 0) return;
    _bannerPages = pages;
    _bannerCallback = nullptr;
    _bannerPageCount = pageCount;
    _bannerCurrentPage = 0;
    _bannerIntervalMs = flipIntervalMs;
    _bannerActive = true;
    _bannerPaused = false;
    _scheduler.reset(ISKAKINO_LCD_SCHED_BANNER);
    _renderBannerPage();
}

void LiquidCrystal_I2C::bannerStart(uint8_t pageCount, uint16_t flipIntervalMs, LCDBannerCallback callback) {
    if (callback == nullptr || pageCount == 0) return;
    _bannerPages = nullptr;
    _bannerCallback = callback;
    _bannerPageCount = pageCount;
    _bannerCurrentPage = 0;
    _bannerIntervalMs = flipIntervalMs;
    _bannerActive = true;
    _bannerPaused = false;
    _scheduler.reset(ISKAKINO_LCD_SCHED_BANNER);
    _renderBannerPage();
}

void LiquidCrystal_I2C::bannerStop() {
    _bannerActive = false;
    _bannerPaused = false;
}

void LiquidCrystal_I2C::bannerPause() {
    _bannerPaused = true;
}

void LiquidCrystal_I2C::bannerResume() {
    _bannerPaused = false;
    _scheduler.reset(ISKAKINO_LCD_SCHED_BANNER);
}

void LiquidCrystal_I2C::bannerNext() {
    if (!_bannerActive || _bannerPageCount == 0) return;
    _bannerCurrentPage = (_bannerCurrentPage + 1) % _bannerPageCount;
    _scheduler.reset(ISKAKINO_LCD_SCHED_BANNER);
    _renderBannerPage();
}

void LiquidCrystal_I2C::bannerPrev() {
    if (!_bannerActive || _bannerPageCount == 0) return;
    _bannerCurrentPage = (_bannerCurrentPage == 0) ? (_bannerPageCount - 1) : (_bannerCurrentPage - 1);
    _scheduler.reset(ISKAKINO_LCD_SCHED_BANNER);
    _renderBannerPage();
}

void LiquidCrystal_I2C::bannerSetPage(uint8_t pageIndex) {
    if (!_bannerActive || pageIndex >= _bannerPageCount) return;
    _bannerCurrentPage = pageIndex;
    _scheduler.reset(ISKAKINO_LCD_SCHED_BANNER);
    _renderBannerPage();
}

void LiquidCrystal_I2C::bannerSetInterval(uint16_t intervalMs) {
    _bannerIntervalMs = intervalMs;
}

void LiquidCrystal_I2C::_renderBannerPage() {
    if (!_initialized) return;

    if (_bannerCallback != nullptr) {
        clear();
        _bannerCallback(*this, _bannerCurrentPage);
    } else if (_bannerPages != nullptr) {
        clear();
        const LCDPage& p = _bannerPages[_bannerCurrentPage];
        const char* lines[4] = { p.line1, p.line2, p.line3, p.line4 };
        for (uint8_t r = 0; r < _rows && r < 4; r++) {
            if (lines[r] != nullptr) {
                setCursor(0, r);
                print(lines[r]);
            }
        }
    }
}

/* =========================================================
   Custom Icon Generator Helpers
========================================================= */
void LiquidCrystal_I2C::createBatteryIcon(uint8_t slot, uint8_t percent) {
    uint8_t map[8];
    generateBatteryIcon(percent, map);
    createChar(slot, map);
}

void LiquidCrystal_I2C::createWifiIcon(uint8_t slot, uint8_t level_0_to_4) {
    uint8_t map[8];
    generateWifiIcon(level_0_to_4, map);
    createChar(slot, map);
}

void LiquidCrystal_I2C::createWifiIconRssi(uint8_t slot, int rssi) {
    createWifiIcon(slot, rssiToWifiBars(rssi));
}

void LiquidCrystal_I2C::createThermometerIcon(uint8_t slot, uint8_t level_0_to_3) {
    uint8_t map[8];
    generateThermometerIcon(level_0_to_3, map);
    createChar(slot, map);
}

void LiquidCrystal_I2C::drawBattery(uint8_t col, uint8_t row, uint8_t percent, uint8_t slot) {
    createBatteryIcon(slot, percent);
    setCursor(col, row);
    write((uint8_t)slot);
}

void LiquidCrystal_I2C::drawWifiSignal(uint8_t col, uint8_t row, uint8_t level_0_to_4, uint8_t slot) {
    createWifiIcon(slot, level_0_to_4);
    setCursor(col, row);
    write((uint8_t)slot);
}

void LiquidCrystal_I2C::drawWifiSignalRssi(uint8_t col, uint8_t row, int rssi, uint8_t slot) {
    createWifiIconRssi(slot, rssi);
    setCursor(col, row);
    write((uint8_t)slot);
}

void LiquidCrystal_I2C::drawThermometer(uint8_t col, uint8_t row, uint8_t level_0_to_3, uint8_t slot) {
    createThermometerIcon(slot, level_0_to_3);
    setCursor(col, row);
    write((uint8_t)slot);
}

/* =========================================================
   update() — dipanggil tiap loop()
========================================================= */
void LiquidCrystal_I2C::update() {
    // --- Backlight auto-timeout ---
    if (_backlightTimeoutMs > 0 && _backlight) {
        if (_scheduler.once(_backlightTimeoutMs, ISKAKINO_LCD_SCHED_BACKLIGHT)) {
            noBacklight();
        }
    }

    // --- Typewriter / Scroll ---
    if (_twActive) {
        if (_scheduler.every(_twIntervalMs, ISKAKINO_LCD_SCHED_TYPEWRITER)) {
            if (!_twIsScrollMode) {
                if (_twIndex >= _twBuffer.length() || _twIndex >= _cols) {
                    _twActive = false;
                } else {
                    setCursor(_twIndex, _twRow);
                    print(_twBuffer[_twIndex]);
                    _twIndex++;
                }
            } else {
                uint16_t totalLen = _twBuffer.length();
                if (totalLen == 0) {
                    _twActive = false;
                } else {
                    setCursor(0, _twRow);
                    for (uint8_t i = 0; i < _cols; i++) {
                        uint16_t charPos = (_twIndex + i) % totalLen;
                        print(_twBuffer[charPos]);
                    }
                    _twIndex++;
                    if (_twIndex >= totalLen) _twIndex = 0;
                }
            }
        }
    }

    // --- Dynamic Banner / Page Flipper ---
    if (_bannerActive && !_bannerPaused && _bannerPageCount > 1) {
        if (_scheduler.every(_bannerIntervalMs, ISKAKINO_LCD_SCHED_BANNER)) {
            _bannerCurrentPage = (_bannerCurrentPage + 1) % _bannerPageCount;
            _renderBannerPage();
        }
    }
}

/* =========================================================
   Progress bar built-in
========================================================= */
void LiquidCrystal_I2C::drawProgressBar(uint8_t percent, uint8_t row) {
    if (percent > 100) percent = 100;

    if (!_progressBarReady) {
        static const uint8_t block[8] = {
            0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F
        };
        createChar(LCD_PROGRESSBAR_CHAR_LOC, block);
        _progressBarReady = true;
    }

    uint8_t filled = (uint16_t)percent * _cols / 100;

    setCursor(0, row);
    for (uint8_t i = 0; i < _cols; i++) {
        write(i < filled ? (uint8_t)LCD_PROGRESSBAR_CHAR_LOC : (uint8_t)' ');
    }
}

/* =========================================================
   printf-style formatted print
========================================================= */
void LiquidCrystal_I2C::printFormatted(const char* format, ...) {
    char buffer[LCD_PRINTF_BUFFER_SIZE];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    print(buffer);
}

/* =========================================================
   isConnected()
========================================================= */
bool LiquidCrystal_I2C::isConnected() {
    Wire.beginTransmission(_addr);
    return (Wire.endTransmission() == 0);
}

#ifndef ISKAKINO_NO_SPLASH
void LiquidCrystal_I2C::_showSplashScreen() {
    backlight();
    clear();

    setCursor(0, 0);
    print("@iskakfatoni");

    setCursor(0, 1);
    print("I2C Addr: 0x");
    if (_addr < 0x10) print("0");
    print(_addr, HEX);

    delay(2000);
    clear();
    home();
}
#endif
