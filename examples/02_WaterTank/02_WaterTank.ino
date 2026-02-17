/************************************************************
 * PROJECT    : Water Tank Control IskakINO
 * BOARD      : ESP32-C3
 * VERSION    : v1.5.1 (Smart Calibration Enabled)
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
  int minLevel;      
  int maxLevel;      
  int tankHeight;    // Tinggi tangki total (cm)
  bool autoMode;     
  uint32_t pumpRuns; 
};
TankConfig settings;

// ================== PIN & GLOBALS ==================
#define PUMP_RELAY 5
#define TRIG_PIN   12
#define ECHO_PIN   13

unsigned long lastTransitionMs = 0;
const unsigned long MIN_INTERVAL = 5000; 
int currentLevel = 0;

IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org");
LiquidCrystal_I2C lcd(16, 2);

// ================== CORE FUNCTION (SMOOTH TRANSITION) ==================
void updatePump(bool state, bool force = false) {
  if (!force && (millis() - lastTransitionMs < MIN_INTERVAL)) {
    ArduFast.log("Safety", "Blocked: Delay active");
    return;
  }
  digitalWrite(PUMP_RELAY, state ? HIGH : LOW);
  lastTransitionMs = millis();
  IskakStorage.save(0, settings); 
}

// ================== SENSOR LOGIC ==================
int readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  int distance = duration * 0.034 / 2;
  
  if (distance <= 0 || distance > settings.tankHeight) return 0;
  
  // Hitung Level: Jarak 10cm dari sensor = 100%, Jarak = tankHeight = 0%
  int level = map(distance, settings.tankHeight, 10, 0, 100);
  return constrain(level, 0, 100);
}

// ================== DASHBOARD UI (With Calibration) ==================
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>WaterTank C3</title>
<style>
  body{font-family:sans-serif; background:#0d1b2a; color:#e0e1dd; text-align:center; padding:10px;}
  .card{background:#1b263b; padding:15px; border-radius:15px; margin-bottom:10px; box-shadow:0 4px 10px rgba(0,0,0,0.5);}
  .tank-visual{width:80px; height:120px; border:3px solid #778da9; margin:15px auto; position:relative; border-radius:0 0 10px 10px; overflow:hidden;}
  .water{background:#415a77; position:absolute; bottom:0; width:100%; transition:height 1s;}
  .btn{padding:10px; border:none; border-radius:8px; font-weight:bold; cursor:pointer; width:100%; margin:5px 0; background:#e0e1dd; color:#0d1b2a;}
  input{width:50px; padding:5px; border-radius:5px; border:none; text-align:center;}
</style>
</head><body>
  <h2>🚰 IskakINO WaterTank</h2>
  <div class="card">
    <div class="tank-visual"><div id="wb" class="water" style="height:0%;"></div></div>
    <h1 id="lv">0%</h1>
    <button class="btn" onclick="fetch('/toggle')">PUMP ON/OFF</button>
  </div>
  
  <div class="card">
    <h3>⚙️ Calibration</h3>
    <form action="/calibrate">
      Tank H: <input type="number" name="th" id="ith"> cm<br><br>
      Min: <input type="number" name="min" id="imin"> % | 
      Max: <input type="number" name="max" id="imax"> %<br><br>
      <button type="submit" class="btn" style="background:#415a77; color:white;">APPLY SETTINGS</button>
    </form>
  </div>

  <div class="card" style="font-size:0.8em; text-align:left;">
    WiFi: <span id="rssi">--</span> | Time: <span id="tm">--</span> | Count: <span id="pc">--</span>
  </div>

  <script>
    setInterval(() => {
      fetch('/status').then(r=>r.json()).then(d=>{
        document.getElementById('wb').style.height = d.l + '%';
        document.getElementById('lv').innerText = d.l + '%';
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

  IskakStorage.begin("watertank", true);
  if (!IskakStorage.load(0, settings)) {
    settings = {20, 90, 200, true, 0}; // Default tank height 200cm
  }

  portal.on("/", []() { portal.send(200, "text/html", DASHBOARD_HTML); });
  
  portal.on("/status", []() {
    String j = "{\"l\":"+String(currentLevel)+",\"r\":"+String(WiFi.RSSI())+",\"t\":\""+ntp.getFormattedTime()+"\",\"c\":"+String(settings.pumpRuns)+"}";
    portal.send(200, "application/json", j);
  });

  portal.on("/calibrate", []() {
    settings.tankHeight = portal.arg("th").toInt();
    settings.minLevel = portal.arg("min").toInt();
    settings.maxLevel = portal.arg("max").toInt();
    IskakStorage.save(0, settings);
    portal.send(200, "text/html", "<script>alert('Calibrated!'); location.href='/';</script>");
  });

  portal.on("/toggle", []() { 
    updatePump(!digitalRead(PUMP_RELAY)); 
    portal.send(200, "text/plain", "OK"); 
  });

  portal.begin("WaterTank-Setup");
  ntp.begin(25200);
  lcd.begin();
  updatePump(false, true); 
}

void loop() {
  portal.handle();
  ntp.update();

  if (ArduFast.every(2000, 0)) {
    currentLevel = readUltrasonic();
    if (settings.autoMode) {
      if (currentLevel <= settings.minLevel && digitalRead(PUMP_RELAY) == LOW) {
        updatePump(true);
        settings.pumpRuns++;
      } 
      else if (currentLevel >= settings.maxLevel && digitalRead(PUMP_RELAY) == HIGH) {
        updatePump(false);
      }
    }
  }

  if (ArduFast.every(1000, 1)) {
    if (lcd.isConnected()) {
      lcd.setCursor(0, 0); lcd.print("Lvl:"); lcd.print(currentLevel); lcd.print("% ");
      lcd.setCursor(11, 0); lcd.print(ntp.getFormattedTime().substring(0,5));
    }
  }
}
