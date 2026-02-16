/************************************************************
 * PROJECT    : 01 - IskakINO Smart Fitting Lamp
 * BOARD      : ESP32 & ESP8266 (Hybrid)
 * LIBRARIES  : ArduFast, WifiPortal, FastNTP, LiquidCrystal_I2C
 * AUTHOR     : iskakfatoni
 * VERSION    : v1.3.5
 * DATE       : 2026-02-16
 ************************************************************/

#include <IskakINO_ArduFast.h>      // Membawa instance ArduFast
#include <IskakINO_WifiPortal.h>    // Membawa IskakWebServer & WiFi
#include <IskakINO_FastNTP.h>       // Membawa IskakINO_FastNTP
#include <IskakINO_LiquidCrystal_I2C.h>
#include <WiFiUdp.h>

// --- HANDLE PREFERENCES (ESP32 Built-in, ESP8266 External) ---
#ifdef ESP32
  #include <Preferences.h>
#else
  #include <vshymanskyy/Preferences.h> 
#endif

// ================== PIN & KONFIGURASI ==========
#define RELAY_PIN 5
#define MAX_LOG 20

// ================== OBJECTS ==================
IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org");
LiquidCrystal_I2C lcd(16, 2);
Preferences prefs;

// ================== STATE ==================
bool lampState = false;

// ================== LOG SYSTEM ==================
void logEvent(String level, String msg) {
    int idx = prefs.getInt("logi", 0);
    String entry = "[" + ntp.getFormattedTime() + "] [" + level + "] " + msg;
    
    prefs.putString(("log" + String(idx)).c_str(), entry);
    prefs.putInt("logi", (idx + 1) % MAX_LOG);
    
    // Gunakan fungsi log dari ArduFast
    ArduFast.log(F("Log Update"), idx);
    Serial.println(entry);
}

// ================== RELAY CONTROL ================
void setLamp(bool state, String reason) {
    lampState = state;
    digitalWrite(RELAY_PIN, lampState ? HIGH : LOW);
    prefs.putBool("lamp", lampState);
    logEvent("INFO", reason);
    
    // Update LCD jika terkoneksi
    if(lcd.isConnected()) {
        lcd.setCursor(0, 1);
        lcd.print("Lamp: ");
        lcd.print(state ? "ON " : "OFF");
    }
}

// ================== SETUP ========================
void setup() {
    // ArduFast.begin sudah menghandle Serial.begin(115200)
    ArduFast.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);

    // 1. Inisialisasi LCD
    lcd.begin();
    lcd.backlight();
    lcd.printCenter("IskakINO v1.0", 0);

    // 2. Storage
    prefs.begin("smartfit", false);
    lampState = prefs.getBool("lamp", false);
    digitalWrite(RELAY_PIN, lampState ? HIGH : LOW);

    // 3. WiFi Portal (Handle STA/AP & Web Server Internal)
    portal.setBrandName("IskakINO Smart Fitting");
    portal.begin("SmartFitting-Setup", "12345678");

    // 4. FastNTP (GMT+7)
    ntp.begin(25200);

    logEvent("INFO", "System Booted Successfully");
}

// ================== LOOP =========================
void loop() {
    // Handle WiFi Portal & Web Server internal
    portal.handle();
    
    // Handle background NTP Sync
    ntp.update();
    
    // Gunakan ArduFast 'every' untuk tugas periodik (Non-Blocking)
    // ID 0: Update Jam di LCD setiap 1 detik
    if (ArduFast.every(1000, 0)) {
        if(lcd.isConnected()) {
            lcd.setCursor(0, 0);
            lcd.print(ntp.getFormattedTime());
        }
    }
    
    // ID 1: Cek Koneksi WiFi setiap 30 detik
    if (ArduFast.every(30000, 1)) {
        if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP) {
            ArduFast.log(F("WiFi Connection Lost!"));
        }
    }
}
