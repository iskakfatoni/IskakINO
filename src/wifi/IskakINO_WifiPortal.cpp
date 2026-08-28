#include "IskakINO_WifiPortal.h"

// Seluruh isi file ini otomatis kosong (tidak ada kode ter-generate, tidak
// error) di board non-WiFi, karena IskakINO_WifiPortal.h sendiri juga
// dibungkus guard yang sama — lihat komentar di header untuk alasannya
// (Arduino mengkompilasi SEMUA .cpp di src/ terlepas dari pemakaian sketch).
#if defined(ISKAKINO_HAS_WIFI)

// Logika cerdas untuk mendeteksi tipe board
#if defined(ISKAKINO_PLATFORM_ESP32)
  #include <Update.h>
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #include <Updater.h>
#endif

#define ISKAKINO_PARAM_FILE   "/iskak_param.txt"
#define ISKAKINO_WIFI_FILE    "/iskak_wifi.txt"
#define ISKAKINO_LEGACY_FILE  "/iskak.txt"
#define ISKAKINO_PREFS_NS     "iskakino"
#define ISKAKINO_PARAM_NS     "iskak_param"
#define ISKAKINO_WIFI_NS      "iskak_wifi"

// ID slot pada _scheduler (lihat komentar di header)
#define ISKAKINO_SCHED_PORTAL_TIMEOUT 0
#define ISKAKINO_SCHED_WIFI_CHECK     1

IskakINO_WifiPortal::IskakINO_WifiPortal() {
    _server = new IskakWebServer(80);
    _paramCount = 0;
    _wifiCount = 0;
    // Perilaku default sama seperti v1.1.0 lama: pesan "[IskakINO] ..."
    // selalu tercetak ke Serial kecuali user memanggil setDebug(false).
    _logger.setDebug(true);
}

IskakINO_WifiPortal::~IskakINO_WifiPortal() {
    if (_server) {
        delete _server;
        _server = nullptr;
    }
    for (uint8_t i = 0; i < _paramCount; i++) {
        if (_params[i]) {
            delete _params[i];
            _params[i] = nullptr;
        }
    }
}

// ============================================================
// KONEKSI (blocking / non-blocking) & STATE MACHINE
// ============================================================

bool IskakINO_WifiPortal::begin(const char* apName, const char* apPass) {
    _logger.log(F("\n[IskakINO] Memulai Portal..."));
    beginAsync(apName, apPass);

    // Pump state machine secara blocking supaya begin() tetap kompatibel
    // dengan perilaku v1.0.x (menunggu sampai konek atau portal terbuka).
    while (_state != IskakPortalState::CONNECTED && _state != IskakPortalState::PORTAL) {
        pumpStateMachine();
        delay(50);
    }
    return (_state == IskakPortalState::CONNECTED);
}

void IskakINO_WifiPortal::beginAsync(const char* apName, const char* apPass) {
    _apName = apName;
    _apPass = apPass;

    loadParams();
    loadWifiList();

    if (_wifiCount == 0) {
        _logger.log(F("[IskakINO] Tidak ada kredensial WiFi tersimpan. Membuka Portal..."));
        setupPortal();
        return;
    }

    startScan();
}

void IskakINO_WifiPortal::startScan() {
    _logger.log(F("[IskakINO] Memindai jaringan WiFi di sekitar..."));
    WiFi.mode(WIFI_STA);
    WiFi.scanNetworks(true); // async scan, tidak blocking
    _state = IskakPortalState::SCANNING;
    _stateStartMs = millis();
}

void IskakINO_WifiPortal::pumpStateMachine() {
    switch (_state) {
        case IskakPortalState::SCANNING: {
            int n = WiFi.scanComplete();

            if (n == -2) { // scan belum pernah dijalankan / gagal start, coba lagi
                WiFi.scanNetworks(true);
                _stateStartMs = millis();
                return;
            }
            if (n == -1) { // masih berjalan
                if (millis() - _stateStartMs < SCAN_SAFETY_TIMEOUT_MS) return;
                n = 0; // safety timeout: anggap tidak ada hasil, lanjut saja
            }

            // Cocokkan hasil scan dengan daftar kredensial tersimpan,
            // urutkan berdasarkan RSSI terkuat.
            int order[ISKAKINO_MAX_WIFI];
            int32_t rssi[ISKAKINO_MAX_WIFI];
            _candidateTotal = 0;

            for (int w = 0; w < _wifiCount; w++) {
                bool found = false;
                int32_t bestRssi = -1000;
                for (int i = 0; i < n; i++) {
                    if (WiFi.SSID(i) == String(_wifiList[w].ssid)) {
                        found = true;
                        if (WiFi.RSSI(i) > bestRssi) bestRssi = WiFi.RSSI(i);
                    }
                }
                if (found) {
                    order[_candidateTotal] = w;
                    rssi[_candidateTotal] = bestRssi;
                    _candidateTotal++;
                }
            }
            // Selection sort (aman untuk N kecil, maks ISKAKINO_MAX_WIFI)
            for (int a = 0; a < _candidateTotal; a++) {
                int best = a;
                for (int b = a + 1; b < _candidateTotal; b++) {
                    if (rssi[b] > rssi[best]) best = b;
                }
                if (best != a) {
                    int tmpO = order[a]; order[a] = order[best]; order[best] = tmpO;
                    int32_t tmpR = rssi[a]; rssi[a] = rssi[best]; rssi[best] = tmpR;
                }
            }
            memcpy(_candidateOrder, order, sizeof(order));
            WiFi.scanDelete();

            if (_candidateTotal == 0) {
                _logger.log(F("[IskakINO] Tidak ada jaringan tersimpan yang terjangkau. Membuka Portal..."));
                setupPortal();
                return;
            }

            _candidateIndex = 0;
            tryNextCandidate();
            break;
        }

        case IskakPortalState::CONNECTING: {
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("\n[IskakINO] Terhubung! IP: " + WiFi.localIP().toString());
                _state = IskakPortalState::CONNECTED;
                _reconnectAttempts = 0;
                return;
            }
            if (millis() - _stateStartMs > CANDIDATE_CONNECT_TIMEOUT_MS) {
                _candidateIndex++;
                tryNextCandidate();
            }
            break;
        }

        default:
            break; // IDLE, CONNECTED, PORTAL: tidak ada yang perlu dipompa di sini
    }
}

void IskakINO_WifiPortal::tryNextCandidate() {
    if (_candidateIndex >= _candidateTotal) {
        _logger.log(F("[IskakINO] Semua kandidat WiFi tersimpan gagal terhubung. Membuka Portal..."));
        setupPortal();
        return;
    }
    int w = _candidateOrder[_candidateIndex];
    Serial.print("[IskakINO] Mencoba: ");
    Serial.println(_wifiList[w].ssid);
    WiFi.begin(_wifiList[w].ssid, _wifiList[w].pass);
    _stateStartMs = millis();
    _state = IskakPortalState::CONNECTING;
}

bool IskakINO_WifiPortal::isConnected() {
    return _state == IskakPortalState::CONNECTED && WiFi.status() == WL_CONNECTED;
}

IskakPortalState IskakINO_WifiPortal::state() {
    return _state;
}

// ============================================================
// MULTI WIFI
// ============================================================

bool IskakINO_WifiPortal::addWifi(const char* ssid, const char* pass) {
    if (!ssid || strlen(ssid) == 0) return false;

    // Kalau SSID sudah ada di daftar, cukup update passwordnya
    for (int i = 0; i < _wifiCount; i++) {
        if (strcmp(_wifiList[i].ssid, ssid) == 0) {
            strncpy(_wifiList[i].pass, pass ? pass : "", sizeof(_wifiList[i].pass) - 1);
            _wifiList[i].pass[sizeof(_wifiList[i].pass) - 1] = '\0';
            return true;
        }
    }

    if (_wifiCount >= ISKAKINO_MAX_WIFI) {
        Serial.print("[IskakINO] Peringatan: daftar WiFi penuh (maks ");
        Serial.print(ISKAKINO_MAX_WIFI);
        Serial.println("), addWifi() diabaikan.");
        return false;
    }

    strncpy(_wifiList[_wifiCount].ssid, ssid, sizeof(_wifiList[_wifiCount].ssid) - 1);
    _wifiList[_wifiCount].ssid[sizeof(_wifiList[_wifiCount].ssid) - 1] = '\0';
    strncpy(_wifiList[_wifiCount].pass, pass ? pass : "", sizeof(_wifiList[_wifiCount].pass) - 1);
    _wifiList[_wifiCount].pass[sizeof(_wifiList[_wifiCount].pass) - 1] = '\0';
    _wifiCount++;
    return true;
}

bool IskakINO_WifiPortal::removeWifi(const char* ssid) {
    if (!ssid || strlen(ssid) == 0) return false;
    int foundIndex = -1;
    for (int i = 0; i < _wifiCount; i++) {
        if (strcmp(_wifiList[i].ssid, ssid) == 0) {
            foundIndex = i;
            break;
        }
    }
    if (foundIndex == -1) return false;

    for (int i = foundIndex; i < _wifiCount - 1; i++) {
        _wifiList[i] = _wifiList[i + 1];
    }
    _wifiCount--;
    saveWifiList();
    return true;
}

void IskakINO_WifiPortal::clearWifiList() {
    _wifiCount = 0;
    saveWifiList();
}

const char* IskakINO_WifiPortal::getWifiSSID(uint8_t index) const {
    if (index >= _wifiCount) return "";
    return _wifiList[index].ssid;
}

String IskakINO_WifiPortal::getCurrentSSID() const {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.SSID();
    }
    return "";
}

void IskakINO_WifiPortal::handleDeleteWifi() {
    if (!checkAdminPin()) {
        _server->send(401, "text/plain", "Unauthorized: PIN salah atau tidak disertakan.");
        return;
    }
    String ssid = _server->arg("s");
    if (ssid.length() > 0) {
        removeWifi(ssid.c_str());
    }
    _server->sendHeader("Location", "/");
    _server->send(302, "text/plain", "");
}

void IskakINO_WifiPortal::mergeWifiCredential(const char* ssid, const char* pass) {
    if (!ssid || strlen(ssid) == 0) return;

    for (int i = 0; i < _wifiCount; i++) {
        if (strcmp(_wifiList[i].ssid, ssid) == 0) {
            strncpy(_wifiList[i].pass, pass ? pass : "", sizeof(_wifiList[i].pass) - 1);
            _wifiList[i].pass[sizeof(_wifiList[i].pass) - 1] = '\0';
            saveWifiList();
            return;
        }
    }

    if (_wifiCount >= ISKAKINO_MAX_WIFI) {
        // Daftar penuh -> FIFO, buang yang paling lama (index 0)
        for (int i = 1; i < _wifiCount; i++) _wifiList[i - 1] = _wifiList[i];
        _wifiCount--;
    }

    strncpy(_wifiList[_wifiCount].ssid, ssid, sizeof(_wifiList[_wifiCount].ssid) - 1);
    _wifiList[_wifiCount].ssid[sizeof(_wifiList[_wifiCount].ssid) - 1] = '\0';
    strncpy(_wifiList[_wifiCount].pass, pass ? pass : "", sizeof(_wifiList[_wifiCount].pass) - 1);
    _wifiList[_wifiCount].pass[sizeof(_wifiList[_wifiCount].pass) - 1] = '\0';
    _wifiCount++;
    saveWifiList();
}

void IskakINO_WifiPortal::loadWifiList() {
    #if defined(ISKAKINO_PLATFORM_ESP32)
        Preferences prefs;
        prefs.begin(ISKAKINO_WIFI_NS, true);
        int n = prefs.getInt("n", -1);
        if (n < 0) {
            prefs.end();
            migrateLegacyConfig();
            return;
        }
        for (int i = 0; i < n && i < ISKAKINO_MAX_WIFI; i++) {
            String s = prefs.getString(("s" + String(i)).c_str(), "");
            String p = prefs.getString(("p" + String(i)).c_str(), "");
            if (s.length() > 0) addWifi(s.c_str(), p.c_str());
        }
        prefs.end();
        _lastError = IskakINO_Result::OK;
    #elif defined(ISKAKINO_PLATFORM_ESP8266)
        if (!LittleFS.begin()) {
            _lastError = IskakINO_Result::NOT_FOUND;
            _logger.logResult(F("loadWifiList: LittleFS.begin"), _lastError);
            return;
        }
        if (!LittleFS.exists(ISKAKINO_WIFI_FILE)) {
            migrateLegacyConfig();
            return;
        }
        File f = LittleFS.open(ISKAKINO_WIFI_FILE, "r");
        // BARU (pilot refactor): dulu tidak divalidasi — kalau open() gagal,
        // f.available()/f.readStringUntil() dipanggil di objek File kosong.
        if (!f) {
            _lastError = IskakINO_Result::WRITE_FAILED;
            _logger.logResult(F("loadWifiList: LittleFS.open"), _lastError);
            return;
        }
        while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            int sep = line.indexOf('|');
            if (sep <= 0) continue;
            String s = line.substring(0, sep);
            String p = line.substring(sep + 1);
            addWifi(s.c_str(), p.c_str());
        }
        f.close();
        _lastError = IskakINO_Result::OK;
    #endif
}

void IskakINO_WifiPortal::saveWifiList() {
    #if defined(ISKAKINO_PLATFORM_ESP32)
        Preferences prefs;
        prefs.begin(ISKAKINO_WIFI_NS, false);
        prefs.putInt("n", _wifiCount);
        for (int i = 0; i < _wifiCount; i++) {
            prefs.putString(("s" + String(i)).c_str(), _wifiList[i].ssid);
            prefs.putString(("p" + String(i)).c_str(), _wifiList[i].pass);
        }
        prefs.end();
        _lastError = IskakINO_Result::OK;
    #elif defined(ISKAKINO_PLATFORM_ESP8266)
        File f = LittleFS.open(ISKAKINO_WIFI_FILE, "w");
        // BARU (pilot refactor): sebelumnya TIDAK divalidasi sama sekali —
        // kalau open() gagal (mis. filesystem penuh/corrupt), kode lama
        // tetap lanjut menulis ke objek File kosong tanpa ada tanda gagal.
        if (!f) {
            _lastError = IskakINO_Result::WRITE_FAILED;
            _logger.logResult(F("saveWifiList: LittleFS.open"), _lastError);
            return;
        }
        for (int i = 0; i < _wifiCount; i++) {
            f.print(_wifiList[i].ssid);
            f.print('|');
            f.println(_wifiList[i].pass);
        }
        f.close();
        _lastError = IskakINO_Result::OK;
    #endif
}

// Migrasi satu-kali dari format penyimpanan single-SSID v1.0.x ke daftar multi-WiFi v1.1.0.
void IskakINO_WifiPortal::migrateLegacyConfig() {
    String ssid, pass;
    if (loadLegacyConfig(ssid, pass) && ssid.length() > 0) {
        _logger.log(F("[IskakINO] Migrasi konfigurasi WiFi lama (v1.0.x) ke format v1.1.0..."));
        addWifi(ssid.c_str(), pass.c_str());
        saveWifiList();
    }
}

bool IskakINO_WifiPortal::loadLegacyConfig(String &ssid, String &pass) {
    #if defined(ISKAKINO_PLATFORM_ESP32)
        Preferences prefs;
        prefs.begin(ISKAKINO_PREFS_NS, true);
        ssid = prefs.getString("ssid", "");
        pass = prefs.getString("pass", "");
        prefs.end();
        return (ssid != "");
    #elif defined(ISKAKINO_PLATFORM_ESP8266)
        if (!LittleFS.begin()) return false;
        if (!LittleFS.exists(ISKAKINO_LEGACY_FILE)) return false;
        File f = LittleFS.open(ISKAKINO_LEGACY_FILE, "r");
        ssid = f.readStringUntil('\n');
        pass = f.readStringUntil('\n');
        f.close();
        ssid.trim();
        pass.trim();
        return (ssid != "");
    #else
        return false;
    #endif
}

// ============================================================
// CUSTOM PARAMETERS
// ============================================================

void IskakINO_WifiPortal::addParameter(const char* id, const char* label, char* value, int length) {
    if (_paramCount < 10) {
        _params[_paramCount] = new IskakParam{id, label, value, length};
        _paramCount++;
    } else {
        _logger.log(F("[IskakINO] Peringatan: Batas maksimal 10 custom parameter tercapai, parameter diabaikan."));
    }
}

void IskakINO_WifiPortal::loadParams() {
    if (_paramCount == 0) return;

    #if defined(ISKAKINO_PLATFORM_ESP32)
        Preferences prefs;
        prefs.begin(ISKAKINO_PARAM_NS, true);
        for (int i = 0; i < _paramCount; i++) {
            String val = prefs.getString(_params[i]->id, "");
            if (val.length() > 0) {
                int maxLen = _params[i]->length;
                int copyLen = val.length();
                if (copyLen > maxLen - 1) copyLen = maxLen - 1;
                memcpy(_params[i]->value, val.c_str(), copyLen);
                _params[i]->value[copyLen] = '\0';
            }
        }
        prefs.end();
        _lastError = IskakINO_Result::OK;
    #elif defined(ISKAKINO_PLATFORM_ESP8266)
        if (!LittleFS.begin()) {
            _lastError = IskakINO_Result::NOT_FOUND;
            _logger.logResult(F("loadParams: LittleFS.begin"), _lastError);
            return;
        }
        if (!LittleFS.exists(ISKAKINO_PARAM_FILE)) return; // belum pernah save, bukan error
        File f = LittleFS.open(ISKAKINO_PARAM_FILE, "r");
        // BARU (pilot refactor): dulu tidak divalidasi.
        if (!f) {
            _lastError = IskakINO_Result::WRITE_FAILED;
            _logger.logResult(F("loadParams: LittleFS.open"), _lastError);
            return;
        }
        while (f.available()) {
            String line = f.readStringUntil('\n');
            line.trim();
            int sep = line.indexOf('=');
            if (sep <= 0) continue;
            String id = line.substring(0, sep);
            String val = line.substring(sep + 1);
            for (int i = 0; i < _paramCount; i++) {
                if (id == _params[i]->id) {
                    int maxLen = _params[i]->length;
                    int copyLen = val.length();
                    if (copyLen > maxLen - 1) copyLen = maxLen - 1;
                    memcpy(_params[i]->value, val.c_str(), copyLen);
                    _params[i]->value[copyLen] = '\0';
                    break;
                }
            }
        }
        f.close();
        _lastError = IskakINO_Result::OK;
    #endif
}

void IskakINO_WifiPortal::saveParams() {
    if (_paramCount == 0) return;

    #if defined(ISKAKINO_PLATFORM_ESP32)
        Preferences prefs;
        prefs.begin(ISKAKINO_PARAM_NS, false);
        for (int i = 0; i < _paramCount; i++) {
            prefs.putString(_params[i]->id, String(_params[i]->value));
        }
        prefs.end();
        _lastError = IskakINO_Result::OK;
    #elif defined(ISKAKINO_PLATFORM_ESP8266)
        File f = LittleFS.open(ISKAKINO_PARAM_FILE, "w");
        // BARU (pilot refactor): sebelumnya TIDAK divalidasi — ini varian
        // ESP8266 dari bug yang sama yang pernah diperbaiki di
        // IskakINO_Storage v1.0.1 (missing File::open() validation).
        if (!f) {
            _lastError = IskakINO_Result::WRITE_FAILED;
            _logger.logResult(F("saveParams: LittleFS.open"), _lastError);
            return;
        }
        for (int i = 0; i < _paramCount; i++) {
            f.print(_params[i]->id);
            f.print('=');
            f.println(_params[i]->value);
        }
        f.close();
        _lastError = IskakINO_Result::OK;
    #endif
}

// ============================================================
// PORTAL WEB SERVER
// ============================================================

void IskakINO_WifiPortal::setBrandName(const char* name) { _brandName = name; }
void IskakINO_WifiPortal::enableOTA(bool status) { _otaEnabled = status; }
void IskakINO_WifiPortal::setPortalTimeout(int sec) { _timeout = sec; }

void IskakINO_WifiPortal::setAdminPin(const char* pin) {
    if (!pin) { _adminPin[0] = '\0'; return; }
    strncpy(_adminPin, pin, sizeof(_adminPin) - 1);
    _adminPin[sizeof(_adminPin) - 1] = '\0';
}

bool IskakINO_WifiPortal::checkAdminPin() {
    if (_adminPin[0] == '\0') return true; // tidak di-set -> perilaku lama, tetap terbuka
    String p = _server->arg("pin");
    return p == String(_adminPin);
}

void IskakINO_WifiPortal::setupPortal() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_apName, _apPass);
    _dnsServer.start(53, "*", WiFi.softAPIP());

    if (!_portalActive) {
        // Daftarkan route hanya sekali agar tidak dobel saat setupPortal() dipanggil ulang
        // (misalnya lewat manual trigger).
        _server->on("/", std::bind(&IskakINO_WifiPortal::handleRoot, this));
        _server->on("/save", HTTP_POST, std::bind(&IskakINO_WifiPortal::handleSave, this));
        _server->on("/delete_wifi", std::bind(&IskakINO_WifiPortal::handleDeleteWifi, this));

        // --- Deteksi Captive Portal OS (agar popup "Sign in to network" muncul otomatis) ---
        _server->on("/generate_204", std::bind(&IskakINO_WifiPortal::handleRoot, this));            // Android
        _server->on("/gen_204", std::bind(&IskakINO_WifiPortal::handleRoot, this));                 // Android
        _server->on("/hotspot-detect.html", std::bind(&IskakINO_WifiPortal::handleRoot, this));     // Apple/iOS
        _server->on("/library/test/success.html", std::bind(&IskakINO_WifiPortal::handleRoot, this)); // Apple/macOS
        _server->on("/connecttest.txt", std::bind(&IskakINO_WifiPortal::handleRoot, this));         // Windows
        _server->on("/ncsi.txt", std::bind(&IskakINO_WifiPortal::handleRoot, this));                // Windows
        _server->on("/fwlink", std::bind(&IskakINO_WifiPortal::handleRoot, this));                  // Windows

        if (_otaEnabled) {
            _server->on("/update", HTTP_GET, std::bind(&IskakINO_WifiPortal::handleOTA, this));
            _server->on("/update", HTTP_POST, [&]() {
                _server->sendHeader("Connection", "close");
                _server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
                ESP.restart();
            }, [&]() {
                HTTPUpload& upload = _server->upload();
                if (upload.status == UPLOAD_FILE_START) {
                    #if defined(ISKAKINO_PLATFORM_ESP32)
                        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
                    #elif defined(ISKAKINO_PLATFORM_ESP8266)
                        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                        if (!Update.begin(maxSketchSpace)) Update.printError(Serial);
                    #endif
                } else if (upload.status == UPLOAD_FILE_WRITE) {
                    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) Update.printError(Serial);
                } else if (upload.status == UPLOAD_FILE_END) {
                    if (Update.end(true)) Serial.println("OTA Success");
                }
            });
        }

        // Fitur Reboot lewat Web (dilindungi PIN kalau di-set lewat setAdminPin())
        _server->on("/reboot", [this]() {
            if (!checkAdminPin()) {
                _server->send(401, "text/plain", "Unauthorized: PIN salah atau tidak disertakan.");
                return;
            }
            _server->send(200, "text/html", "<html><body style='background:#1a1a1a;color:#eee;text-align:center;padding-top:50px;'><h2>Restarting...</h2><p>Please wait a few seconds.</p></body></html>");
            delay(1000);
            ESP.restart();
        });

        // Fitur Reset/Clear Data lewat Web (dilindungi PIN kalau di-set lewat setAdminPin())
        _server->on("/clear", [this]() {
            if (!checkAdminPin()) {
                _server->send(401, "text/plain", "Unauthorized: PIN salah atau tidak disertakan.");
                return;
            }
            resetSettings();
        });

        _server->onNotFound(std::bind(&IskakINO_WifiPortal::handleRoot, this));
        _server->begin();
    }

    _portalActive = true;
    _scheduler.reset(ISKAKINO_SCHED_PORTAL_TIMEOUT); // baseline timeout portal, ganti _portalStartTime lama
    _state = IskakPortalState::PORTAL;
    _logger.log(F("[IskakINO] Portal Aktif di 192.168.4.1"));
}

String IskakINO_WifiPortal::htmlEscape(const String& raw) {
    String out;
    out.reserve(raw.length());
    for (unsigned int i = 0; i < raw.length(); i++) {
        char c = raw[i];
        switch (c) {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&#39;";  break;
            default:   out += c;        break;
        }
    }
    return out;
}

String IskakINO_WifiPortal::jsEscape(const String& raw) {
    String out;
    out.reserve(raw.length());
    for (unsigned int i = 0; i < raw.length(); i++) {
        char c = raw[i];
        if (c == '\\' || c == '\'') out += '\\';
        if (c == '\n' || c == '\r') continue;
        out += c;
    }
    return out;
}

void IskakINO_WifiPortal::handleSave() {
    String ssid = _server->arg("s");
    String pass = _server->arg("p");

    for (int i = 0; i < _paramCount; i++) {
        String val = _server->arg(_params[i]->id);
        int maxLen = _params[i]->length;
        if (maxLen > 0) {
            int copyLen = val.length();
            if (copyLen > maxLen - 1) copyLen = maxLen - 1;
            memcpy(_params[i]->value, val.c_str(), copyLen);
            _params[i]->value[copyLen] = '\0';
        }
    }
    saveParams();

    if (ssid.length() > 0) {
        mergeWifiCredential(ssid.c_str(), pass.c_str());
    }

    _server->send(200, "text/html", "Saved! Restarting...");
    delay(2000);
    ESP.restart();
}

void IskakINO_WifiPortal::tick() {
    // 1. Jalankan state machine koneksi (scan/connect kandidat WiFi secara bertahap)
    pumpStateMachine();

    // 2. Tangani DNS Server (Captive Portal)
    if (_portalActive) {
        _dnsServer.processNextRequest();
    }

    // 3. Tangani Web Server
    if (_server != nullptr) {
        _server->handleClient();
    }

    // 4. Fitur Portal Timeout: tutup portal & restart jika waktu habis
    if (_portalActive && _timeout > 0) {
        if (_scheduler.once((unsigned long)_timeout * 1000UL, ISKAKINO_SCHED_PORTAL_TIMEOUT)) {
            _logger.log(F("[IskakINO] Waktu portal habis. Restarting..."));
            delay(200);
            ESP.restart();
        }
    }

    // 5. Fitur Auto-Reconnect & Failover (setiap 30 detik), hanya saat sudah pernah CONNECTED
    if (_state == IskakPortalState::CONNECTED && _scheduler.every(30000, ISKAKINO_SCHED_WIFI_CHECK)) {
        if (WiFi.status() != WL_CONNECTED) {
            _logger.log(F("[IskakINO] WiFi terputus! Mencoba menghubungkan kembali..."));
            WiFi.reconnect();
            _reconnectAttempts++;

            if (_reconnectAttempts >= _maxReconnectAttempts) {
                _logger.log(F("[IskakINO] Gagal reconnect ke SSID aktif. Memulai failover scan ke profil tersimpan lain..."));
                _reconnectAttempts = 0;
                startScan();
            }
        } else {
            _reconnectAttempts = 0;
        }
    }
}

void IskakINO_WifiPortal::handleRoot() {
    // 1. SCANNING WIFI (untuk ditampilkan sebagai daftar pilihan di form)
    int n = WiFi.scanNetworks();
    String wifiList = "";

    if (n == 0) {
        wifiList = "<p style='color:#888;'>No networks found. Refresh to try again.</p>";
    } else {
        for (int i = 0; i < n; ++i) {
            String ssid = WiFi.SSID(i);
            String ssidJs = jsEscape(ssid);
            String ssidHtml = htmlEscape(ssid);
            wifiList += "<div class='nw' onclick=\"fillSSID('" + ssidJs + "')\">";
            wifiList += "<span>" + ssidHtml + "</span>";
            wifiList += "<span class='rssi'>" + String(WiFi.RSSI(i)) + " dBm</span>";
            wifiList += "</div>";
        }
    }

    // 2. SAVED PROFILES LIST
    String savedList = "";
    bool pinRequired = (_adminPin[0] != '\0');

    if (_wifiCount == 0) {
        savedList = "<p style='color:#888;font-size:0.85em;padding:6px 0;'>Belum ada profil WiFi tersimpan.</p>";
    } else {
        for (int i = 0; i < _wifiCount; i++) {
            String s = htmlEscape(String(_wifiList[i].ssid));
            String sJs = jsEscape(String(_wifiList[i].ssid));
            savedList += "<div class='saved-item'>";
            savedList += "<span>" + s + "</span>";
            if (pinRequired) {
                savedList += "<button type='button' class='del-btn' onclick=\"var p=prompt('Masukkan PIN Admin:'); if(p!=null) location.href='/delete_wifi?s=" + sJs + "&pin='+encodeURIComponent(p)\" title='Hapus Profil'>&times;</button>";
            } else {
                savedList += "<button type='button' class='del-btn' onclick=\"if(confirm('Hapus WiFi " + sJs + "?')) location.href='/delete_wifi?s=" + sJs + "'\" title='Hapus Profil'>&times;</button>";
            }
            savedList += "</div>";
        }
    }

    // 3. SYSTEM INFO
    unsigned long sec = millis() / 1000;
    unsigned long minutes = sec / 60;
    unsigned long hr = minutes / 60;
    String uptime = String(hr) + "h " + String(minutes % 60) + "m " + String(sec % 60) + "s";
    String freeHeap = String(ESP.getFreeHeap() / 1024) + " KB";
    String ipAddr = WiFi.localIP().toString();
    if (ipAddr == "0.0.0.0") ipAddr = "Disconnected";

    // 4. HTML & CSS
    String html = "<html><head><title>IskakINO Config</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body{background:#1a1a1a;color:#eee;font-family:sans-serif;text-align:center;padding:20px;}";
    html += ".card{background:#252525;padding:20px;border-radius:10px;box-shadow:0 4px 10px rgba(0,0,0,0.5);max-width:400px;margin:0 auto;}";
    html += "h2{color:#00d1b2;} h3{font-size:1.1em;color:#888;text-align:left;margin:15px 0 5px 0;}";
    html += ".net-box{margin-bottom:20px;max-height:150px;overflow-y:auto;background:#111;border-radius:5px;border:1px solid #444;}";
    html += ".saved-box{margin-bottom:20px;background:#111;border-radius:5px;border:1px solid #444;padding:4px 8px;}";
    html += ".saved-item{display:flex;justify-content:space-between;align-items:center;padding:8px 6px;border-bottom:1px solid #282828;font-size:0.9em;}";
    html += ".saved-item:last-child{border-bottom:none;}";
    html += ".del-btn{width:auto!important;margin:0!important;padding:2px 8px!important;background:#e74c3c!important;font-size:1em;line-height:1;border-radius:3px;}";
    html += ".del-btn:hover{background:#c0392b!important;}";
    html += ".nw{display:flex;justify-content:space-between;padding:12px;border-bottom:1px solid #333;cursor:pointer;font-size:0.9em;}";
    html += ".nw:hover{background:#00d1b2;color:#fff;} .rssi{color:#666;font-size:0.8em;}";
    html += ".dash{background:#111;padding:10px;border-radius:5px;border-left:4px solid #00d1b2;text-align:left;font-size:0.85em;margin-top:20px;}";
    html += ".dash div{display:flex;justify-content:space-between;margin:3px 0;}";
    html += ".label{color:#888;} .val{color:#00d1b2;font-weight:bold;}";
    html += "input{width:100%;padding:12px;margin:10px 0;border-radius:5px;border:1px solid #444;background:#333;color:#fff;box-sizing:border-box;}";
    html += "button{width:100%;padding:12px;background:#00d1b2;border:none;color:#fff;font-weight:bold;border-radius:5px;cursor:pointer;margin-top:10px;}";
    html += ".footer{margin-top:20px;font-size:0.8em;color:#555;}";
    html += "</style>";
    html += "<script>function fillSSID(s){document.getElementsByName('s')[0].value=s;document.getElementsByName('p')[0].focus();}</script>";
    html += "</head><body><div class='card'><h2>" + htmlEscape(String(_brandName)) + "</h2>";

    html += "<h3>Saved WiFi Profiles</h3><div class='saved-box'>" + savedList + "</div>";

    html += "<h3>Select Network</h3><div class='net-box'>" + wifiList + "</div>";

    html += "<form action='/save' method='POST'><h3>Add / Connect WiFi</h3>";
    html += "<input name='s' placeholder='SSID' required>";
    html += "<input name='p' type='password' placeholder='Password'>";

    if (_paramCount > 0) {
        html += "<h3>Custom Parameters</h3>";
        for (int i = 0; i < _paramCount; i++) {
            html += "<input name='" + String(_params[i]->id) + "' placeholder='" + htmlEscape(String(_params[i]->label)) + "' value='" + htmlEscape(String(_params[i]->value)) + "'>";
        }
    }
    html += "<button type='submit'>SAVE & CONNECT</button></form>";
    if (_otaEnabled) html += "<br><a href='/update' style='color:#555;font-size:0.8em;'>Firmware Update</a>";

    html += "<h3>Device Status</h3><div class='dash'>";
    html += "<div><span class='label'>IP Address:</span><span class='val'>" + ipAddr + "</span></div>";
    html += "<div><span class='label'>Uptime:</span><span class='val'>" + uptime + "</span></div>";
    html += "<div><span class='label'>Free RAM:</span><span class='val'>" + freeHeap + "</span></div></div>";

    html += "<h3>Actions</h3><div style='display:flex; gap:10px;'>";
    if (pinRequired) {
        html += "<button onclick=\"var pin=prompt('Masukkan PIN Admin:'); if(pin!=null && confirm('Restart device?')) location.href='/reboot?pin='+encodeURIComponent(pin)\" style='background:#f39c12;'>RESTART</button>";
        html += "<button onclick=\"var pin=prompt('Masukkan PIN Admin:'); if(pin!=null && confirm('Clear all settings?')) location.href='/clear?pin='+encodeURIComponent(pin)\" style='background:#e74c3c;'>RESET INFO</button>";
    } else {
        html += "<button onclick=\"if(confirm('Restart device?')) location.href='/reboot'\" style='background:#f39c12;'>RESTART</button>";
        html += "<button onclick=\"if(confirm('Clear all settings?')) location.href='/clear'\" style='background:#e74c3c;'>RESET INFO</button>";
    }
    html += "</div><div class='footer'>IskakINO_WifiPortal v1.2.0</div></div></body></html>";

    if (_server) {
        _server->send(200, "text/html", html);
    }
}

void IskakINO_WifiPortal::handleOTA() {
    String html = "<html><body><form method='POST' action='/update' enctype='multipart/form-data'>";
    html += "<h2>OTA Update</h2><input type='file' name='update'><button type='submit'>Update</button></form></body></html>";
    _server->send(200, "text/html", html);
}

void IskakINO_WifiPortal::resetSettings() {
    #if defined(ISKAKINO_PLATFORM_ESP32)
        Preferences prefs; prefs.begin(ISKAKINO_PREFS_NS, false); prefs.clear(); prefs.end();
        Preferences prefsP; prefsP.begin(ISKAKINO_PARAM_NS, false); prefsP.clear(); prefsP.end();
        Preferences prefsW; prefsW.begin(ISKAKINO_WIFI_NS, false); prefsW.clear(); prefsW.end();
    #elif defined(ISKAKINO_PLATFORM_ESP8266)
        LittleFS.begin();
        LittleFS.remove(ISKAKINO_LEGACY_FILE);
        LittleFS.remove(ISKAKINO_PARAM_FILE);
        LittleFS.remove(ISKAKINO_WIFI_FILE);
    #endif
    ESP.restart();
}

#endif // defined(ISKAKINO_HAS_WIFI)
