#pragma once
#include <Arduino.h>

/* =========================================================================
   IskakINO_LCD_Icons.h

   Kumpulan preset custom character (8x5 dot matrix) siap pakai & generator
   ikon dinamis untuk LCD HD44780 via IskakINO_LiquidCrystal_I2C.

   HD44780 hanya punya 8 slot custom character (lokasi 0-7). Slot 7 dipakai
   internal oleh fitur drawProgressBar() di IskakINO_LiquidCrystal_I2C, jadi
   HINDARI createChar(7, ...) kalau kamu juga pakai drawProgressBar().

   Cara pakai preset:
     #include <IskakINO.h>

     lcd.createChar(0, ICON_WIFI);
     lcd.createChar(1, ICON_BATTERY_FULL);
     ...
     lcd.setCursor(0, 0);
     lcd.write((uint8_t)0);  // tampilkan ikon wifi di slot 0
     lcd.write((uint8_t)1);  // tampilkan ikon baterai penuh di slot 1

   Cara pakai generator dinamis (via IskakINO_LiquidCrystal_I2C):
     lcd.drawBattery(15, 0, 75);         // baterai 75% di col 15, row 0
     lcd.drawWifiSignalRssi(14, 0, -65); // sinyal wifi dari RSSI di col 14, row 0
     lcd.drawThermometer(0, 1, 2);       // termometer level 2 di col 0, row 1
========================================================================= */

// Wi-Fi (Antena lengkung)
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

// Wi-Fi Bar (0 sampai 4 Bar bertingkat)
static const uint8_t ICON_WIFI_BAR_0[8] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b10000
};

static const uint8_t ICON_WIFI_BAR_1[8] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b01000,
    0b01000
};

static const uint8_t ICON_WIFI_BAR_2[8] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00100,
    0b00100,
    0b01100,
    0b01100
};

static const uint8_t ICON_WIFI_BAR_3[8] = {
    0b00000,
    0b00000,
    0b00010,
    0b00010,
    0b00110,
    0b00110,
    0b01110,
    0b01110
};

static const uint8_t ICON_WIFI_BAR_4[8] = {
    0b00001,
    0b00001,
    0b00011,
    0b00011,
    0b00111,
    0b00111,
    0b01111,
    0b01111
};

// Baterai bertingkat (0% - 100%)
static const uint8_t ICON_BATTERY_0[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111
};

static const uint8_t ICON_BATTERY_20[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
    0b11111
};

static const uint8_t ICON_BATTERY_40[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b10001,
    0b11111,
    0b11111,
    0b11111
};

static const uint8_t ICON_BATTERY_60[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b10001,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

static const uint8_t ICON_BATTERY_80[8] = {
    0b01110,
    0b11111,
    0b10001,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

static const uint8_t ICON_BATTERY_100[8] = {
    0b01110,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111,
    0b11111
};

// Alias untuk kompatibilitas ke belakang
static const uint8_t ICON_BATTERY_FULL[8]  = { 0b01110, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };
static const uint8_t ICON_BATTERY_EMPTY[8] = { 0b01110, 0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11111 };

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

// Panah kiri
static const uint8_t ICON_ARROW_LEFT[8] = {
    0b00010,
    0b00110,
    0b01110,
    0b11110,
    0b01110,
    0b00110,
    0b00010,
    0b00000
};

// Panah kanan
static const uint8_t ICON_ARROW_RIGHT[8] = {
    0b01000,
    0b01100,
    0b01110,
    0b01111,
    0b01110,
    0b01100,
    0b01000,
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

// Termometer bertingkat
static const uint8_t ICON_THERMO_0[8] = {
    0b00100,
    0b01010,
    0b01010,
    0b01010,
    0b01010,
    0b10001,
    0b11111,
    0b01110
};

static const uint8_t ICON_THERMO_1[8] = {
    0b00100,
    0b01010,
    0b01010,
    0b01010,
    0b01110,
    0b10001,
    0b11111,
    0b01110
};

static const uint8_t ICON_THERMO_2[8] = {
    0b00100,
    0b01010,
    0b01010,
    0b01110,
    0b01110,
    0b10001,
    0b11111,
    0b01110
};

static const uint8_t ICON_THERMO_3[8] = {
    0b00100,
    0b01010,
    0b01110,
    0b01110,
    0b01110,
    0b10001,
    0b11111,
    0b01110
};

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

/* =========================================================================
   Generator Fungsi Mandiri (Dapat dipakai tanpa LCD atau dengan LCD kustom)
========================================================================= */

/**
 * Menghasilkan bitmap 8-byte untuk indikator baterai berdasarkan persentase (0-100%).
 */
inline void generateBatteryIcon(uint8_t percent, uint8_t outMap[8]) {
    if (percent > 100) percent = 100;
    outMap[0] = 0b01110; // Terminal atas
    outMap[1] = 0b11111; // Batas atas
    outMap[7] = 0b11111; // Batas bawah

    // 5 tingkat pengisian internal (baris 2..6)
    uint8_t bars = (percent == 0) ? 0 : ((uint16_t)percent + 19) / 20;
    if (bars > 5) bars = 5;

    for (uint8_t r = 0; r < 5; r++) {
        // r=4 adalah baris terbawah (row 6)
        if ((4 - r) < bars) {
            outMap[2 + r] = 0b11111;
        } else {
            outMap[2 + r] = 0b10001;
        }
    }
}

/**
 * Menghasilkan bitmap 8-byte untuk bar sinyal Wi-Fi (0-4 bar).
 */
inline void generateWifiIcon(uint8_t level, uint8_t outMap[8]) {
    if (level > 4) level = 4;
    for (uint8_t r = 0; r < 8; r++) {
        uint8_t rowBits = 0;
        if (level >= 1 && r >= 6) rowBits |= 0b01000;
        if (level >= 2 && r >= 4) rowBits |= 0b00100;
        if (level >= 3 && r >= 2) rowBits |= 0b00010;
        if (level >= 4 && r >= 0) rowBits |= 0b00001;

        if (level == 0 && r == 7) rowBits = 0b10000;

        outMap[r] = rowBits;
    }
}

/**
 * Mengonversi nilai RSSI Wi-Fi (dBm) menjadi tingkat bar 0-4.
 */
inline uint8_t rssiToWifiBars(int rssi) {
    if (rssi <= -100 || rssi == 0) return 0;
    if (rssi >= -55) return 4;
    if (rssi >= -67) return 3;
    if (rssi >= -78) return 2;
    if (rssi >= -88) return 1;
    return 0;
}

/**
 * Menghasilkan bitmap 8-byte untuk termometer (level 0-3).
 */
inline void generateThermometerIcon(uint8_t level, uint8_t outMap[8]) {
    if (level > 3) level = 3;
    outMap[0] = 0b00100; // stem top
    outMap[1] = 0b01010; // stem upper
    outMap[2] = (level >= 3) ? 0b01110 : 0b01010; // high level
    outMap[3] = (level >= 2) ? 0b01110 : 0b01010; // med level
    outMap[4] = (level >= 1) ? 0b01110 : 0b01010; // low level
    outMap[5] = 0b10001; // bulb top
    outMap[6] = 0b11111; // bulb full
    outMap[7] = 0b01110; // bulb bottom
}
