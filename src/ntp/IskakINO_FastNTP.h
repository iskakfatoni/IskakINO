#ifndef ISKAKINO_FASTNTP_H
#define ISKAKINO_FASTNTP_H

#include <Arduino.h>
#include <time.h>

#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Version.h"

// PENTING: Udp.h TIDAK tersedia di board non-WiFi (mis. AVR Uno/Nano) tanpa
// library shield jaringan tambahan. Sama seperti WifiPortal, SELURUH isi
// file ini (bukan cuma #include Udp.h) dibungkus guard ini, karena Arduino
// mengkompilasi SEMUA .cpp di src/ terlepas dari pemakaian sketch — di
// board non-WiFi, file ini otomatis jadi kosong (tidak ada kode
// ter-generate, tidak error), bukan gagal compile.
#if defined(ISKAKINO_HAS_WIFI)

#include <Udp.h>

// PILOT REFACTOR #4 — bagian dari penggabungan ekosistem IskakINO. Beda
// dari ArduFast/WifiPortal/Storage: modul ini TIDAK punya macro platform
// (murni UDP standar, tidak sentuh hardware langsung) dan SEBELUMNYA TIDAK
// PUNYA logging sama sekali — jadi tidak ada yang "dipindah" ke core, cuma
// ada satu penambahan baru:
//   - IskakINO_Logger (_logger) ditambahkan sebagai diagnostik OPSIONAL
//     (default nonaktif, beda dari ArduFast/WifiPortal/Storage yang
//     defaultnya aktif — di sini defaultnya nonaktif karena sebelumnya
//     memang tidak pernah ada output apa pun, jadi default silent adalah
//     yang paling backward-compatible). Aktifkan lewat setDebug(true).
// State machine (backoff eksponensial, retry-lebih-cepat, rotate server)
// SENGAJA TIDAK dipetakan ke IskakINO_Scheduler — logikanya terlalu
// spesifik (interval yang berubah-ubah akibat backoff, forceUpdate() yang
// memanipulasi baseline langsung) untuk dipetakan bersih ke every()/once()
// tanpa risiko mengubah perilaku. Modul ini tidak wajib pakai semua
// komponen core; itu memang bukan tujuannya.

// --- Konfigurasi build-time (bisa di-override sebelum #include jika perlu) ---
#ifndef ISKAKINO_FASTNTP_MAX_SERVERS
#define ISKAKINO_FASTNTP_MAX_SERVERS 4
#endif

#ifndef ISKAKINO_FASTNTP_MAX_SYNC_INTERVAL_MS
#define ISKAKINO_FASTNTP_MAX_SYNC_INTERVAL_MS 21600000UL // 6 jam - cap untuk backoff Kiss-of-Death
#endif

/**
 * @enum NTP_Language
 * Pilihan bahasa untuk nama hari dan bulan.
 */
enum NTP_Language { LANG_EN, LANG_ID };

/**
 * @enum NTP_State
 * Status State Machine untuk proses non-blocking.
 */
enum NTP_State { STATE_IDLE, STATE_SEND_REQUEST, STATE_AWAIT_RESPONSE };

/**
 * Callback dipanggil saat sinkronisasi NTP berhasil.
 * @param utcEpoch Epoch UTC murni (belum ditambah offset GMT/DST).
 */
typedef void (*NTPSyncCallback)(uint32_t utcEpoch);

/**
 * Callback dipanggil setiap kali satu siklus request NTP gagal
 * (timeout ATAU server membalas Kiss-of-Death).
 * @param consecutiveFails Jumlah kegagalan berturut-turut ke server yang aktif saat ini.
 */
typedef void (*NTPFailCallback)(uint8_t consecutiveFails);

class IskakINO_FastNTP {
  private:
    UDP* _udp;

    // --- Multi-server (v1.1.0) ---
    const char* _serverList[ISKAKINO_FASTNTP_MAX_SERVERS];
    uint8_t _serverCount;
    uint8_t _currentServerIdx;
    const char* _ntpServer;        // pointer ke server yang sedang aktif (== _serverList[_currentServerIdx])
    uint8_t _consecutiveFails;
    uint8_t _failThreshold;        // default 3 - jumlah gagal berturut-turut sebelum rotate server

    long _gmtOffsetSec;
    int _daylightOffsetSec;

    uint32_t _syncInterval;        // interval AKTIF saat ini (bisa membesar sementara akibat backoff KoD)
    uint32_t _baseSyncInterval;    // interval "normal" yang di-set user, dipulihkan setelah sync sukses
    uint32_t _requestTimeoutMs;    // default 2000 ms
    uint32_t _lastSyncMs;
    uint32_t _requestMs;
    uint32_t _currentEpoch;        // epoch LOKAL (sudah + gmtOffset + daylightOffset)
    uint32_t _lastUpdateTick;
    uint32_t _bootTimestamp;

    // --- Diagnostics (v1.1.0) ---
    uint16_t _syncSuccessCount;
    uint16_t _syncFailCount;

    // --- Callbacks (v1.1.0) ---
    NTPSyncCallback _onSyncCb;
    NTPFailCallback _onFailCb;

    NTP_State _state = STATE_IDLE;
    byte _packetBuffer[48];

    // BARU (pilot refactor): diagnostik opsional, default nonaktif (lihat
    // banner di atas). TIDAK ada satu pun perilaku lama yang berubah.
    IskakINO_Logger _logger;

    void sendNTPPacket();
    void _initCommon(UDP& udp);
    void _rotateServer();          // pindah ke server berikutnya dalam list (round-robin)

  public:
    IskakINO_FastNTP(UDP& udp, const char* server = "pool.ntp.org");

    /**
     * @brief Konstruktor multi-server. Jika salah satu server gagal berturut-turut
     * (lihat setFailThreshold()) atau membalas Kiss-of-Death, library otomatis
     * pindah ke server berikutnya secara round-robin.
     * @param serverList Array string nama host NTP. Disimpan sebagai pointer,
     * pastikan array/string ini tetap hidup selama umur objek (mis. `static const char*`).
     * @param count Jumlah server dalam array. Di-cap ke ISKAKINO_FASTNTP_MAX_SERVERS (default 4).
     */
    IskakINO_FastNTP(UDP& udp, const char* const* serverList, uint8_t count);

    void begin(long gmtOffset = 25200, int daylightOffset = 0);
    void update();
    void forceUpdate();

    // --- Status & Validasi ---
    bool isTimeSet() const { return _currentEpoch > 0; }
    bool isSynced() const { return isTimeSet(); }
    bool isTimeReliable(uint32_t maxAgeSeconds = 86400);
    uint32_t getEpoch();                // epoch lokal (legacy, sama seperti v1.0.x)
    uint32_t getUtcEpoch();             // (v1.1.0) epoch UTC murni, tanpa offset GMT/DST
    uint32_t getUptimeSeconds();
    uint32_t getMillisSinceLastSync();

    // --- Getters Waktu (Satuan) ---
    int getSeconds();
    int getMinutes();
    int getHours();
    int getDay();                       // Hari dalam bulan (1 - 31)
    int getDayOfWeek();                 // Hari dalam pekan: 0 (Minggu) s/d 6 (Sabtu)
    int getMonth();                     // Bulan (1 - 12)
    int getYear();                      // Tahun (mis. 2026)

    // --- Getters Nama (Multibahasa) ---
    String getDayName(NTP_Language lang = LANG_ID);
    String getMonthName(NTP_Language lang = LANG_ID);

    // --- Getters Formatted String ---
    String getFormattedTime();
    String getFormattedDate(char separator = '-');

    // --- Fitur Kontrol & Alarm ---
    bool isAlarmActive(int hr, int min, int sec = 0);
    bool isAlarmActive(int hr, int min, int sec, bool &firedFlag);

    void setEpoch(uint32_t manualEpoch);      // input = epoch LOKAL (legacy, sama seperti v1.0.x)
    void setUtcEpoch(uint32_t utcEpoch);      // (v1.1.0) input = epoch UTC murni

    void setSyncInterval(uint32_t intervalMs) { _baseSyncInterval = intervalMs; _syncInterval = intervalMs; }

    // --- Multi-server & Reliability (v1.1.0) ---
    const char* getCurrentServer() const { return _ntpServer; }
    uint8_t getServerCount() const { return _serverCount; }
    void setFailThreshold(uint8_t threshold) { _failThreshold = (threshold == 0) ? 1 : threshold; }
    void setRequestTimeout(uint32_t timeoutMs) { _requestTimeoutMs = timeoutMs; }

    // --- Diagnostics (v1.1.0) ---
    uint16_t getSyncSuccessCount() const { return _syncSuccessCount; }
    uint16_t getSyncFailCount() const { return _syncFailCount; }
    uint8_t getConsecutiveFails() const { return _consecutiveFails; }

    // --- Event Callbacks (v1.1.0) ---
    void onSync(NTPSyncCallback cb) { _onSyncCb = cb; }
    void onSyncFail(NTPFailCallback cb) { _onFailCb = cb; }

    // BARU (pilot refactor): aktifkan pesan diagnostik opsional ke Serial
    // (sync sukses/gagal, rotate server, Kiss-of-Death). Default nonaktif.
    void setDebug(bool debugMode) { _logger.setDebug(debugMode); }
};

#endif // defined(ISKAKINO_HAS_WIFI)

#endif
