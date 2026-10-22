#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Print.h>
#include <stdarg.h>

#include "core/IskakINO_Scheduler.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Version.h"
#include "IskakINO_LCD_Icons.h"

/* =========================================================
   Compile-time configuration
========================================================= */
#ifndef LCD_ENABLE_SERIAL_DEBUG
#define LCD_ENABLE_SERIAL_DEBUG 0
#endif

// Ukuran buffer internal untuk printFormatted(). Bisa di-override
// sebelum #include jika butuh string lebih panjang.
#ifndef LCD_PRINTF_BUFFER_SIZE
#define LCD_PRINTF_BUFFER_SIZE 40
#endif

/* =========================================================
   PCF8574 Pin Mapping
========================================================= */
#define En 0b00000100
#define Rw 0b00000010
#define Rs 0b00000001
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

/* =========================================================
   HD44780 Commands
========================================================= */
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

/* =========================================================
   Entry Mode Flags
========================================================= */
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/* =========================================================
   Display Control Flags
========================================================= */
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

/* =========================================================
   Function Set Flags
========================================================= */
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

/* =========================================================
   Cursor / Display Shift Flags
========================================================= */
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

/* =========================================================
   Reserved custom-character slots
   (0-7 tersedia di HD44780; slot 7 dipakai internal utk progress bar)
========================================================= */
#define LCD_PROGRESSBAR_CHAR_LOC 7

// Forward declaration
class LiquidCrystal_I2C;

/* =========================================================
   Dynamic Banner / Page Flipper Helper Types
========================================================= */
struct LCDPage {
    const char* line1;
    const char* line2;
    const char* line3;
    const char* line4;

    LCDPage() : line1(nullptr), line2(nullptr), line3(nullptr), line4(nullptr) {}
    LCDPage(const char* l1, const char* l2 = nullptr, const char* l3 = nullptr, const char* l4 = nullptr)
        : line1(l1), line2(l2), line3(l3), line4(l4) {}
};

typedef void (*LCDBannerCallback)(LiquidCrystal_I2C& lcd, uint8_t pageIndex);

/* =========================================================
   Class Definition
========================================================= */
class LiquidCrystal_I2C : public Print {
public:
    LiquidCrystal_I2C(uint8_t cols, uint8_t rows);

    /* Basic control */
    void begin();
    void clear();
    void home();
    void setCursor(uint8_t col, uint8_t row);

    /* Display control */
    void display();
    void noDisplay();
    void cursor();
    void noCursor();
    void blink();
    void noBlink();

    /* Backlight */
    void backlight();
    void noBacklight();

    // Auto-off backlight setelah idle sekian ms. 0 = nonaktif (default).
    // WAJIB panggil update() di loop() supaya timer ini berjalan.
    void setBacklightTimeout(unsigned long timeoutMs);

    /* Scrolling (bawaan HD44780 - geser seluruh display) */
    void scrollDisplayLeft();
    void scrollDisplayRight();
    void leftToRight();
    void rightToLeft();
    void autoscroll();
    void noAutoscroll();

    /* Address control */
    void setAddress(uint8_t addr);
    uint8_t getAddress() const;

    /* Custom character */
    void createChar(uint8_t location, const uint8_t charmap[]);

    /* Print override */
    virtual size_t write(uint8_t);

    /* ===================== Text Helpers ===================== */
    void printCenter(const char* text, int row);
    void printCenter(const String& text, int row);

    // Versi BLOCKING (backward compatibility)
    void typewriter(const char* text, int row, int delayTime = 100);
    void typewriter(const String& text, int row, int delayTime = 100);

    /* ================ Non-blocking typewriter ================ */
    void typewriterStart(const char* text, int row, int delayTime = 100);
    void typewriterStart(const String& text, int row, int delayTime = 100);
    void typewriterStop();
    bool isTypewriterActive() const;

    /* ================ Auto horizontal scroll text (Non-blocking) ================ */
    void scrollTextStart(const char* text, int row, uint16_t intervalMs = 300);
    void scrollTextStart(const String& text, int row, uint16_t intervalMs = 300);
    void scrollTextStop();
    bool isScrollActive() const;

    /* ================ Dynamic Banner / Page Flipper ================ */
    // Menjalankan banner multi-halaman teks statis
    void bannerStart(const LCDPage* pages, uint8_t pageCount, uint16_t flipIntervalMs = 3000);
    // Menjalankan banner multi-halaman dengan callback dinamis (untuk data sensor/realtime)
    void bannerStart(uint8_t pageCount, uint16_t flipIntervalMs, LCDBannerCallback callback);
    void bannerStop();
    void bannerPause();
    void bannerResume();
    void bannerNext();
    void bannerPrev();
    void bannerSetPage(uint8_t pageIndex);
    uint8_t bannerGetCurrentPage() const { return _bannerCurrentPage; }
    uint8_t bannerGetPageCount() const { return _bannerPageCount; }
    bool isBannerActive() const { return _bannerActive; }
    bool isBannerPaused() const { return _bannerPaused; }
    void bannerSetInterval(uint16_t intervalMs);

    /* ================ Custom Icon Generator Helpers ================ */
    // Membuat custom character baterai pada slot tertentu (0-100%)
    void createBatteryIcon(uint8_t slot, uint8_t percent);
    // Membuat custom character bar sinyal Wi-Fi (0-4 bar)
    void createWifiIcon(uint8_t slot, uint8_t level_0_to_4);
    // Membuat custom character bar sinyal Wi-Fi dari nilai dBm RSSI (mis. -65 dBm)
    void createWifiIconRssi(uint8_t slot, int rssi);
    // Membuat custom character termometer (level 0-3)
    void createThermometerIcon(uint8_t slot, uint8_t level_0_to_3);

    // Helper langsung gambar custom icon pada koordinat (col, row)
    void drawBattery(uint8_t col, uint8_t row, uint8_t percent, uint8_t slot = 0);
    void drawWifiSignal(uint8_t col, uint8_t row, uint8_t level_0_to_4, uint8_t slot = 1);
    void drawWifiSignalRssi(uint8_t col, uint8_t row, int rssi, uint8_t slot = 1);
    void drawThermometer(uint8_t col, uint8_t row, uint8_t level_0_to_3, uint8_t slot = 2);

    /* Progress bar built-in (0-100%) */
    void drawProgressBar(uint8_t percent, uint8_t row);

    /* printf-style formatted print */
    void printFormatted(const char* format, ...);

    /* Panggil sekali tiap loop() */
    void update();

    bool isConnected();

    void setDebug(bool debugMode) { _logger.setDebug(debugMode); }

    static void useExternalWireBegin();

private:
    void _command(uint8_t value);
    void _send(uint8_t value, uint8_t mode);
    void _write4bits(uint8_t value);
    void _pulseEnable(uint8_t data);
    void _expanderWrite(uint8_t data);
    void _scanAddress();
    void _printCenterImpl(const char* text, int row);
    void _typewriterBlockingImpl(const char* text, int row, int delayTime);
    void _renderBannerPage();
#ifndef ISKAKINO_NO_SPLASH
    void _showSplashScreen();
#endif

    uint8_t _cols;
    uint8_t _rows;
    uint8_t _addr;
    bool _backlight;
    bool _initialized;
    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;

    static bool _wireInitialized;

    // --- State non-blocking ---
    String _twBuffer;
    uint8_t _twIndex;
    uint8_t _twRow;
    uint16_t _twIntervalMs;
    bool _twActive;
    bool _twIsScrollMode;

    unsigned long _backlightTimeoutMs;

    // --- State Banner / Page Flipper ---
    const LCDPage* _bannerPages;
    LCDBannerCallback _bannerCallback;
    uint8_t _bannerPageCount;
    uint8_t _bannerCurrentPage;
    uint16_t _bannerIntervalMs;
    bool _bannerActive;
    bool _bannerPaused;

    IskakINO_Scheduler _scheduler{3};
    IskakINO_Logger _logger;

    bool _progressBarReady;
};
