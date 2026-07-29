#include "IskakINO_LiquidCrystal_I2C.h"
#include <Arduino.h>
#include <Wire.h>

// ID slot pada _scheduler (lihat komentar di header)
#define ISKAKINO_LCD_SCHED_BACKLIGHT   0
#define ISKAKINO_LCD_SCHED_TYPEWRITER  1

// Definisi static member (FITUR #6)
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
      _progressBarReady(false)
{
    // Perilaku default sama seperti v1.1.0 lama: LCD_ENABLE_SERIAL_DEBUG
    // adalah flag compile-time; kalau di-set 1 sebelum #include, debug
    // otomatis aktif dari awal (sama seperti dulu). setDebug() runtime
    // (BARU) bisa mengubahnya kapan saja setelahnya.
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

    // FITUR #6: Wire.begin() cuma dipanggil sekali untuk SEMUA instance,
    // supaya konfigurasi pin custom (SDA/SCL) dari instance pertama
    // tidak ketiban/reset oleh instance LCD kedua/ketiga di bus yang sama.
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
    _scheduler.reset(ISKAKINO_LCD_SCHED_BACKLIGHT); // FITUR #3: hitung ini sebagai aktivitas
    _expanderWrite(0);
}

void LiquidCrystal_I2C::noBacklight() {
    _backlight = false;
    _expanderWrite(0);
}

// FITUR #3
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
    _scheduler.reset(ISKAKINO_LCD_SCHED_BACKLIGHT); // FITUR #3: reset idle timer setiap ada output
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
   FITUR #2: printCenter — overload const char* & String
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
   typewriter() versi BLOCKING (perilaku lama, dipertahankan)
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
   FITUR #1: typewriter NON-BLOCKING
========================================================= */
void LiquidCrystal_I2C::typewriterStart(const char* text, int row, int delayTime) {
    _twBuffer = text;          // disimpan sebagai String internal, aman dari lifetime issue
    _twIndex = 0;
    _twRow = row;
    _twIntervalMs = (uint16_t)delayTime;
    _scheduler.reset(ISKAKINO_LCD_SCHED_TYPEWRITER);
    _twActive = true;
    _twIsScrollMode = false;

    setCursor(0, row);
    // Bersihkan baris dulu supaya tidak tercampur teks lama
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
   FITUR #4: Auto horizontal scroll text (NON-BLOCKING)
========================================================= */
void LiquidCrystal_I2C::scrollTextStart(const char* text, int row, uint16_t intervalMs) {
    // Tambahkan spasi pemisah di akhir supaya scroll terlihat "looping" mulus
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
   update() — dipanggil tiap loop(), menangani SEMUA fitur non-blocking:
   - typewriterStart()
   - scrollTextStart()
   - setBacklightTimeout()
========================================================= */
void LiquidCrystal_I2C::update() {
    // --- Backlight auto-timeout (FITUR #3) ---
    // Dulu: `if (now - _lastActivityMillis >= _backlightTimeoutMs) noBacklight();`
    // — ini terpanggil ULANG setiap update() selama idle (I2C write redundan
    // tapi tidak berbahaya). Sekarang once(): noBacklight() cuma sekali per
    // idle-period, baru terpicu lagi setelah reset() (lihat titik reset di
    // begin()/backlight()/setBacklightTimeout()/write()). Efek yang TERLIHAT
    // user (backlight mati) identik.
    if (_backlightTimeoutMs > 0 && _backlight) {
        if (_scheduler.once(_backlightTimeoutMs, ISKAKINO_LCD_SCHED_BACKLIGHT)) {
            noBacklight();
        }
    }

    // --- Typewriter / Scroll (FITUR #1 & #4) ---
    if (!_twActive) return;
    if (!_scheduler.every(_twIntervalMs, ISKAKINO_LCD_SCHED_TYPEWRITER)) return;

    if (!_twIsScrollMode) {
        // Mode typewriter: cetak 1 karakter per tick
        if (_twIndex >= _twBuffer.length() || _twIndex >= _cols) {
            _twActive = false;
            return;
        }
        setCursor(_twIndex, _twRow);
        print(_twBuffer[_twIndex]);
        _twIndex++;
    } else {
        // Mode scroll: geser window sepanjang _cols melalui _twBuffer
        uint16_t totalLen = _twBuffer.length();
        if (totalLen == 0) {
            _twActive = false;
            return;
        }

        setCursor(0, _twRow);
        for (uint8_t i = 0; i < _cols; i++) {
            uint16_t charPos = (_twIndex + i) % totalLen;
            print(_twBuffer[charPos]);
        }

        _twIndex++;
        if (_twIndex >= totalLen) _twIndex = 0; // loop terus sampai scrollTextStop()
    }
}

/* =========================================================
   FITUR #5: Progress bar built-in
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
   FITUR #7: printf-style formatted print
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
