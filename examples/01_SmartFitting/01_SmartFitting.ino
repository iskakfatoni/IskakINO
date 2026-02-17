/************************************************************
 * PROJECT    : Smart Fitting Lamp IskakINO
 * BOARD      : ESP32-C3
 * VERSION    : v1.5.0 (Optimized with IskakINO Ecosystem)
 * AUTHOR     : iskakfatoni
 ************************************************************/

#include <IskakINO_ArduFast.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_Storage.h>
#include <IskakINO_LiquidCrystal_I2C.h>
#include <WiFiUdp.h>

// ================== STRUKTUR DATA (Preferences + CRC32) ==================
struct ConfigData {
  bool lampState;
  int onHour, onMin;   // Jadwal ON
  int offHour, offMin; // Jadwal OFF
  uint16_t bootCount;
};

ConfigData settings;

// ================== PIN & OBJECTS ==================
#define RELAY_PIN 5
IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org");
LiquidCrystal_I2C lcd(16, 2); // Opsional jika fisik ada

const char* FW_VERSION = "v1.5.0-IskakINO";

// ================== DASHBOARD UI (ACTIVE) ==================
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Smart Fitting C3</title>
<style>
  body{font-family:sans-serif; background:#121212; color:#eee; text-align:center; padding:20px;}
  .card{background:#1e1e1e; padding:20px; border-radius:15px; margin-bottom:15px; box-shadow:0 4px 10px rgba(0,0,0,0.3);}
  .btn{padding:15px 30px; border:none; border-radius:8px; font-weight:bold; cursor:pointer; width:100%; margin:5px 0;}
  .btn-toggle{background:#4CAF50; color:white;}
  .btn-reset{background:#f44336; color:white; font-size:12px;}
  input{padding:8px; border-radius:5px; border:1px solid #444; background:#333; color:white; width:60px; text-align:center;}
</style>
</head><body>
  <h2>💡 Smart Fitting IskakINO</h2>
  <div class="card">
    <h1 id="st">--</h1>
    <button class="btn btn-toggle" onclick="fetch('/toggle')">TOGGLE POWER</button>
  </div>
  <div class="card">
    <h3>Auto Schedule</h3>
    <form action="/setsched">
      ON  : <input type="number" name="onH" id="onH">:<input type="number" name="onM" id="onM"><br><br>
      OFF : <input type="number" name="offH" id="offH">:<input type="number" name="offM" id="offM"><br><br>
      <button type="submit" class="btn" style="background:#2196F3; color:white;">SIMPAN JADWAL</button>
    </form>
  </div>
  <div class="card" style="font-size:0.8em; text-align:left;">
    Uptime: <span id="up">--</span> | WiFi: <span id="rssi">--</span><br>
    Time: <span id="tm">--</span> | Boot: <span id="bt">--</span>
  </div>
  <button class="btn btn-reset" onclick="if(confirm('Reset WiFi?')) location.href='/resetwifi'">RESET WIFI</button>
  
  <script>
    setInterval(() => {
      fetch('/status').then(r=>r.json()).then(d=>{
        document.getElementById('st').innerText = d.s ? '💡 ON' : '⚫ OFF';
        document.getElementById('up').innerText = d.u;
        document.getElementById('rssi').innerText = d.r + 'dBm';
        document.getElementById('tm').innerText = d.t;
        document.getElementById('bt').innerText = d.b;
      });
    }, 2000);
  </script>
</body></html>
)rawliteral";

// ================== CORE FUNCTIONS ==================
void updateRelay(bool state) {
  settings.lampState = state;
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  IskakStorage.save(0, settings);
  if(lcd.isConnected()) {
    lcd.setCursor(0,1); lcd.print(state ? "STATUS: ON " : "STATUS: OFF");
  }
}

void setup() {
  ArduFast.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);

  // 1. STORAGE (Hybrid Preferences)
  IskakStorage.begin("smartfit", true);
  if (!IskakStorage.load(0, settings)) {
    settings = {false, 18, 0, 5, 0, 0}; // Default ON 18:00, OFF 05:00
  }
  settings.bootCount++;
  IskakStorage.save(0, settings);

  // 2. WIFI & PORTAL (Menggantikan WiFiManager + Captive Manual)
  portal.setBrandName("IskakINO Smart Fitting");
  
  // Custom Handlers
  portal.on("/", []() { portal.send(200, "text/html", DASHBOARD_HTML); });
  
  portal.on("/status", []() {
    String j = "{\"s\":"+String(settings.lampState)+",\"u\":\""+String(millis()/1000)+"s\",\"r\":"+String(WiFi.RSSI())+",\"t\":\""+ntp.getFormattedTime()+"\",\"b\":"+String(settings.bootCount)+"}";
    portal.send(200, "application/json", j);
  });

  portal.on("/toggle", []() { updateRelay(!settings.lampState); portal.send(200, "text/plain", "OK"); });

  portal.on("/setsched", []() {
    settings.onHour = portal.arg("onH").toInt(); settings.onMin = portal.arg("onM").toInt();
    settings.offHour = portal.arg("offH").toInt(); settings.offMin = portal.arg("offM").toInt();
    IskakStorage.save(0, settings);
    portal.send(200, "text/html", "<script>alert('Jadwal disimpan'); location.href='/';</script>");
  });

  portal.on("/resetwifi", []() {
    portal.send(200, "text/plain", "WiFi Reset... Restarting AP Mode");
    delay(1000); WiFi.disconnect(true, true); ESP.restart();
  });

  portal.begin("SmartFitting-Setup"); 

  // 3. NTP
  ntp.begin(25200); // GMT+7

  // 4. LCD & Self Test
  lcd.begin();
  lcd.printCenter("IskakINO C3", 0);
  updateRelay(settings.lampState); // Restore last state
}

void loop() {
  portal.handle();
  ntp.update();

  // LOGIKA PENJADWALAN (Tiap 30 Detik)
  if (ArduFast.every(30000, 0)) {
    if (ntp.isTimeSet()) {
      int h = ntp.getHours();
      int m = ntp.getMinutes();

      // Cek Jadwal ON
      if (h == settings.onHour && m == settings.onMin && !settings.lampState) {
        updateRelay(true);
        ArduFast.log("Schedule", "Auto ON triggered");
      }
      // Cek Jadwal OFF
      if (h == settings.offHour && m == settings.offMin && settings.lampState) {
        updateRelay(false);
        ArduFast.log("Schedule", "Auto OFF triggered");
      }
    }
  }

  // UPDATE LCD (Tiap 1 Detik)
  if (ArduFast.every(1000, 1)) {
    if (lcd.isConnected() && ntp.isTimeSet()) {
      lcd.setCursor(0, 0); lcd.print(ntp.getFormattedTime());
    }
  }
}
