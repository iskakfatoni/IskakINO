#include "IskakINO_FastNTP.h"

// Seluruh isi file ini otomatis kosong (tidak ada kode ter-generate, tidak
// error) di board non-WiFi — lihat komentar di IskakINO_FastNTP.h.
#if defined(ISKAKINO_HAS_WIFI)

/**
 * Konstruktor single-server (backward compatible dengan v1.0.x).
 */
IskakINO_FastNTP::IskakINO_FastNTP(UDP& udp, const char* server) {
    _initCommon(udp);
    _serverList[0] = server;
    _serverCount = 1;
    _currentServerIdx = 0;
    _ntpServer = _serverList[0];
}

/**
 * Konstruktor multi-server (v1.1.0).
 */
IskakINO_FastNTP::IskakINO_FastNTP(UDP& udp, const char* const* serverList, uint8_t count) {
    _initCommon(udp);

    if (count == 0) count = 1;                                   // guard: minimal 1 server
    if (count > ISKAKINO_FASTNTP_MAX_SERVERS) count = ISKAKINO_FASTNTP_MAX_SERVERS; // cap

    for (uint8_t i = 0; i < count; i++) {
        _serverList[i] = serverList[i];
    }
    _serverCount = count;
    _currentServerIdx = 0;
    _ntpServer = _serverList[0];
}

/**
 * Inisialisasi field yang sama untuk semua konstruktor.
 */
void IskakINO_FastNTP::_initCommon(UDP& udp) {
    _udp = &udp;

    _syncInterval = 3600000;       // Default 1 jam
    _baseSyncInterval = 3600000;
    _requestTimeoutMs = 2000;      // Default 2 detik

    _currentEpoch = 0;
    _lastSyncMs = 0;
    _requestMs = 0;
    _lastUpdateTick = 0;
    _bootTimestamp = 0;
    _gmtOffsetSec = 0;
    _daylightOffsetSec = 0;

    _serverCount = 1;
    _currentServerIdx = 0;
    _consecutiveFails = 0;
    _failThreshold = 3;

    _syncSuccessCount = 0;
    _syncFailCount = 0;

    _onSyncCb = nullptr;
    _onFailCb = nullptr;

    _state = STATE_IDLE;
}

/**
 * begin: Inisialisasi awal.
 */
void IskakINO_FastNTP::begin(long gmtOffset, int daylightOffset) {
    _gmtOffsetSec = gmtOffset;
    _daylightOffsetSec = daylightOffset;
    _udp->begin(123);
}

/**
 * Pindah ke server berikutnya dalam list (round-robin). No-op jika cuma 1 server.
 */
void IskakINO_FastNTP::_rotateServer() {
    if (_serverCount <= 1) return;
    _currentServerIdx = (_currentServerIdx + 1) % _serverCount;
    _ntpServer = _serverList[_currentServerIdx];
    _logger.log(F("[FastNTP] Pindah ke server NTP berikutnya."));
}

/**
 * update: Jantung dari Library (State Machine).
 */
void IskakINO_FastNTP::update() {
    // 1. CLOCK SIMULATION
    if (_currentEpoch > 0) {
        if (millis() - _lastUpdateTick >= 1000) {
            uint32_t diff = (millis() - _lastUpdateTick) / 1000;
            _currentEpoch += diff;
            _lastUpdateTick += diff * 1000;
        }
    }

    // 2. STATE MACHINE
    switch (_state) {
        case STATE_IDLE:
            if (millis() - _lastSyncMs >= _syncInterval || _lastSyncMs == 0) {
                _state = STATE_SEND_REQUEST;
            }
            break;

        case STATE_SEND_REQUEST:
            sendNTPPacket();
            _requestMs = millis();
            _state = STATE_AWAIT_RESPONSE;
            break;

        case STATE_AWAIT_RESPONSE: {
            int packetSize = _udp->parsePacket();

            if (packetSize >= 48) {
                _udp->read(_packetBuffer, 48);

                // --- Deteksi Kiss-of-Death (v1.1.0) ---
                // Stratum == 0 berarti server menolak melayani & minta kita mundur
                // (RFC 5905). Ini BUKAN kegagalan jaringan biasa, jadi ditangani
                // beda dari timeout: backoff eksponensial + rotate server langsung.
                if (_packetBuffer[1] == 0) {
                    _consecutiveFails++;
                    _syncFailCount++;
                    if (_onFailCb) _onFailCb(_consecutiveFails);
                    _logger.log(F("[FastNTP] Kiss-of-Death dari server, backoff + rotate."));

                    // Backoff eksponensial dengan cap; pakai uint64 supaya tidak overflow.
                    uint64_t backoff64 = (uint64_t)_syncInterval * 2;
                    _syncInterval = (backoff64 > ISKAKINO_FASTNTP_MAX_SYNC_INTERVAL_MS)
                                        ? (uint32_t)ISKAKINO_FASTNTP_MAX_SYNC_INTERVAL_MS
                                        : (uint32_t)backoff64;

                    _rotateServer();
                    _consecutiveFails = 0;

                    _lastSyncMs = millis();
                    _state = STATE_IDLE;
                    break;
                }

                uint32_t highWord = word(_packetBuffer[40], _packetBuffer[41]);
                uint32_t lowWord = word(_packetBuffer[42], _packetBuffer[43]);
                uint32_t secsSince1900 = highWord << 16 | lowWord;

                const uint32_t seventyYears = 2208988800UL;
                uint32_t utcEpoch = secsSince1900 - seventyYears;

                _currentEpoch = utcEpoch + _gmtOffsetSec + _daylightOffsetSec;
                _lastSyncMs = millis();
                _lastUpdateTick = millis();

                if (_bootTimestamp == 0) _bootTimestamp = _currentEpoch - (millis() / 1000);

                // Sync sukses -> reset semua state kegagalan & backoff
                _consecutiveFails = 0;
                _syncInterval = _baseSyncInterval;
                _syncSuccessCount++;

                if (_onSyncCb) _onSyncCb(utcEpoch);
                _logger.log(F("[FastNTP] Sinkronisasi berhasil."));

                _state = STATE_IDLE;
            }
            else if (packetSize > 0) {
                // Paket datang tapi ukurannya tidak valid untuk NTP (< 48 byte).
                // Buang, tetap tunggu paket berikutnya sampai timeout tercapai.
            }
            else if (millis() - _requestMs > _requestTimeoutMs) {
                _consecutiveFails++;
                _syncFailCount++;
                if (_onFailCb) _onFailCb(_consecutiveFails);
                _logger.log(F("[FastNTP] Request timeout."));

                if (_consecutiveFails >= _failThreshold) {
                    _rotateServer();
                    _consecutiveFails = 0;
                }

                _state = STATE_IDLE;
                // Retry lebih cepat (~15 detik lagi) daripada menunggu interval penuh,
                // dengan guard supaya tidak underflow kalau _syncInterval di-set sangat kecil.
                _lastSyncMs = (_syncInterval > 15000) ? (millis() - (_syncInterval - 15000)) : millis();
            }
            break;
        }
    }
}

void IskakINO_FastNTP::sendNTPPacket() {
    memset(_packetBuffer, 0, 48);
    _packetBuffer[0] = 0b11100011;
    _udp->beginPacket(_ntpServer, 123);
    _udp->write(_packetBuffer, 48);
    _udp->endPacket();
}

/**
 * getMillisSinceLastSync: Menghitung selisih waktu sejak sinkronisasi sukses terakhir.
 */
uint32_t IskakINO_FastNTP::getMillisSinceLastSync() {
    if (_lastSyncMs == 0) return 0;
    return (uint32_t)(millis() - _lastSyncMs);
}

// --- Getters ---
uint32_t IskakINO_FastNTP::getEpoch() { return _currentEpoch; }

uint32_t IskakINO_FastNTP::getUtcEpoch() {
    // _currentEpoch = utcEpoch + gmtOffset + daylightOffset, jadi tinggal dikurangi balik.
    return _currentEpoch - _gmtOffsetSec - _daylightOffsetSec;
}

int IskakINO_FastNTP::getSeconds()    { return _currentEpoch % 60; }
int IskakINO_FastNTP::getMinutes()    { return (_currentEpoch % 3600) / 60; }
int IskakINO_FastNTP::getHours()      { return (_currentEpoch % 86400L) / 3600; }

int IskakINO_FastNTP::getDay() {
    time_t t = (time_t)_currentEpoch;
    tm *ptm = gmtime(&t);
    return ptm->tm_mday;
}

int IskakINO_FastNTP::getMonth() {
    time_t t = (time_t)_currentEpoch;
    tm *ptm = gmtime(&t);
    return ptm->tm_mon + 1;
}

int IskakINO_FastNTP::getYear() {
    time_t t = (time_t)_currentEpoch;
    tm *ptm = gmtime(&t);
    return ptm->tm_year + 1900;
}

String IskakINO_FastNTP::getDayName(NTP_Language lang) {
    int day = ((_currentEpoch / 86400L) + 4) % 7;
    if (lang == LANG_ID) {
        const char* daysID[] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu"};
        return daysID[day];
    }
    const char* daysEN[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return daysEN[day];
}

String IskakINO_FastNTP::getMonthName(NTP_Language lang) {
    int mon = getMonth() - 1;
    if (lang == LANG_ID) {
        const char* monID[] = {"Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};
        return monID[mon];
    }
    const char* monEN[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    return monEN[mon];
}

String IskakINO_FastNTP::getFormattedTime() {
    char buf[10];
    sprintf(buf, "%02d:%02d:%02d", getHours(), getMinutes(), getSeconds());
    return String(buf);
}

String IskakINO_FastNTP::getFormattedDate(char separator) {
    char buf[16];
    sprintf(buf, "%02d%c%02d%c%d", getDay(), separator, getMonth(), separator, getYear());
    return String(buf);
}

bool IskakINO_FastNTP::isAlarmActive(int hr, int min, int sec) {
    return (getHours() == hr && getMinutes() == min && getSeconds() == sec);
}

bool IskakINO_FastNTP::isAlarmActive(int hr, int min, int sec, bool &firedFlag) {
    bool match = isAlarmActive(hr, min, sec);

    if (match && !firedFlag) {
        firedFlag = true;
        return true;
    }

    if (!match) {
        firedFlag = false;
    }

    return false;
}

bool IskakINO_FastNTP::isTimeReliable(uint32_t maxAgeSeconds) {
    if (_currentEpoch == 0) return false;

    uint32_t elapsedMs = millis() - _lastSyncMs;

    uint64_t maxAgeMs64 = (uint64_t)maxAgeSeconds * 1000ULL;
    uint32_t maxAgeMs = (maxAgeMs64 > 0xFFFFFFFFULL) ? 0xFFFFFFFFUL : (uint32_t)maxAgeMs64;

    return elapsedMs < maxAgeMs;
}

uint32_t IskakINO_FastNTP::getUptimeSeconds() {
    if (_bootTimestamp == 0) return millis() / 1000;
    return _currentEpoch - _bootTimestamp;
}

void IskakINO_FastNTP::setEpoch(uint32_t manualEpoch) {
    _currentEpoch = manualEpoch;
    _lastUpdateTick = millis();
    _lastSyncMs = millis();

    if (_bootTimestamp == 0) _bootTimestamp = _currentEpoch - (millis() / 1000);
}

void IskakINO_FastNTP::setUtcEpoch(uint32_t utcEpoch) {
    setEpoch(utcEpoch + _gmtOffsetSec + _daylightOffsetSec);
}

void IskakINO_FastNTP::forceUpdate() {
    _state = STATE_SEND_REQUEST;
    _lastSyncMs = millis() - _syncInterval;
}

#endif // defined(ISKAKINO_HAS_WIFI)
