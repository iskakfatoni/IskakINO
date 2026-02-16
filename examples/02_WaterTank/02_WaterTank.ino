/*******************************************************************************
 * PROJECT 02: ISKAKINO WATER TANK CONTROL (IoT BASED)
 * VERSION    : v1.0.0 (Production Ready)
 * AUTHOR     : iskakfatoni
 * DATE       : 2026-02-16
 * BOARD      : ESP32 / ESP8266 (Hybrid)
 * * [LOGIKA KERJA]
 * 1. SENSOR  : Membaca jarak via Ultrasonik, dikonversi ke % (0-100%).
 * 2. OTOMASI : Pompa ON jika air <= minLevel, OFF jika >= maxLevel.
 * 3. STORAGE : Menyimpan ambang batas & statistik pumpRuns aman dengan CRC32.
 * 4. DASHBOARD: Antarmuka Web untuk monitoring & kontrol manual dari HP.
 *******************************************************************************/

#include <IskakINO_ArduFast.h>      // Task Scheduler
#include <IskakINO_WifiPortal.h>    // Web Server & WiFi
#include <IskakINO_FastNTP.h>       // Real Time Clock
#include <IskakINO_Storage.h>       // Safety Data Storage
#include <IskakINO_LiquidCrystal_I2C.h>
#include <WiFiUdp.h>

// ================== STRUKTUR DATA (SAVED IN STORAGE) ==================
struct TankConfig {
  int minLevel;       // Batas bawah pompa mulai nyala (%)
  int maxLevel;       // Batas atas pompa mati (%)
  bool autoMode;      // Mode Otomatis/Manual
  uint32_t pumpRuns;  // Counter berapa kali pompa bekerja
};

TankConfig settings;

// ================== KONFIGURASI PIN & HARDWARE ==================
#define PUMP_RELAY 5     // Pin Relay Pompa
#define TRIG_PIN   12    // Pin Trigger Ultrasonik
#define ECHO_PIN   14    // Pin Echo Ultrasonik
#define TANK_HEIGHT 200  // Tinggi tangki dalam CM (Sesuaikan!)

// ================== OBJECTS ==================
IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org");
LiquidCrystal_I2C lcd(16, 2);

int currentLevel = 0;    // Variabel level air global (%)
bool pumpActive = false; // Status pompa saat ini

// ================== DASHBOARD HTML (UI) ==================
const char DASHBOARD_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>IskakINO WaterTank</title>
<style>
  body { font-family: 'Segoe UI', sans-serif; text-align: center; background: #eceff1; color: #37474f; }
  .card { background: white; padding: 20px; border-radius: 15px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); display: inline-block; width: 90%; max-width: 400px; margin-top: 20px; }
  .tank { width: 120px; height: 180px; border: 4px solid #455a64; margin: 20px auto; position: relative; border-radius: 0 0 15px 15px; background: #fff; overflow: hidden; }
  .water { background: #0288d1; position: absolute; bottom: 0; width: 100%; transition: height 1s ease-in-out; }
  .btn { padding: 12px 25px; margin: 5px; border: none; border-radius: 8px; font-weight: bold; cursor: pointer; }
  .btn-on { background: #43a047; color: white; }
  .btn-off { background: #e53935; color: white; }
</style>
</head><body>
  <div class="card">
    <h2>Water Tank System</h2>
    <div class="tank"><div id="w" class="water" style="height: 0%;"></div></div>
    <h1 id="l">0%</h1>
    <p>Pump Status: <b id="s">OFF</b></p>
    <hr>
    <button class="btn btn-on" onclick="fetch('/p?s=1')">PUMP ON</button>
    <button class="btn btn-off" onclick="fetch('/p?s=0')">PUMP OFF</button>
  </div>
  <script>
    setInterval(() => {
      fetch('/status').then(r => r.json()).then(d => {
        document.getElementById('w').style.height = d.lv + '%';
        document.getElementById('l').innerText = d.lv + '%';
        document.getElementById('s').innerText = d.p ? 'ON' : 'OFF';
        document.getElementById('s').style.color = d.p ? '#43a047' : '#e53935';
      });
    }, 2000);
  </script>
</body></html>
)=====";

// ================== LOGIKA KONTROL POMPA ==================
void controlPump(bool state) {
  pumpActive = state;
  digitalWrite(PUMP_RELAY, state ? HIGH : LOW);
  ArduFast.log(F("Action"), state ? "Pump ON" : "Pump OFF");
}

// ================== PEMBACAAN SENSOR (NON-BLOCKING) ==================
int getWaterLevel() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 25000); // Max 25ms
  int dist = duration * 0.034 / 2;
  if (dist <= 0 || dist > TANK_HEIGHT) return 0;
  // Memetakan jarak ke persentase (Contoh: 180cm=0%, 10cm=100%)
  int p = map(dist, TANK_HEIGHT - 20, 10, 0, 100);
  return constrain(p, 0, 100);
}

// ================== SETUP ==================
void setup() {
  ArduFast.begin(115200);
  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // 1. Storage Recovery
  IskakStorage.begin("iskak_water", true);
  if (!IskakStorage.load(0, settings)) {
    settings = {20, 90, true, 0}; // Default: Min 20%, Max 90%
  }

  // 2. LCD & Portal
  lcd.begin();
  lcd.printCenter("IskakINO Water", 0);
  
  // Define Web Routes
  portal.on("/", []() { portal.send(200, "text/html", DASHBOARD_HTML); });
  portal.on("/status", []() {
    String j = "{\"lv\":"+String(currentLevel)+",\"p\":"+String(pumpActive)+"}";
    portal.send(200, "application/json", j);
  });
  portal.on("/p", []() { // Handle Manual Control dari Dashboard
    if(portal.hasArg("s")) controlPump(portal.arg("s") == "1");
    portal.send(200, "text/plain", "OK");
  });

  portal.begin("Iskak-WaterTank");
  ntp.begin(25200); // GMT+7
}

// ================== MAIN LOOP ==================
void loop() {
  portal.handle(); // Web Server
  ntp.update();    // Time Sync

  // TUGAS 1: Pembacaan Sensor & Logic (Setiap 3 Detik)
  if (ArduFast.every(3000, 0)) {
    currentLevel = getWaterLevel();
    
    // Logic Otomasi
    if (settings.autoMode) {
      if (currentLevel <= settings.minLevel && !pumpActive) {
        controlPump(true);
        settings.pumpRuns++;
        IskakStorage.save(0, settings); // Simpan statistik
      } 
      else if (currentLevel >= settings.maxLevel && pumpActive) {
        controlPump(false);
      }
    }
  }

  // TUGAS 2: Update LCD Display (Setiap 1 Detik)
  if (ArduFast.every(1000, 1)) {
    if (lcd.isConnected()) {
      lcd.setCursor(0, 0);
      lcd.print("Level: "); lcd.print(currentLevel); lcd.print("%  ");
      lcd.setCursor(0, 1);
      lcd.print(pumpActive ? "[RUNNING]" : "[STANDBY]");
      if(ntp.isTimeSet()) { lcd.setCursor(11, 1); lcd.print(ntp.getFormattedTime().substring(0,5)); }
    }
  }
}
