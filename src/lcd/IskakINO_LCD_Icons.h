#pragma once
#include <Arduino.h>

/* =========================================================================
   IskakINO_LCD_Icons.h

   Kumpulan preset custom character (8x5 dot matrix) siap pakai untuk LCD
   HD44780 via IskakINO_LiquidCrystal_I2C.

   HD44780 hanya punya 8 slot custom character (lokasi 0-7). Slot 7 dipakai
   internal oleh fitur drawProgressBar() di IskakINO_LiquidCrystal_I2C, jadi
   HINDARI createChar(7, ...) kalau kamu juga pakai drawProgressBar().

   Cara pakai:
     #include <IskakINO_LiquidCrystal_I2C.h>
     #include <IskakINO_LCD_Icons.h>

     lcd.createChar(0, ICON_WIFI);
     lcd.createChar(1, ICON_BATTERY_FULL);
     ...
     lcd.setCursor(0, 0);
     lcd.write((uint8_t)0);  // tampilkan ikon wifi di slot 0
     lcd.write((uint8_t)1);  // tampilkan ikon baterai penuh di slot 1
========================================================================= */

// Wi-Fi
static const uint8_t ICON_WIFI[8] = {
    0b00000,
    0b01110,
    0b10001,
    0b00100,
    0b01010,
    0b00000,
    0b00100,
    0b00000
};

// Baterai penuh
static const uint8_t ICON_BATTERY_FULL[8] = {
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

// Baterai kosong
static const uint8_t ICON_BATTERY_EMPTY[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111
};

// Panah atas
static const uint8_t ICON_ARROW_UP[8] = {
    0b00100,
    0b01110,
    0b10101,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00000
};

// Panah bawah
static const uint8_t ICON_ARROW_DOWN[8] = {
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b10101,
    0b01110,
    0b00100,
    0b00000
};

// Lonceng / notifikasi
static const uint8_t ICON_BELL[8] = {
    0b00100,
    0b01110,
    0b01110,
    0b01110,
    0b11111,
    0b00000,
    0b00100,
    0b00000
};

// Hati
static const uint8_t ICON_HEART[8] = {
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000
};

// Centang / check
static const uint8_t ICON_CHECK[8] = {
    0b00000,
    0b00001,
    0b00011,
    0b10110,
    0b11100,
    0b01000,
    0b00000,
    0b00000
};

// Silang / cross
static const uint8_t ICON_CROSS[8] = {
    0b00000,
    0b10001,
    0b01010,
    0b00100,
    0b01010,
    0b10001,
    0b00000,
    0b00000
};

// Simbol derajat (°) — berguna untuk tampilan suhu
static const uint8_t ICON_DEGREE[8] = {
    0b01100,
    0b10010,
    0b10010,
    0b01100,
    0b00000,
    0b00000,
    0b00000,
    0b00000
};

// Termometer
static const uint8_t ICON_THERMOMETER[8] = {
    0b00100,
    0b01010,
    0b01010,
    0b01010,
    0b01010,
    0b10001,
    0b10001,
    0b01110
};

// Jam/waktu
static const uint8_t ICON_CLOCK[8] = {
    0b00000,
    0b01110,
    0b10101,
    0b10111,
    0b10001,
    0b01110,
    0b00000,
    0b00000
};
