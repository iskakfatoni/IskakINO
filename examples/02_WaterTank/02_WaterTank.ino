/************************************************************
 * PROJECT    : Water Tank Control IskakINO
 * BOARD      : ESP32-C3
 * VERSION    : v1.5.0 (Optimized Ecosystem)
 * AUTHOR     : iskakfatoni
 ************************************************************/

#include <IskakINO_ArduFast.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_Storage.h>
#include <IskakINO_LiquidCrystal_I2C.h>
#include <WiFiUdp.h>

// ================== STRUKTUR DATA (CRC32 Protected) ==================
struct TankConfig {
  int minLevel;      // Batas bawah (%) untuk ON
  int maxLevel;      // Batas atas (%) untuk OFF
  bool autoMode;     // Auto/Manual
  uint32_t pumpRuns; // Statistik
};
TankConfig settings;

// ================== PIN & GLOBALS ==================
#define PUMP_RELAY 5
#define TRIG_PIN   12
#define ECHO_PIN   13
#define TANK_HEIGHT 200 // Tinggi tangki (cm)

unsigned long lastTransitionMs = 0;
const unsigned long MIN_INTERVAL = 5000; // Proteksi pompa (5 detik)
int currentLevel = 0;

IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org");
LiquidCrystal_I2C lcd(16, 2);

// ================== CORE FUNCTION (SMOOTH TRANSITION) ==================
void updatePump(bool state, bool force = false) {
  // Proteksi agar pompa tidak ON-OFF terlalu cepat (Mencegah kerusakan motor)
  if (!force && (millis() - lastTransitionMs < MIN_INTERVAL)) {
    ArduFast.log("Safety", "Pump transition blocked: Too frequent!");
    return;
  }

  digitalWrite(PUMP_RELAY, state ? HIGH : LOW);
  lastTransitionMs = millis();
  
  if(lcd.isConnected()) {
    lcd.setCursor(0, 1);
    lcd.print(state ? "PUMP: RUNNING   " : "PUMP: STOPPED   ");
  }
  ArduFast.log("Pump", state ? "Activated" : "Deactivated");
}

// ================== SENSOR LOGIC (NON-BLOCKING) ==================
int readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Timeout 30ms
  int distance = duration * 0.034 / 2;
  
  if (distance <= 0 || distance > TANK_HEIGHT) return 0;
  int level = map(distance, TANK_HEIGHT, 10, 0, 100);
  return constrain(level, 0, 100);
}

// ================== DASHBOARD UI ==================
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>WaterTank C3</title>
<style>
  body{font-family:sans-serif; background:#0d1b2a; color:#e0e1dd; text-align:center; padding:20px;}
  .card{background:#1b263b; padding:20px; border-radius:15px; margin-bottom:15px; box-shadow:0 4px 10px rgba(0,0,0,0.5);}
  .tank-visual{width:100px; height:150px; border:3px solid #778da9; margin:20px auto; position:relative; border-radius:0 0 10px 10px; overflow:hidden;}
  .water{background:#415a77; position:absolute; bottom:0; width:100%; transition:height 1s;}
  .btn{padding:12px; border:none; border-radius:8px; font-weight:bold; cursor:pointer; width:100%; margin:5px 0; background:#e0e1dd; color:#0d1b2a;}
</style>
</head><body>
  <h2>🚰 IskakINO WaterTank</h2>
  <div class="card">
    <div class="tank-visual"><div id="wb" class="water" style="height:0%;"></div></div>
    <h1 id="lv">0%</h1>
    <p id="ps">Status: --</p>
    <button class="btn" onclick="fetch('/toggle')">MANUAL TOGGLE</button>
  </div>
  <div class="card" style="font-size:0.8em; text-align:left;">
    WiFi: <span id="rssi">--</span> | Time: <span id="tm">--</span><br>
    Pump Count: <span id="pc">--</span>
  </div>
  <script>
    setInterval(() => {
      fetch('/status').then(r=>r.json()).then(d=>{
        document.getElementById('wb').style.height = d.l + '%';
        document.getElementById('lv').innerText = d.l + '%';
        document.getElementById('ps').innerText = d.p ? 'PUMP: ON' : 'PUMP: OFF';
        document.getElementById('rssi').innerText = d.r + 'dBm';
        document.getElementById('tm').innerText = d.t;
        document.getElementById('pc').innerText = d.c;
      });
    }, 2000);
  </script>
</body></html>
)rawliteral";

void setup() {
  ArduFast.begin(115200);
  pinMode(PUMP_RELAY, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // 1. STORAGE
  IskakStorage.begin("watertank", true);
  if (!IskakStorage.load(0, settings)) {
    settings = {20, 90, true, 0}; // Default: ON <20%, OFF >90%
  }

  // 2. PORTAL & ROUTES
  portal.setBrandName("IskakINO WaterTank");
  portal.on("/", []() { portal.send(200, "text/html", DASHBOARD_HTML); });
  portal.on("/status", []() {
    String j = "{\"l\":"+String(currentLevel)+",\"p\":"+String(digitalRead(PUMP_RELAY))+",\"r\":"+String(WiFi.RSSI())+",\"t\":\""+ntp.getFormattedTime()+"\",\"c\":"+String(settings.pumpRuns)+"}";
    portal.send(200, "application/json", j);
  });
  portal.on("/toggle", []() { 
    bool currentState = digitalRead(PUMP_RELAY);
    updatePump(!currentState); 
    portal.send(200, "text/plain", "OK"); 
  });

  portal.begin("WaterTank-Setup");
  ntp.begin(25200);
  
  lcd.begin();
  lcd.printCenter("WaterTank C3", 0);
  updatePump(false, true); // Safety start
}

void loop() {
  portal.handle();
  ntp.update();

  // TUGAS 1: Pembacaan Sensor & Otomasi (Tiap 2 Detik)
  if (ArduFast.every(2000, 0)) {
    currentLevel = readUltrasonic();
    
    if (settings.autoMode) {
      if (currentLevel <= settings.minLevel && digitalRead(PUMP_RELAY) == LOW) {
        updatePump(true);
        settings.pumpRuns++;
        IskakStorage.save(0, settings);
      } 
      else if (currentLevel >= settings.maxLevel && digitalRead(PUMP_RELAY) == HIGH) {
        updatePump(false);
      }
    }
  }

  // TUGAS 2: LCD Maintenance (Tiap 1 Detik)
  if (ArduFast.every(1000, 1)) {
    if (lcd.isConnected()) {
      lcd.setCursor(0, 0);
      lcd.print("Lvl:"); lcd.print(currentLevel); lcd.print("%  ");
      lcd.setCursor(11, 0); lcd.print(ntp.getFormattedTime().substring(0,5));
      
      // Indikator Smooth Transition
      lcd.setCursor(15, 1);
      lcd.print((millis() - lastTransitionMs < MIN_INTERVAL) ? "!" : " ");
    }
  }
}
