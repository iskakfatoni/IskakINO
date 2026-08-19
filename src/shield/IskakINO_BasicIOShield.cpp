/*
 * src/shield/IskakINO_BasicIOShield.cpp
 * Implementasi modul driver EMS Basic I/O Shield untuk ekosistem IskakINO.
 */

#include "IskakINO_BasicIOShield.h"

#if defined(ISKAKINO_PLATFORM_AVR)

// 7-segment bitmask patterns untuk digit 0-9 (A=bit0, B=bit1, C=bit2, D=bit3, E=bit4, F=bit5, G=bit6, DP=bit7)
static const uint8_t SEVEN_SEG_PATTERNS[10] = {
    0x3F, // 0: A B C D E F
    0x06, // 1: B C
    0x5B, // 2: A B D E G
    0x4F, // 3: A B C D G
    0x66, // 4: B C F G
    0x6D, // 5: A C D F G
    0x7D, // 6: A C D E F G
    0x07, // 7: A B C
    0x7F, // 8: A B C D E F G
    0x6F  // 9: A B C D F G
};

IskakINO_BasicIOShield::IskakINO_BasicIOShield()
{
    _LEDsPin[GREEN_ARRAY] = GREEN_PIN;
    _LEDsPin[BLUE_ARRAY] = BLUE_PIN;
    _LEDsPin[YELLOW_ARRAY] = YELLOW_PIN;
    _LEDsPin[RED_ARRAY] = RED_PIN;

    _ButtonsPin[BUTTON_1_ARRAY] = BUTTON_1_PIN;
    _ButtonsPin[BUTTON_2_ARRAY] = BUTTON_2_PIN;

    _SegmentsPin[SEGMENT_A_ARRAY] = SEGMENT_A_PIN;
    _SegmentsPin[SEGMENT_B_ARRAY] = SEGMENT_B_PIN;
    _SegmentsPin[SEGMENT_C_ARRAY] = SEGMENT_C_PIN;
    _SegmentsPin[SEGMENT_D_ARRAY] = SEGMENT_D_PIN;
    _SegmentsPin[SEGMENT_E_ARRAY] = SEGMENT_E_PIN;
    _SegmentsPin[SEGMENT_F_ARRAY] = SEGMENT_F_PIN;
    _SegmentsPin[SEGMENT_G_ARRAY] = SEGMENT_G_PIN;
    _SegmentsPin[SEGMENT_DP_ARRAY] = SEGMENT_DP_PIN;

    _SevenSegmentsPin[SEVEN_SEGMENT_1_ARRAY] = SEVEN_SEGMENT_1_PIN;
    _SevenSegmentsPin[SEVEN_SEGMENT_2_ARRAY] = SEVEN_SEGMENT_2_PIN;

    _displayNumber = 0;
    _displayEnabled = false;
    _activeDigit = 0;
    _lastDigitSwitchMicros = 0;
    _refreshIntervalMicros = 3000;
}

void IskakINO_BasicIOShield::begin()
{
    for(int i = 0; i < 4; i++)
        pinMode(_LEDsPin[i], OUTPUT);
    for(int i = 0; i < 2; i++)
        pinMode(_ButtonsPin[i], INPUT);
    for(int i = 0; i < 8; i++)
        pinMode(_SegmentsPin[i], OUTPUT);
    for(int i = 0; i < 2; i++)
        pinMode(_SevenSegmentsPin[i], OUTPUT);

    clearDisplay();
    Wire.begin();
}

uint16_t IskakINO_BasicIOShield::ReadPotentiometer()
{
    return analogRead(POTENTIOMETER_PIN);
}

void IskakINO_BasicIOShield::RedOn()
{
    digitalWrite(_LEDsPin[RED_ARRAY], HIGH);
}

void IskakINO_BasicIOShield::RedOff()
{
    digitalWrite(_LEDsPin[RED_ARRAY], LOW);
}

void IskakINO_BasicIOShield::GreenOn()
{
    digitalWrite(_LEDsPin[GREEN_ARRAY], HIGH);
}

void IskakINO_BasicIOShield::GreenOff()
{
    digitalWrite(_LEDsPin[GREEN_ARRAY], LOW);
}

void IskakINO_BasicIOShield::BlueOn()
{
    digitalWrite(_LEDsPin[BLUE_ARRAY], HIGH);
}

void IskakINO_BasicIOShield::BlueOff()
{
    digitalWrite(_LEDsPin[BLUE_ARRAY], LOW);
}

void IskakINO_BasicIOShield::YellowOn()
{
    digitalWrite(_LEDsPin[YELLOW_ARRAY], HIGH);
}

void IskakINO_BasicIOShield::YellowOff()
{
    digitalWrite(_LEDsPin[YELLOW_ARRAY], LOW);
}

bool IskakINO_BasicIOShield::Button1State()
{
    return digitalRead(_ButtonsPin[BUTTON_1_ARRAY]);
}

bool IskakINO_BasicIOShield::Button2State()
{
    return digitalRead(_ButtonsPin[BUTTON_2_ARRAY]);
}

void IskakINO_BasicIOShield::SegmentsPinWrite(uint8_t pattern)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        digitalWrite(_SegmentsPin[i], (pattern >> i) & 0x01);
    }
}

void IskakINO_BasicIOShield::SevenSegment(uint8_t number)
{
    if(number <= 9)
    {
        SegmentsPinWrite(SEVEN_SEG_PATTERNS[number]);
    }
    else
    {
        SegmentsPinWrite(0x00);
    }
}

void IskakINO_BasicIOShield::PrintSevenSegment(uint8_t number, uint8_t times, uint8_t delayTime)
{
    uint8_t puluhan = number / 10;
    uint8_t satuan = number % 10;

    for(uint16_t loopTimes = 0; loopTimes < times; loopTimes++)
    {
        digitalWrite(SEVEN_SEGMENT_1_PIN, LOW);
        digitalWrite(SEVEN_SEGMENT_2_PIN, LOW);
        SevenSegment(puluhan);
        digitalWrite(SEVEN_SEGMENT_1_PIN, LOW);
        digitalWrite(SEVEN_SEGMENT_2_PIN, HIGH);
        delay(delayTime);

        digitalWrite(SEVEN_SEGMENT_1_PIN, LOW);
        digitalWrite(SEVEN_SEGMENT_2_PIN, LOW);
        SevenSegment(satuan);
        digitalWrite(SEVEN_SEGMENT_1_PIN, HIGH);
        digitalWrite(SEVEN_SEGMENT_2_PIN, LOW);
        delay(delayTime);
    }
}

void IskakINO_BasicIOShield::setDisplay(uint8_t number)
{
    _displayNumber = (number > 99) ? 99 : number;
    _displayEnabled = true;
}

void IskakINO_BasicIOShield::clearDisplay()
{
    _displayEnabled = false;
    digitalWrite(_SevenSegmentsPin[SEVEN_SEGMENT_1_ARRAY], LOW);
    digitalWrite(_SevenSegmentsPin[SEVEN_SEGMENT_2_ARRAY], LOW);
    SegmentsPinWrite(0x00);
}

void IskakINO_BasicIOShield::setRefreshInterval(uint16_t intervalMicros)
{
    _refreshIntervalMicros = (intervalMicros > 0) ? intervalMicros : 3000;
}

void IskakINO_BasicIOShield::update()
{
    if (!_displayEnabled) return;

    uint32_t currentMicros = micros();
    if (currentMicros - _lastDigitSwitchMicros >= _refreshIntervalMicros)
    {
        _lastDigitSwitchMicros = currentMicros;

        // Matikan kedua transistor digit terlebih dahulu untuk mencegah bayangan / ghosting
        digitalWrite(_SevenSegmentsPin[SEVEN_SEGMENT_1_ARRAY], LOW);
        digitalWrite(_SevenSegmentsPin[SEVEN_SEGMENT_2_ARRAY], LOW);

        if (_activeDigit == 0)
        {
            // Peragakan digit puluhan
            uint8_t puluhan = _displayNumber / 10;
            SevenSegment(puluhan);
            digitalWrite(_SevenSegmentsPin[SEVEN_SEGMENT_1_ARRAY], LOW);
            digitalWrite(_SevenSegmentsPin[SEVEN_SEGMENT_2_ARRAY], HIGH);
            _activeDigit = 1;
        }
        else
        {
            // Peragakan digit satuan
            uint8_t satuan = _displayNumber % 10;
            SevenSegment(satuan);
            digitalWrite(_SevenSegmentsPin[SEVEN_SEGMENT_1_ARRAY], HIGH);
            digitalWrite(_SevenSegmentsPin[SEVEN_SEGMENT_2_ARRAY], LOW);
            _activeDigit = 0;
        }
    }
}

bool IskakINO_BasicIOShield::WriteDAC(uint16_t value)
{
    if (value > 1023) value = 1023; // AD5612 adalah 10-bit DAC (0 - 1023)

    uint8_t highByte = (value >> 4) & 0x3F; // PD1=0, PD0=0, D9..D4
    uint8_t lowByte  = (value << 4) & 0xF0; // D3..D0, x, x, x, x

    Wire.beginTransmission(AD5612Address);
    Wire.write(highByte);
    Wire.write(lowByte);
    return (Wire.endTransmission() == 0);
}

#endif // ISKAKINO_PLATFORM_AVR
