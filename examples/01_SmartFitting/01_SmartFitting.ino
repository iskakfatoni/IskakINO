#include <IskakINO_ArduFast.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_Storage.h> 
#include <IskakINO_LiquidCrystal_I2C.h>
#include <WiFiUdp.h>

// Struktur Data Produk
struct ConfigData {
  bool lampState;
  uint16_t bootCount;
};

ConfigData mySettings;

// Inisialisasi Objects
IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org");
LiquidCrystal_I2C lcd(16, 2);

void updateRelay(bool state) {
  mySettings.lampState = state;
  digitalWrite(5, state ? HIGH : LOW);
  IskakStorage.save(0, mySettings); // Simpan ke Storage Hybrid
}

void setup() {
  ArduFast.begin(115200);
  
  // 1. Load Storage (Hybrid Engine: Preferences/LittleFS/EEPROM)
  IskakStorage.begin("smartfit", true);
  if (!IskakStorage.load(0, mySettings)) {
    mySettings.lampState = false;
    mySettings.bootCount = 0;
  }
  mySettings.bootCount++;
  IskakStorage.save(0, mySettings);

  // 2. Restore Hardware State
  pinMode(5, OUTPUT);
  updateRelay(mySettings.lampState);

  // 3. Network & LCD
  lcd.begin();
  lcd.backlight();
  portal.begin("IskakINO-SmartFitting");
  ntp.begin(25200); // GMT+7
}

void loop() {
  portal.handle(); // Web Server & DNS
  ntp.update();   // State Machine NTP

  // Logika 1: Tampilkan Jam di LCD (Setiap 1 Detik)
  if (ArduFast.every(1000, 0)) {
    if (lcd.isConnected() && ntp.isTimeSet()) {
      lcd.setCursor(0, 0);
      lcd.print(ntp.getFormattedTime());
    }
  }

  // Logika 2: Maintenance Berkala (Setiap 1 Jam)
  if (ArduFast.every(3600000, 1)) {
    if (WiFi.status() == WL_CONNECTED) {
      ntp.forceUpdate(); // Sekarang tidak akan error lagi
    }
  }
}
