#include "IskakINO_RTC.h"

#if defined(ISKAKINO_HAS_WIFI)
#include "../ntp/IskakINO_FastNTP.h"
#endif

// Helper BCD Conversion
static inline uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
static inline uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }

// Nama Hari & Bulan
static const char* const DAYS_ID[] = { "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu" };
static const char* const DAYS_EN[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
static const char* const MONTHS_ID[] = { "Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember" };
static const char* const MONTHS_EN[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

static const uint8_t DAYS_PER_MONTH[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static bool isLeapYear(uint16_t year) {
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

uint8_t IskakINO_RTC::calculateDayOfWeek(uint16_t y, uint8_t m, uint8_t d) {
    // Tomohiko Sakamoto's Algorithm (0 = Minggu, 1 = Senin, ..., 6 = Sabtu)
    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
    if (m < 3) y -= 1;
    int dow = (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
    return (uint8_t)(dow + 1); // 1 = Minggu, ..., 7 = Sabtu
}

// ============================================================
// IskakDateTime Methods
// ============================================================

uint32_t IskakDateTime::toEpoch() const {
    uint32_t days = 0;
    for (uint16_t y = 1970; y < year; y++) {
        days += isLeapYear(y) ? 366 : 365;
    }
    for (uint8_t m = 1; m < month; m++) {
        days += DAYS_PER_MONTH[m - 1];
        if (m == 2 && isLeapYear(year)) days += 1;
    }
    days += (day - 1);
    return ((days * 24UL + hour) * 60UL + minute) * 60UL + second;
}

IskakDateTime IskakDateTime::fromEpoch(uint32_t epoch) {
    uint32_t rawSeconds = epoch;
    uint8_t s = rawSeconds % 60; rawSeconds /= 60;
    uint8_t min = rawSeconds % 60; rawSeconds /= 60;
    uint8_t h = rawSeconds % 24;
    uint32_t days = rawSeconds / 24;

    uint8_t dow = (uint8_t)((days + 4) % 7) + 1; // 1970-01-01 was Thursday (5)

    uint16_t y = 1970;
    while (true) {
        uint16_t daysInYear = isLeapYear(y) ? 366 : 365;
        if (days >= daysInYear) {
            days -= daysInYear;
            y++;
        } else {
            break;
        }
    }

    uint8_t m = 1;
    while (true) {
        uint8_t dim = DAYS_PER_MONTH[m - 1];
        if (m == 2 && isLeapYear(y)) dim += 1;
        if (days >= dim) {
            days -= dim;
            m++;
        } else {
            break;
        }
    }
    uint8_t d = (uint8_t)(days + 1);

    return IskakDateTime(y, m, d, h, min, s, dow);
}

const char* IskakDateTime::getDayName(bool inIndonesian) const {
    uint8_t idx = (dayOfWeek >= 1 && dayOfWeek <= 7) ? (dayOfWeek - 1) : 0;
    return inIndonesian ? DAYS_ID[idx] : DAYS_EN[idx];
}

const char* IskakDateTime::getMonthName(bool inIndonesian) const {
    uint8_t idx = (month >= 1 && month <= 12) ? (month - 1) : 0;
    return inIndonesian ? MONTHS_ID[idx] : MONTHS_EN[idx];
}

String IskakDateTime::getTimeString(bool withSeconds) const {
    char buf[10];
    if (withSeconds) {
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", hour, minute, second);
    } else {
        snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
    }
    return String(buf);
}

String IskakDateTime::getDateString(bool inIndonesian) const {
    char buf[32];
    snprintf(buf, sizeof(buf), "%s, %02d %s %04d",
             getDayName(inIndonesian), day, getMonthName(inIndonesian), year);
    return String(buf);
}

String IskakDateTime::format(const char* fmt) const {
    if (!fmt) return getTimeString();
    String out = "";
    out.reserve(32);

    for (size_t i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] == '%' && fmt[i+1] != '\0') {
            i++;
            char c = fmt[i];
            char temp[8];
            switch (c) {
                case 'Y': snprintf(temp, sizeof(temp), "%04d", year); out += temp; break;
                case 'y': snprintf(temp, sizeof(temp), "%02d", year % 100); out += temp; break;
                case 'm': snprintf(temp, sizeof(temp), "%02d", month); out += temp; break;
                case 'd': snprintf(temp, sizeof(temp), "%02d", day); out += temp; break;
                case 'H': snprintf(temp, sizeof(temp), "%02d", hour); out += temp; break;
                case 'M': snprintf(temp, sizeof(temp), "%02d", minute); out += temp; break;
                case 'S': snprintf(temp, sizeof(temp), "%02d", second); out += temp; break;
                case '%': out += '%'; break;
                default:  out += '%'; out += c; break;
            }
        } else {
            out += fmt[i];
        }
    }
    return out;
}

// ============================================================
// IskakINO_RTC Methods
// ============================================================

IskakINO_RTC::IskakINO_RTC() {
    _logger.setDebug(true);
}

bool IskakINO_RTC::readRegisters(uint8_t reg, uint8_t* buffer, size_t length) {
    _wire->beginTransmission(_i2cAddress);
    _wire->write(reg);
    if (_wire->endTransmission() != 0) {
        _lastError = IskakINO_Result::NOT_FOUND;
        return false;
    }

    size_t readCount = _wire->requestFrom((int)_i2cAddress, (int)length);
    if (readCount != length) {
        _lastError = IskakINO_Result::TIMEOUT;
        return false;
    }

    for (size_t i = 0; i < length; i++) {
        buffer[i] = _wire->read();
    }
    _lastError = IskakINO_Result::OK;
    return true;
}

bool IskakINO_RTC::writeRegisters(uint8_t reg, const uint8_t* buffer, size_t length) {
    _wire->beginTransmission(_i2cAddress);
    _wire->write(reg);
    for (size_t i = 0; i < length; i++) {
        _wire->write(buffer[i]);
    }
    if (_wire->endTransmission() != 0) {
        _lastError = IskakINO_Result::WRITE_FAILED;
        return false;
    }
    _lastError = IskakINO_Result::OK;
    return true;
}

bool IskakINO_RTC::writeRegister(uint8_t reg, uint8_t value) {
    return writeRegisters(reg, &value, 1);
}

uint8_t IskakINO_RTC::readRegister(uint8_t reg) {
    uint8_t val = 0;
    readRegisters(reg, &val, 1);
    return val;
}

bool IskakINO_RTC::detectChip() {
    // 1. Cek address 0x68 (DS3231 / DS1307)
    _wire->beginTransmission(0x68);
    if (_wire->endTransmission() == 0) {
        _i2cAddress = 0x68;
        // Bedakan DS3231 vs DS1307 via register kontrol / temperatur
        // DS3231 memiliki register temperatur di 0x11 (MSB suhu -40 .. +85 C)
        uint8_t tempMsb = 0;
        _wire->beginTransmission(0x68);
        _wire->write(0x11);
        if (_wire->endTransmission() == 0 && _wire->requestFrom(0x68, 1) == 1) {
            tempMsb = _wire->read();
        }

        // Pada DS3231 suhu wajar berada di antara -40 dan 85
        // dan bit 7 pada 0x0E (EOSC) defaultnya 0
        if ((int8_t)tempMsb >= -40 && (int8_t)tempMsb <= 85 && tempMsb != 0xFF) {
            _chipType = IskakRTCType::DS3231;
        } else {
            _chipType = IskakRTCType::DS1307;
        }
        return true;
    }

    // 2. Cek address 0x51 (PCF8563)
    _wire->beginTransmission(0x51);
    if (_wire->endTransmission() == 0) {
        _i2cAddress = 0x51;
        _chipType = IskakRTCType::PCF8563;
        return true;
    }

    return false;
}

const char* IskakINO_RTC::chipName() const {
    switch (_chipType) {
        case IskakRTCType::DS3231: return "DS3231 (TCXO High Precision)";
        case IskakRTCType::DS1307: return "DS1307 (Standard RTC)";
        case IskakRTCType::PCF8563: return "PCF8563 (Low Power RTC)";
        default: return "Unknown / Auto";
    }
}

bool IskakINO_RTC::begin(TwoWire& wire, IskakRTCType type) {
    _wire = &wire;
    _wire->begin();

    if (type == IskakRTCType::AUTO) {
        if (!detectChip()) {
            _logger.log(F("[IskakINO RTC] Gagal menemukan chip RTC pada bus I2C (0x68 / 0x51)."));
            _lastError = IskakINO_Result::NOT_FOUND;
            return false;
        }
    } else {
        _chipType = type;
        _i2cAddress = (_chipType == IskakRTCType::PCF8563) ? 0x51 : 0x68;
    }

    _logger.logf(F("[IskakINO RTC] Terdeteksi: %s pada I2C address 0x%02X"), chipName(), _i2cAddress);

    // Pastikan oscillator aktif
    if (_chipType == IskakRTCType::DS1307) {
        uint8_t sec = readRegister(0x00);
        if (sec & 0x80) { // CH bit set = oscillator berhenti
            writeRegister(0x00, sec & 0x7F); // Hapus CH bit untuk memulai
        }
    } else if (_chipType == IskakRTCType::DS3231) {
        uint8_t ctl = readRegister(0x0E);
        writeRegister(0x0E, ctl & ~0x80); // Clear EOSC bit
    } else if (_chipType == IskakRTCType::PCF8563) {
        writeRegister(0x00, 0x00); // Clear STOP bit
    }

    _lastError = IskakINO_Result::OK;
    return true;
}

bool IskakINO_RTC::isRunning() {
    if (_chipType == IskakRTCType::DS1307) {
        return !(readRegister(0x00) & 0x80);
    } else if (_chipType == IskakRTCType::DS3231) {
        return !(readRegister(0x0E) & 0x80);
    } else if (_chipType == IskakRTCType::PCF8563) {
        return !(readRegister(0x00) & 0x20);
    }
    return true;
}

bool IskakINO_RTC::lostPower() {
    if (_chipType == IskakRTCType::DS3231) {
        return (readRegister(0x0F) & 0x80) != 0; // OSF flag
    } else if (_chipType == IskakRTCType::PCF8563) {
        return (readRegister(0x02) & 0x80) != 0; // VL bit
    } else if (_chipType == IskakRTCType::DS1307) {
        return (readRegister(0x00) & 0x80) != 0; // CH bit
    }
    return false;
}

IskakDateTime IskakINO_RTC::now() {
    uint8_t buf[7];

    if (_chipType == IskakRTCType::PCF8563) {
        if (!readRegisters(0x02, buf, 7)) {
            return IskakDateTime();
        }
        uint8_t s = bcd2bin(buf[0] & 0x7F);
        uint8_t min = bcd2bin(buf[1] & 0x7F);
        uint8_t h = bcd2bin(buf[2] & 0x3F);
        uint8_t d = bcd2bin(buf[3] & 0x3F);
        uint8_t dow = (buf[4] & 0x07) + 1; // PCF8563 0=Minggu -> 1=Minggu
        uint8_t m = bcd2bin(buf[5] & 0x1F);
        uint16_t y = 2000 + bcd2bin(buf[6]);
        return IskakDateTime(y, m, d, h, min, s, dow);
    } else {
        // DS3231 / DS1307
        if (!readRegisters(0x00, buf, 7)) {
            return IskakDateTime();
        }
        uint8_t s = bcd2bin(buf[0] & 0x7F);
        uint8_t min = bcd2bin(buf[1] & 0x7F);
        uint8_t h = bcd2bin(buf[2] & 0x3F);
        uint8_t dow = buf[3] & 0x07;
        uint8_t d = bcd2bin(buf[4] & 0x3F);
        uint8_t m = bcd2bin(buf[5] & 0x1F);
        uint16_t y = 2000 + bcd2bin(buf[6]);
        return IskakDateTime(y, m, d, h, min, s, dow);
    }
}

void IskakINO_RTC::setDateTime(const IskakDateTime& dt) {
    setDateTime(dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
}

void IskakINO_RTC::setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    uint8_t dow = calculateDayOfWeek(year, month, day);

    if (_chipType == IskakRTCType::PCF8563) {
        uint8_t buf[7];
        buf[0] = bin2bcd(second) & 0x7F; // clear VL
        buf[1] = bin2bcd(minute) & 0x7F;
        buf[2] = bin2bcd(hour) & 0x3F;
        buf[3] = bin2bcd(day) & 0x3F;
        buf[4] = (dow - 1) & 0x07;
        buf[5] = bin2bcd(month) & 0x1F;
        buf[6] = bin2bcd((uint8_t)(year % 100));
        writeRegisters(0x02, buf, 7);
    } else {
        // DS3231 / DS1307
        uint8_t buf[7];
        buf[0] = bin2bcd(second) & 0x7F;
        buf[1] = bin2bcd(minute) & 0x7F;
        buf[2] = bin2bcd(hour) & 0x3F;
        buf[3] = dow & 0x07;
        buf[4] = bin2bcd(day) & 0x3F;
        buf[5] = bin2bcd(month) & 0x1F;
        buf[6] = bin2bcd((uint8_t)(year % 100));
        writeRegisters(0x00, buf, 7);

        // Hapus flag OSF di DS3231 jika ada
        if (_chipType == IskakRTCType::DS3231) {
            uint8_t stat = readRegister(0x0F);
            writeRegister(0x0F, stat & ~0x80);
        }
    }
}

void IskakINO_RTC::setEpoch(uint32_t epoch) {
    IskakDateTime dt = IskakDateTime::fromEpoch(epoch);
    setDateTime(dt);
}

uint32_t IskakINO_RTC::getEpoch() {
    return now().toEpoch();
}

float IskakINO_RTC::getTemperature() {
    if (_chipType != IskakRTCType::DS3231) {
        return 0.0f;
    }
    uint8_t buf[2];
    if (!readRegisters(0x11, buf, 2)) {
        return 0.0f;
    }
    int8_t msb = (int8_t)buf[0];
    float lsb = (float)(buf[1] >> 6) * 0.25f;
    return (msb >= 0) ? (msb + lsb) : (msb - lsb);
}

#if defined(ISKAKINO_HAS_WIFI)
void IskakINO_RTC::syncWithNTP(IskakINO_FastNTP& ntp, uint32_t intervalMs) {
    _ntpSyncSource = &ntp;
    _ntpSyncIntervalMs = intervalMs;
}
#endif

void IskakINO_RTC::tick() {
    #if defined(ISKAKINO_HAS_WIFI)
    if (_ntpSyncSource && _ntpSyncSource->isSynced()) {
        if (_scheduler.every(_ntpSyncIntervalMs, 0)) {
            uint32_t epoch = _ntpSyncSource->getEpoch();
            if (epoch > 100000) {
                setEpoch(epoch);
                _logger.logf(F("[IskakINO RTC] Sinkronisasi otomatis dari FastNTP berhasil -> Epoch: %lu"), epoch);
            }
        }
    }
    #endif
}
