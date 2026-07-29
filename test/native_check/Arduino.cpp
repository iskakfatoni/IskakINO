// test/mock/Arduino.cpp — definisi variabel global untuk mock Arduino.h
#include "Arduino.h"

unsigned long _mock_millis_value = 0;
int _mock_analog_value = 512;
MockSerial Serial;
