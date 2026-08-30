/*
 * src/oled/IskakINO_OLED.h
 * Modul driver Layar Grafis OLED I2C (SSD1306 & SH1106) untuk ekosistem IskakINO.
 *
 * Fitur:
 *  - Ultra-hemat RAM: Rendering berbasis Page Streaming (< 40 bytes RAM),
 *    bebas beban buffer 1024 bytes bawaan library umum.
 *  - Kompatibel penuh dengan SSD1306 (128x64 & 128x32) dan SH1106 (128x64).
 *  - Auto-scan alamat I2C (0x3C / 0x3D) otomatis saat begin().
 *  - Animasi Non-Blocking: Typewriter, Marquee Scrolling, & Progress Bar.
 *  - Generator Ikon Grafis Siap Pakai: Sinyal WiFi (0-4), Baterai (0-5), Simbol Status.
 *  - Dukungan teks ganda (Size 1x & 2x) dan fungsi helper printCenter / printRight.
 *  - Kompatibel lintas platform: Arduino AVR (Uno/Nano/Mega), ESP8266, & ESP32.
 */

#ifndef ISKAKINO_OLED_H
#define ISKAKINO_OLED_H

#include <Arduino.h>
#include <Wire.h>
#include <Print.h>
#include "../core/IskakINO_Platform.h"
#include "IskakINO_OLEDFonts.h"

enum IskakOLEDChip : uint8_t {
    OLED_CHIP_AUTO = 0,
    OLED_CHIP_SSD1306,
    OLED_CHIP_SH1106
};

class IskakINO_OLED : public Print {
public:
    // Konstruktor
    // width: lebar pixel (default 128)
    // height: tinggi pixel (default 64, atau 32)
    // i2cAddress: 0x3C (default) atau 0x3D (atau 0x00 untuk auto-scan)
    explicit IskakINO_OLED(uint8_t width = 128, uint8_t height = 64, uint8_t i2cAddress = 0x3C);

    // Inisialisasi komunikasi I2C dan inisialisasi chip OLED
    bool begin(TwoWire &wirePort = Wire);
    bool begin(uint8_t width, uint8_t height, uint8_t i2cAddress = 0x3C, TwoWire &wirePort = Wire);

    // Status deteksi I2C
    bool isConnected() const;
    uint8_t getAddress() const;

    // Refresh state machine animasi non-blocking (Wajib dipanggil di loop())
    void update();

    // --- Kontrol Tampilan Dasar ---
    void clear();
    void clearRow(uint8_t row); // row: 0 s/d 7 (1 page = 8 pixel tinggi)
    void clearArea(uint8_t startCol, uint8_t row, uint8_t widthCols);
    void setCursor(uint8_t col, uint8_t row); // col: 0..127, row: 0..7
    void setTextSize(uint8_t size); // 1 = Normal (5x7 + 1px), 2 = Double (10x14)
    void invertDisplay(bool invert);
    void setContrast(uint8_t contrast); // 0 s/d 255
    void displayOn();
    void displayOff();
    void setChipType(IskakOLEDChip chip);

    // --- Pencetakan Teks & Format (Implementasi Print class) ---
    size_t write(uint8_t c) override;
    void printCenter(const char* text, uint8_t row);
    void printCenter(const __FlashStringHelper* text, uint8_t row);
    void printRight(const char* text, uint8_t row);
    void printRight(const __FlashStringHelper* text, uint8_t row);
    void printf(const char* fmt, ...);

    // --- Grafis & Ikon Helper ---
    void drawHLine(uint8_t startCol, uint8_t row, uint8_t width, uint8_t pattern = 0x01);
    void drawProgressBar(uint8_t percent, uint8_t row, uint8_t startCol = 0, uint8_t endCol = 127);
    void drawWifiIcon(uint8_t level, uint8_t col, uint8_t row); // level: 0..4
    void drawBatteryIcon(uint8_t level, uint8_t col, uint8_t row); // level: 0..5
    void drawIcon(const uint8_t* iconProgmem, uint8_t col, uint8_t row, uint8_t width = 8);
    void drawBitmap(uint8_t col, uint8_t row, const uint8_t* bitmap, uint8_t w, uint8_t h_pages = 1, bool isProgmem = true);

    // --- Animasi Non-Blocking: Typewriter ---
    void typewriterStart(const char* text, uint8_t col = 0, uint8_t row = 0, uint16_t intervalMs = 60);
    void typewriterStart(const __FlashStringHelper* text, uint8_t col = 0, uint8_t row = 0, uint16_t intervalMs = 60);
    void typewriterStop();
    bool isTypewriterActive() const;

    // --- Animasi Non-Blocking: Marquee Scroll ---
    void scrollTextStart(const char* text, uint8_t row = 0, uint16_t intervalMs = 150);
    void scrollTextStart(const __FlashStringHelper* text, uint8_t row = 0, uint16_t intervalMs = 150);
    void scrollTextStop();
    bool isScrollActive() const;

private:
    uint8_t _width;
    uint8_t _height;
    uint8_t _pages;
    uint8_t _address;
    TwoWire* _wire;
    bool _connected;
    IskakOLEDChip _chip;

    uint8_t _cursorCol;
    uint8_t _cursorRow;
    uint8_t _textSize;

    // Buffer internal kecil untuk typewriter & scrolling teks
    // Max 48 karakter cukup untuk sebagian besar pesan (hanya ~48 bytes RAM!)
    static const uint8_t MAX_ANIM_TEXT_LEN = 48;
    char _animText[MAX_ANIM_TEXT_LEN + 1];

    // State Typewriter
    bool _typewriterActive;
    uint8_t _twTargetCol;
    uint8_t _twTargetRow;
    uint8_t _twIndex;
    uint16_t _twIntervalMs;
    uint32_t _twTimerMark;

    // State Scroll (Marquee)
    bool _scrollActive;
    uint8_t _scrollRow;
    uint16_t _scrollOffset;
    uint16_t _scrollIntervalMs;
    uint32_t _scrollTimerMark;

    // Helper Low-Level I2C
    void _sendCommand(uint8_t cmd);
    void _sendCommand2(uint8_t cmd1, uint8_t cmd2);
    void _sendCommand3(uint8_t cmd1, uint8_t cmd2, uint8_t cmd3);
    void _sendData(uint8_t data);
    void _sendDataChunk(const uint8_t* data, size_t len, bool isProgmem = false);
    void _setPageAndCol(uint8_t col, uint8_t page);
    void _initDisplay();
    void _drawChar5x7(uint8_t c);
    void _drawChar2x(uint8_t c);
};

#endif // ISKAKINO_OLED_H
