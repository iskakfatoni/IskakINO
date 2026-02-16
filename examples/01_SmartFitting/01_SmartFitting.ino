/************************************************************
 * PROJECT    : 01 - IskakINO Smart Fitting Lamp
 * BOARD      : ESP32-C3
 * AUTHOR     : Iskak Fatoni
 * VERSION    : v1.3.5 (Refactored with IskakINO Libraries)
 * * DEPENDENCIES:
 * 1. IskakINO_ArduFast
 * 2. IskakINO_LiquidCrystal_I2C (Optional for Debug)
 * 3. IskakINO_WifiPortal
 * 4. IskakINO_FastNTP (Finalized 2026-02-16)
 ************************************************************/

#include <IskakINO_ArduFast.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_LiquidCrystal_I2C.h>

#ifdef ESP32
  #include <Preferences.h>
#else
  // Opsional: Untuk ESP8266 gunakan LittleFS atau library Preferences pihak ketiga
  // #include <LittleFS.h> 
#endif
#include <WebServer.h>

// ================== IDENTITAS ==================
const char* PROJECT_ID = "01 - IskakINO";
const char* FW_VERSION = "v1.3.5";

// ================== PIN & KONFIGURASI ==========
#define RELAY_PIN 5
#define MAX_LOG 20

// ================== OBJECTS ==================
IskakINO_WifiPortal portal("SmartFitting-Setup", "12345678");
IskakINO_FastNTP ntp;
IskakINO_ArduFast timer;
WebServer server(80);
Preferences prefs;

// ================== STATE ==================
bool lampState = false;
String currentDay = "";

// ================== LOG SYSTEM ==================
void logEvent(String level, String msg) {
    int idx = prefs.getInt("logi", 0);
    // Menggunakan IskakINO_FastNTP untuk timestamp
    String entry = "[" + ntp.getFormattedTime() + "] [" + level + "] " + msg;
    prefs.putString(("log" + String(idx)).c_str(), entry);
    prefs.putInt("logi", (idx + 1) % MAX_LOG);
    Serial.println(entry);
}

// ================== RELAY CONTROL ================
void setLamp(bool state, String reason) {
    lampState = state;
    digitalWrite(RELAY_PIN, lampState ? HIGH : LOW);
    prefs.putBool("lamp", lampState);
    logEvent("INFO", reason);
}

// ================== WEB HANDLERS =================
void handleRoot() {
    // Memanggil dashboard HTML (Tetap gunakan konstanta DASHBOARD_HTML Anda sebelumnya)
    // server.send_P(200, "text/html", DASHBOARD_HTML); 
}

void handleToggle() {
    setLamp(!lampState, "Web Toggle");
    server.send(200, "text/plain", "OK");
}

void handleSys() {
    // Menggunakan IskakINO_ArduFast untuk uptime
    String wifiStatus = WiFi.isConnected() ? String(WiFi.RSSI()) + " dBm" : "Offline";
    String json = "{";
    json += "\"wifi\":\"" + wifiStatus + "\",";
    json += "\"uptime\":\"" + timer.getUptimeString() + "\",";
    json += "\"mode\":\"" + String(WiFi.getMode() == WIFI_AP ? "AP" : "STA") + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

// ================== SETUP ========================
void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);

    // 1. Inisialisasi Preferences
    prefs.begin("smartfit", false);
    lampState = prefs.getBool("lamp", false);
    digitalWrite(RELAY_PIN, lampState ? HIGH : LOW);

    // 2. WiFi Portal (Otomatis Handle STA & AP)
    portal.begin();

    // 3. FastNTP (Finalized Version)
    ntp.begin();

    // 4. Web Server
    server.on("/", handleRoot);
    server.on("/toggle", handleToggle);
    server.on("/sys", handleSys);
    server.begin();

    logEvent("INFO", "System 01-IskakINO Booted");
}

// ================== LOOP =========================
void loop() {
    server.handleClient();
    ntp.update(); // Update waktu NTP secara background
    
    // Gunakan ArduFast untuk cek koneksi tiap 1 menit
    if (timer.isTimeUp(60000)) {
        if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP) {
            logEvent("WARN", "WiFi Lost, Retrying...");
        }
    }
}
