/************************************************************
 * PROJECT    : 01 - IskakINO Smart Fitting Lamp
 * VERSION    : v1.3.6 (Production Ready)
 * AUTHOR     : iskakfatoni
 * DATE       : 2026-02-16
 * BOARD      : ESP32 / ESP8266 (Hybrid)
 * * DEPENDENCIES: IskakINO_ArduFast, IskakINO_WifiPortal, 
 * IskakINO_FastNTP, IskakINO_Storage,
 * IskakINO_LiquidCrystal_I2C
 ************************************************************/

#include <IskakINO_ArduFast.h>      // Global ArduFast instance
#include <IskakINO_WifiPortal.h>    // WiFi & WebServer handler
#include <IskakINO_FastNTP.h>       // Non-blocking NTP sync
#include <IskakINO_Storage.h>       // Hybrid storage with CRC32
#include <IskakINO_LiquidCrystal_I2C.h>
#include <WiFiUdp.h>

// ================== STRUKTUR DATA (PRODUCT LEVEL) ==================
struct ConfigData {
  bool lampState;
  uint32_t totalOnTime; // Menghitung total waktu lampu menyala (menit)
  uint16_t bootCount;
};

ConfigData mySettings;
const int RELAY_PIN = 5;

// ================== OBJECTS ==================
IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org"); //
LiquidCrystal_I2C lcd(16, 2); //

// ================== LOGIKA KONTROL ==================
void updateLamp(bool state) {
  mySettings.lampState = state;
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  IskakStorage.save(0, mySettings); // Simpan otomatis dengan CRC32
  
  if (lcd.isConnected()) {
    lcd.setCursor(0, 1);
    lcd.print(state ? "Status: ON " : "Status: OFF");
  }
  ArduFast.log(F("Lamp State Changed"), state); //
}

// ================== SETUP ==================
void setup() {
  // 1. Inisialisasi ArduFast (Serial 115200)
  ArduFast.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);

  // 2. Inisialisasi Storage & Load Data
  IskakStorage.begin("smart_fitting", true);
  if (!IskakStorage.load(0, mySettings)) {
    // Factory Reset / Data Baru
    mySettings.lampState = false;
    mySettings.totalOnTime = 0;
    mySettings.bootCount = 0;
  }
  mySettings.bootCount++;
  IskakStorage.save(0, mySettings);

  // 3. Hardware Feedback
  lcd.begin();
  lcd.backlight();
  lcd.printCenter("IskakINO Smart", 0);
  updateLamp(mySettings.lampState); // Restore state terakhir

  // 4. Portal & Network
  portal.setBrandName("IskakINO Fitting");
  portal.begin("SmartFitting-Setup"); 
  ntp.begin(25200); // GMT+7
}

// ================== LOOP (NON-BLOCKING) ==================
void loop() {
  portal.handle(); // Handle Web Server & DNS Portal
  ntp.update();   // State machine update NTP

  // Timer 1: Update Jam di LCD setiap 1 detik
  if (ArduFast.every(1000, 0)) {
    if (lcd.isConnected() && ntp.isTimeSet()) {
      lcd.setCursor(0, 0);
      lcd.print(ntp.getFormattedTime());
    }
  }

  // Timer 2: Autosave & Waktu Nyala (setiap 1 menit)
  if (ArduFast.every(60000, 1)) {
    if (mySettings.lampState) {
      mySettings.totalOnTime++;
      IskakStorage.save(0, mySettings);
    }
  }

  // Timer 3: Cek Koneksi & Re-sync NTP (setiap 1 jam)
  if (ArduFast.every(3600000, 2)) {
    if (WiFi.status() == WL_CONNECTED) {
      ntp.forceUpdate(); //
    }
  }
}
