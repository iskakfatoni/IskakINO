/*
 * src/shield/IskakINO_BasicIOShield.h
 * Modul driver hardware untuk modul pembelajaran EMS Basic I/O Shield
 * (Innovative Electronics - Training Division).
 *
 * KHUSUS ARDUINO AVR (Arduino Uno, Nano, Mega, Duemilanove, dll.):
 * Modul ini secara otomatis aktif hanya pada mikrokontroler berbasis AVR.
 * Pada board non-AVR (ESP32/ESP8266), header ini aman di-include dan
 * menghasilkan no-op tanpa konflik pin / kompilasi.
 *
 * Fitur:
 *   - 4x LED Digital Output (Red, Yellow, Blue, Green)
 *   - 2x Push Button Digital Input (Button 1, Button 2)
 *   - 1x Potensiometer Analog Input (Pin A1)
 *   - Dual-Digit 7-Segment Multiplexing Display (Non-blocking & Legacy)
 *   - 10-Bit I2C Digital-to-Analog Converter (IC AD5612 di alamat 0x0E)
 */

#ifndef ISKAKINO_BASIC_IO_SHIELD_H
#define ISKAKINO_BASIC_IO_SHIELD_H

#include <Arduino.h>
#include <Wire.h>
#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_PLATFORM_AVR)

#define POTENTIOMETER_PIN     A1
#define GREEN_PIN             6
#define BLUE_PIN              7
#define YELLOW_PIN            8
#define RED_PIN               9
#define BUTTON_1_PIN          2
#define BUTTON_2_PIN          4
#define SEVEN_SEGMENT_1_PIN   A2
#define SEVEN_SEGMENT_2_PIN   A3
#define SEGMENT_A_PIN         6
#define SEGMENT_B_PIN         7
#define SEGMENT_C_PIN         8
#define SEGMENT_D_PIN         9
#define SEGMENT_E_PIN         10
#define SEGMENT_F_PIN         11
#define SEGMENT_G_PIN         12
#define SEGMENT_DP_PIN        13

#define GREEN_ARRAY           0
#define BLUE_ARRAY            1
#define YELLOW_ARRAY          2
#define RED_ARRAY             3
#define BUTTON_1_ARRAY        0
#define BUTTON_2_ARRAY        1
#define SEGMENT_A_ARRAY       0
#define SEGMENT_B_ARRAY       1
#define SEGMENT_C_ARRAY       2
#define SEGMENT_D_ARRAY       3
#define SEGMENT_E_ARRAY       4
#define SEGMENT_F_ARRAY       5
#define SEGMENT_G_ARRAY       6
#define SEGMENT_DP_ARRAY      7
#define SEVEN_SEGMENT_1_ARRAY 0
#define SEVEN_SEGMENT_2_ARRAY 1

#define AD5612Address         0x0E
#define AD5612_I2C_ADDRESS    0x0E

class IskakINO_BasicIOShield
{
  public:
    IskakINO_BasicIOShield();
    void begin();

    // Analog & Digital Inputs
    uint16_t ReadPotentiometer();
    bool Button1State();
    bool Button2State();

    // LED Controls
    void RedOn();
    void RedOff();
    void GreenOn();
    void GreenOff();
    void BlueOn();
    void BlueOff();
    void YellowOn();
    void YellowOff();

    // 7-Segment Controls (Non-Blocking / Asynchronous - Disarankan)
    void setDisplay(uint8_t number);
    void clearDisplay();
    void update();
    void setRefreshInterval(uint16_t intervalMicros = 3000);

    // 7-Segment Controls (Blocking - Legacy)
    void PrintSevenSegment(uint8_t number, uint8_t times = 10, uint8_t delayTime = 5);

    // I2C 10-Bit DAC (AD5612)
    bool WriteDAC(uint16_t value);

  private:
    uint8_t _LEDsPin[4];
    uint8_t _ButtonsPin[2];
    uint8_t _SegmentsPin[8];
    uint8_t _SevenSegmentsPin[2];

    uint8_t _displayNumber;
    bool _displayEnabled;
    uint8_t _activeDigit;
    uint32_t _lastDigitSwitchMicros;
    uint16_t _refreshIntervalMicros;

    void SevenSegment(uint8_t number);
    void SegmentsPinWrite(uint8_t pattern);
};

// Alias backward-compatibility dengan library asli
typedef IskakINO_BasicIOShield BasicIOShield;

#endif // ISKAKINO_PLATFORM_AVR

#endif // ISKAKINO_BASIC_IO_SHIELD_H
