#ifndef ISKAKINO_WIFIPORTAL_H
#define ISKAKINO_WIFIPORTAL_H

#include <Arduino.h>

#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Scheduler.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Result.h"
#include "core/IskakINO_Version.h"

// --- Seleksi Arsitektur Otomatis ---
// PENTING: DNSServer.h/LittleFS.h/WiFi.h dst. TIDAK tersedia di board
// non-WiFi (mis. AVR Uno/Nano). Arduino mengkompilasi SEMUA file .cpp di
// src/ (rekursif) begitu sketch meng-include library ini, TERLEPAS dari
// apakah sketch itu benar-benar memakai WifiPortal — jadi guard di sini
// TIDAK BOLEH pakai #error (itu akan bikin seluruh IskakINO gagal compile
// di AVR walau sketch cuma pakai ArduFast/LCD/dst). Sebagai gantinya,
// SELURUH isi file ini (struct, class, semuanya) dibungkus
// #if defined(ISKAKINO_HAS_WIFI) — di board non-WiFi, file ini otomatis
// jadi kosong secara efektif (tidak ada kode yang di-generate, tidak error).
#if defined(ISKAKINO_HAS_WIFI)

#include <DNSServer.h>
#if defined(ISKAKINO_PLATFORM_ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  #include <Preferences.h>
  #include <Update.h>
  using IskakWebServer = WebServer;
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  #include <LittleFS.h>
  #include <Updater.h>
  using IskakWebServer = ESP8266WebServer;
#endif

// Jumlah maksimal kredensial WiFi yang bisa disimpan (hardcode + hasil Save di portal)
#define ISKAKINO_MAX_WIFI 5

// Struktur untuk Parameter Kustom
struct IskakParam {
  const char* id;
  const char* label;
  char* value;
  int length;
};

// Satu slot kredensial WiFi
struct IskakWifiCred {
  char ssid[33];
  char pass[65];
};

// State internal koneksi (dipakai oleh begin()/beginAsync()/tick())
enum class IskakPortalState : uint8_t {
  IDLE,
  SCANNING,
  CONNECTING,
  CONNECTED,
  PORTAL
};

// PILOT REFACTOR #2 — bagian dari penggabungan ekosistem IskakINO (setelah
// ArduFast). Signature semua fungsi PUBLIK di bawah TIDAK BERUBAH dari
// v1.1.0 standalone. Yang berubah cuma internal:
//   - Macro #if defined(ESP32)/ESP8266 mentah (dulu terulang di ~8 titik)
//     kini pakai ISKAKINO_PLATFORM_ESP32/ESP8266 dari core/IskakINO_Platform.h.
//   - _lastWifiCheck (every 30 detik) & _portalStartTime (timeout portal)
//     yang dulu masing-masing pakai variabel millis() manual sendiri, kini
//     pakai satu IskakINO_Scheduler bersama (id 0 = timeout portal,
//     id 1 = cek auto-reconnect).
//   - Serial.println() informatif kini lewat IskakINO_Logger (default aktif
//     via setDebug(true) di constructor, supaya perilaku lama—selalu
//     mencetak—tetap sama persis). Panggil setDebug(false) untuk membungkam.
//   - BARU: lastError() mengembalikan IskakINO_Result dari operasi
//     load/save terakhir (sebelumnya loadParams()/saveParams()/dst. gagal
//     secara diam-diam tanpa cara mengetahui penyebabnya dari kode
//     pemanggil). Sekalian memperbaiki bug lama: varian ESP8266 di
//     saveParams()/saveWifiList() tidak memvalidasi LittleFS.open()
//     sebelum menulis (pola sama yang pernah diperbaiki di IskakINO_Storage
//     v1.0.1) — sekarang divalidasi dan tercermin di lastError().
class IskakINO_WifiPortal {
  public:
    IskakINO_WifiPortal();
    virtual ~IskakINO_WifiPortal();
    IskakWebServer* _server;
    IskakWebServer* server() { return _server; }

    DNSServer _dnsServer;

    // --- Fungsi Utama ---
    // Versi blocking (kompatibel dengan v1.0.x): menunggu sampai konek atau portal terbuka.
    bool begin(const char* apName, const char* apPass = NULL);
    // Versi non-blocking: langsung return, proses scan/connect ditangani bertahap lewat tick().
    void beginAsync(const char* apName, const char* apPass = NULL);
    void tick(); // WAJIB dipanggil di loop()
    void resetSettings();
    void setBrandName(const char* name);

    // --- Status Koneksi ---
    bool isConnected();
    IskakPortalState state();

    // --- Multi WiFi ---
    // Daftarkan kandidat WiFi (bisa dipanggil berkali-kali sebelum begin()/beginAsync()).
    // Saat konek, semua kandidat yang jaringannya terjangkau akan dicoba,
    // diurutkan dari sinyal (RSSI) terkuat. Kredensial yang disimpan lewat
    // portal (tombol SAVE) juga otomatis masuk ke daftar ini secara permanen.
    bool addWifi(const char* ssid, const char* pass = NULL);
    bool removeWifi(const char* ssid);
    void clearWifiList();
    uint8_t getWifiCount() const { return _wifiCount; }
    const char* getWifiSSID(uint8_t index) const;
    String getCurrentSSID() const;

    // --- Custom Parameters ---
    void addParameter(const char* id, const char* label, char* value, int length);

    // --- Keamanan ---
    // Set PIN admin untuk melindungi endpoint /reboot dan /clear.
    // Jika tidak dipanggil, kedua endpoint tetap terbuka tanpa PIN (perilaku lama).
    void setAdminPin(const char* pin);

    // --- Maintenance ---
    void enableOTA(bool status);
    void setPortalTimeout(int sec); // 0 = tanpa batas waktu (default)
    void setupPortal();

    // BARU (pilot refactor): lihat penyebab kegagalan load/save terakhir
    // (loadParams/saveParams/loadWifiList/saveWifiList). OK berarti operasi
    // terakhir sukses atau belum pernah gagal.
    IskakINO_Result lastError() const { return _lastError; }

    // BARU (pilot refactor): default aktif (setara perilaku v1.1.0 lama
    // yang selalu mencetak ke Serial). Panggil setDebug(false) untuk
    // membungkam pesan informatif "[IskakINO] ..." tanpa menghapusnya
    // dari kode.
    void setDebug(bool debugMode) { _logger.setDebug(debugMode); }

    // --- Live Web Log Viewer ---
    void appendLog(const String& msg);
    void appendLog(const char* msg) { if (msg) appendLog(String(msg)); }
    String getLogsJson() const;

  private:
    const char* _apName;
    const char* _apPass;
    const char* _brandName = "IskakINO Portal";

    int _reconnectAttempts = 0;
    const int _maxReconnectAttempts = 3;

    bool _portalActive = false;
    bool _otaEnabled = false;
    int  _timeout = 0; // detik, 0 = nonaktif

    // --- Live Log Ring-Buffer ---
    static const uint8_t LOG_BUFFER_SIZE = 15;
    String  _logBuffer[LOG_BUFFER_SIZE];
    uint8_t _logHead = 0;
    uint8_t _logCount = 0;

    // BARU (pilot refactor): menggantikan _portalStartTime (id 0, once())
    // dan _lastWifiCheck (id 1, every()) dari v1.1.0 lama dengan satu
    // Scheduler bersama.
    IskakINO_Scheduler _scheduler{2};
    IskakINO_Logger _logger;
    IskakINO_Result _lastError = IskakINO_Result::OK;

    IskakParam* _params[10];
    uint8_t _paramCount = 0;

    // --- Multi WiFi state ---
    IskakWifiCred _wifiList[ISKAKINO_MAX_WIFI];
    uint8_t _wifiCount = 0;

    IskakPortalState _state = IskakPortalState::IDLE;
    static const unsigned long CANDIDATE_CONNECT_TIMEOUT_MS = 8000;
    static const unsigned long SCAN_SAFETY_TIMEOUT_MS = 10000;
    int _candidateOrder[ISKAKINO_MAX_WIFI];
    int _candidateTotal = 0;
    int _candidateIndex = 0;
    unsigned long _stateStartMs = 0;

    // --- Admin PIN ---
    char _adminPin[9] = {0};
    bool checkAdminPin();

    // --- Handlers ---
    void handleRoot();
    void handleSave();
    void handleDeleteWifi();
    void handleOTA();
    void handleLogsApi();

    // --- State machine helpers ---
    void pumpStateMachine();
    void startScan();
    void tryNextCandidate();

    // --- Storage & Helpers ---
    void loadParams();
    void saveParams();
    void loadWifiList();
    void saveWifiList();
    void migrateLegacyConfig();
    bool loadLegacyConfig(String &ssid, String &pass); // format storage v1.0.x, hanya untuk migrasi
    void mergeWifiCredential(const char* ssid, const char* pass);
    String htmlEscape(const String& raw);
    String jsEscape(const String& raw);
};

#endif // defined(ISKAKINO_HAS_WIFI)

#endif
