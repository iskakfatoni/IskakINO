//include
#include "IskakINO_ArduFast.h"

// Constructor — _scheduler(maxTasks) menggantikan alokasi manual
// new unsigned long[]/new bool[]/new bool[] yang dulu ada langsung di sini.
// _logger di-set debug=true supaya perilaku log()/logFloat()/logf() persis
// sama seperti v1.1.0 lama (selalu mencetak ke Serial secara default).
IskakINO_ArduFast::IskakINO_ArduFast(uint8_t maxTasks)
    : _scheduler(maxTasks) {
    _logger.setDebug(true);
}

// Serial Begin
void IskakINO_ArduFast::begin(unsigned long baud) {
    Serial.begin(baud);
}

// --- Task Manager: delegasi murni ke IskakINO_Scheduler ---
bool IskakINO_ArduFast::every(unsigned long interval, uint8_t id) {
    return _scheduler.every(interval, id);
}

bool IskakINO_ArduFast::once(unsigned long delay_ms, uint8_t id) {
    return _scheduler.once(delay_ms, id);
}

void IskakINO_ArduFast::reset(uint8_t id) {
    _scheduler.reset(id);
}

void IskakINO_ArduFast::cancel(uint8_t id) {
    _scheduler.cancel(id);
}

// --- Analog: logika tidak berubah, hanya makro platform kini dari core/ ---
int IskakINO_ArduFast::readNorm(uint8_t pin) {
    int val = analogRead(pin);
#if defined(ISKAKINO_PLATFORM_ESP32)
    return val >> 2; // 12-bit ke 10-bit
#else
    return val;
#endif
}

int IskakINO_ArduFast::readStable(uint8_t pin, uint8_t samples) {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++) {
        sum += readNorm(pin);
    }
    return (int)(sum / samples);
}

int IskakINO_ArduFast::mapAnalog(uint8_t pin, int outMin, int outMax) {
    int normalized = readNorm(pin);
    return map(normalized, 0, 1023, outMin, outMax);
}

// 'state' disimpan di sisi pemanggil, library hanya melakukan perhitungannya.
float IskakINO_ArduFast::readEMA(uint8_t pin, float &state, float alpha) {
    int raw = readNorm(pin);
    state = (alpha * (float)raw) + ((1.0f - alpha) * state);
    return state;
}

// --- Logging: delegasi murni ke IskakINO_Logger ---
void IskakINO_ArduFast::log(const __FlashStringHelper* msg) {
    _logger.log(msg);
}

void IskakINO_ArduFast::log(const __FlashStringHelper* msg, long val) {
    _logger.log(msg, val);
}

void IskakINO_ArduFast::logFloat(const __FlashStringHelper* msg, float val, uint8_t decimals) {
    _logger.logFloat(msg, val, decimals);
}

void IskakINO_ArduFast::logf(const __FlashStringHelper* fmt, ...) {
    // IskakINO_Logger::logf() sendiri variadic dan tidak bisa dipanggil
    // langsung dengan meneruskan va_list dari sini (logf() memakai
    // va_start(args, fmt) yang butuh nama parameter formal terakhir persis
    // di titik pemanggilan). Jadi logika printf-style tetap di sini,
    // hanya output-nya lewat _logger untuk menghormati flag debug.
    if (!_logger.isDebug()) return;
    char buf[ISKAKINO_LOGF_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
#if defined(__AVR__)
    vsnprintf_P(buf, sizeof(buf), (const char*)fmt, args);
#else
    vsnprintf(buf, sizeof(buf), (const char*)fmt, args);
#endif
    va_end(args);
    Serial.print(F("[LOG] "));
    Serial.println(buf);
}

// --- Wrapper IO standar (untuk pin dinamis / non-template) ---
void IskakINO_ArduFast::pinMode(uint8_t pin, uint8_t mode) {
    ::pinMode(pin, mode);
}
void IskakINO_ArduFast::digitalWrite(uint8_t pin, uint8_t val) {
    ::digitalWrite(pin, val);
}
int IskakINO_ArduFast::digitalRead(uint8_t pin) {
    return ::digitalRead(pin);
}
