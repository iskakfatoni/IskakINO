#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Print.h>
#include <stdarg.h>

#include "core/IskakINO_Scheduler.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Version.h"

// PILOT REFACTOR #5 — bagian dari penggabungan ekosistem IskakINO. Signature
// semua fungsi PUBLIK di bawah TIDAK BERUBAH dari v1.1.0 standalone. Yang
// berubah cuma internal:
//   - _lastActivityMillis (backlight auto-timeout) & _twLastMillis
//     (interval typewriter/scroll) — dulu masing-masing variabel millis()
//     manual — kini satu IskakINO_Scheduler bersama (id 0 = backlight,
//     id 1 = typewriter/scroll). Titik reset (begin()/backlight()/
//     setBacklightTimeout()/write() untuk id 0; typewriterStart()/
//     scrollTextStart() untuk id 1) dipertahankan PERSIS di tempat yang
//     sama, cuma manggil _scheduler.reset() alih-alih assignment manual.
//     Efek samping kecil yang MENGUNTUNGKAN: backlight timeout dulu
//     memanggil noBacklight() BERULANG setiap update() selama idle
//     (redundant I2C write, tidak berbahaya tapi boros bus); sekarang
//     cuma sekali per idle-period berkat semantik once(). Perilaku yang
//     TERLIHAT USER (backlight mati) sama persis.
//   - LCD_ENABLE_SERIAL_DEBUG (dulu flag compile-time) kini juga bisa
//     dikontrol runtime lewat setDebug() (BARU, opsional) via
//     IskakINO_Logger (_logger). Kalau LCD_ENABLE_SERIAL_DEBUG=1 di-set
//     sebelum #include (cara lama), constructor otomatis panggil
//     _logger.setDebug(true) supaya perilaku lama tetap sama persis.
//     Dua baris yang mencetak alamat I2C dalam HEX tetap Serial.print
//     mentah (Logger belum dukung format HEX), tapi tetap digerbang oleh
//     _logger.isDebug() supaya konsisten dgn setDebug().

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

    // FITUR #3 (v1.1.0): Auto-off backlight setelah idle sekian ms.
    // 0 = nonaktif (default). Aktivitas dihitung dari setiap write()/print().
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
    // Bisa dipanggil kapan saja. Jika dipanggil setelah begin(),
    // LCD otomatis di-reinit dengan alamat baru.
    void setAddress(uint8_t addr);
    uint8_t getAddress() const;

    /* Custom character */
    void createChar(uint8_t location, const uint8_t charmap[]);

    /* Print override */
    virtual size_t write(uint8_t);

    /* ===================== Text Helpers ===================== */
    // FITUR #2 (v1.1.0): overload const char* di samping String,
    // supaya bisa pakai string literal langsung tanpa alokasi String
    // (lebih hemat RAM di board kecil seperti Uno/Nano).
    void printCenter(const char* text, int row);
    void printCenter(const String& text, int row);

    // Versi BLOCKING (perilaku lama, dipertahankan untuk backward compatibility)
    void typewriter(const char* text, int row, int delayTime = 100);
    void typewriter(const String& text, int row, int delayTime = 100);

    /* ================ FITUR #1 (v1.1.0): Non-blocking typewriter ================
       Pola pemakaian:
         lcd.typewriterStart("Hello World", 0);
         // di loop():
         lcd.update();
    ================================================================= */
    void typewriterStart(const char* text, int row, int delayTime = 100);
    void typewriterStart(const String& text, int row, int delayTime = 100);
    void typewriterStop();
    bool isTypewriterActive() const;

    /* ================ FITUR #4 (v1.1.0): Auto horizontal scroll text ================
       Untuk teks yang lebih panjang dari lebar layar (mis. nama lagu, status panjang).
       Non-blocking, jalan lewat update().
    ================================================================= */
    void scrollTextStart(const char* text, int row, uint16_t intervalMs = 300);
    void scrollTextStart(const String& text, int row, uint16_t intervalMs = 300);
    void scrollTextStop();
    bool isScrollActive() const;

    // Panggil sekali tiap loop() — menjalankan timer non-blocking untuk
    // typewriterStart(), scrollTextStart(), dan setBacklightTimeout().
    void update();

    /* FITUR #5 (v1.1.0): Progress bar built-in (0-100%) */
    void drawProgressBar(uint8_t percent, uint8_t row);

    /* FITUR #7 (v1.1.0): printf-style formatted print
       Contoh: lcd.printFormatted("Suhu: %d C", suhu); */
    void printFormatted(const char* format, ...);

    bool isConnected(); // Fitur cek koneksi hardware

    // BARU (pilot refactor): kontrol debug runtime, selain
    // LCD_ENABLE_SERIAL_DEBUG compile-time lama (yang tetap didukung).
    void setDebug(bool debugMode) { _logger.setDebug(debugMode); }

private:
    void _command(uint8_t value);
    void _send(uint8_t value, uint8_t mode);
    void _write4bits(uint8_t value);
    void _pulseEnable(uint8_t data);
    void _expanderWrite(uint8_t data);
    void _scanAddress();
    void _printCenterImpl(const char* text, int row);
    void _typewriterBlockingImpl(const char* text, int row, int delayTime);
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

    // FITUR #6 (v1.1.0): status inisialisasi bus I2C dibuat static (per-class,
    // bukan per-instance) supaya kalau ada beberapa objek LCD (multi-LCD di
    // satu bus I2C dengan alamat berbeda), Wire.begin() cuma dipanggil SEKALI.
    // Ini mencegah re-init bus menimpa konfigurasi pin custom (SDA/SCL) dari
    // instance pertama.
    static bool _wireInitialized;

public:
    // FITUR TAMBAHAN (fix regresi v1.1.0): panggil method ini SETELAH kamu
    // memanggil Wire.begin(SDA, SCL) sendiri secara manual (mis. custom pin
    // di ESP32/ESP8266), dan SEBELUM lcd.begin(). Ini memberi tahu library
    // supaya tidak memanggil Wire.begin() versi default lagi di dalam begin(),
    // yang kalau tidak dicegah akan menimpa konfigurasi pin custom kamu.
    //
    // Contoh:
    //   Wire.begin(18, 19);
    //   LiquidCrystal_I2C::useExternalWireBegin();
    //   lcd.begin();
    static void useExternalWireBegin();
private:

    // --- State non-blocking (FITUR #1, #3, #4) ---
    String _twBuffer;          // penyimpan teks aktif untuk typewriter/scroll
    uint8_t _twIndex;
    uint8_t _twRow;
    uint16_t _twIntervalMs;
    bool _twActive;
    bool _twIsScrollMode;      // true = mode scrollText, false = mode typewriter

    unsigned long _backlightTimeoutMs;   // 0 = nonaktif

    // BARU (pilot refactor): menggantikan _twLastMillis (id 1, every()) dan
    // _lastActivityMillis (id 0, once()) dari v1.1.0 lama.
    IskakINO_Scheduler _scheduler{2};
    IskakINO_Logger _logger;

    bool _progressBarReady;
};
