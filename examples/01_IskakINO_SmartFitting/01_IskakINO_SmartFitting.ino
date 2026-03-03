/************************************************************
 * PROJECT    : Smart Fitting Lamp IskakINO
 * VERSION    : v1.6.0 (Hostname & Auto-Reconnect)
 * BOARD      : ESP32-C3 / ESP8266
 * AUTHOR     : iskakfatoni
 ************************************************************/

#include <IskakINO_ArduFast.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_Storage.h>
#include <WiFiUdp.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

// ================== STRUKTUR DATA ==================
struct ConfigData {
  int onHour, onMin;
  int offHour, offMin;
  bool lampState;
  uint32_t bootCount;
};
ConfigData settings;

// ================== PIN & GLOBALS ==================
#define RELAY_PIN 5

IskakINO_ArduFast ArduFast;
IskakINO_Storage IskakStorage; //
IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org");

unsigned long lastTransitionMs = 0;
const unsigned long MIN_INTERVAL = 3000; 
const char* MY_HOSTNAME = "IskakINO-SmartLamp";

// ================== LOGIK RELAY + DEBUG ==================
void updateRelay(bool state, bool force = false) {
  unsigned long current = millis();
  
  if (!force && (current - lastTransitionMs < MIN_INTERVAL)) {
    Serial.println(F("[!]\tSafety Triggered: Debounce Relay aktif."));
    return;
  }
  
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  settings.lampState = state;
  lastTransitionMs = current;
  
  Serial.print(F("[>]\tRelay State: "));
  Serial.println(state ? F("NYALA") : F("MATI"));
  
  IskakStorage.save(0, settings);
}

// ================== DASHBOARD HTML ==================
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>IskakINO SmartLamp</title>
<style>
  body { font-family: sans-serif; background: #121212; color: #eee; text-align: center; padding: 20px; }
  .card { background: #1e1e1e; padding: 25px; border-radius: 20px; max-width: 400px; margin: auto; box-shadow: 0 8px 16px rgba(0,0,0,0.5); }
  .btn { padding: 15px; font-size: 18px; cursor: pointer; border: none; border-radius: 12px; background: #03dac6; width: 100%; font-weight: bold; }
  input { background: #333; border: 1px solid #444; color: #fff; padding: 8px; border-radius: 6px; width: 50px; text-align: center; }
  .status-box { margin: 20px 0; padding: 10px; border: 1px solid #333; border-radius: 10px; font-size: 0.9em; color: #888; }
</style>
</head><body>
  <div class="card">
    <h2>💡 <span id="st">...</span></h2>
    <button class="btn" onclick="fetch('/toggle').then(()=>location.reload())">SWITCH</button>
    <div class="status-box">Hostname: IskakINO-SmartLamp</div>
    <form action="/setsched">
      <p>Jadwal Otomatis</p>
      ON: <input type="number" name="onH" placeholder="HH">:<input type="number" name="onM" placeholder="mm"><br><br>
      OFF: <input type="number" name="offH" placeholder="HH">:<input type="number" name="offM" placeholder="mm"><br><br>
      <button type="submit" style="background:#444; color:white; border:none; padding:10px; width:100%; border-radius:8px;">SIMPAN JADWAL</button>
    </form>
  </div>
  <script>
    fetch('/status').then(r=>r.json()).then(d=>{ document.getElementById('st').innerText = d.s ? 'LAMPU NYALA' : 'LAMPU MATI'; });
  </script>
</body></html>)rawliteral";

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println(F("\n\n--- ISKAKINO SMART LAMP SYSTEM ---"));
  
  pinMode(RELAY_PIN, OUTPUT);

  // 1. Storage Setup
  IskakStorage.begin("smartfit", true);
  if (!IskakStorage.load(0, settings)) {
    settings = {18, 0, 5, 0, false, 0};
  }
  settings.bootCount++;
  IskakStorage.save(0, settings);

  // 2. Network Configuration (Hostname)
  Serial.print(F("[+]\tSetting Hostname: "));
  Serial.println(MY_HOSTNAME);
  #if defined(ESP32)
    WiFi.setHostname(MY_HOSTNAME);
  #else
    WiFi.hostname(MY_HOSTNAME);
  #endif

  // 3. Portal Setup
  portal.begin("IskakINO-SmartFit");

  Serial.println(F("------------------------------------"));
  Serial.print(F("[*]\tIP Address : ")); Serial.println(WiFi.localIP());
  Serial.print(F("[*]\tHostname   : ")); Serial.println(MY_HOSTNAME);
  Serial.println(F("------------------------------------"));

  // 4. Web Routes
  portal.server()->on("/", HTTP_GET, [&]() {
    portal.server()->send(200, "text/html", DASHBOARD_HTML);
  });

  portal.server()->on("/status", HTTP_GET, [&]() {
    portal.server()->send(200, "application/json", "{\"s\":" + String(settings.lampState) + "}");
  });

  portal.server()->on("/toggle", HTTP_GET, [&]() {
    updateRelay(!settings.lampState);
    portal.server()->send(200, "text/plain", "OK");
  });

  portal.server()->on("/setsched", HTTP_GET, [&]() {
    if(portal.server()->hasArg("onH")) settings.onHour = portal.server()->arg("onH").toInt();
    if(portal.server()->hasArg("onM")) settings.onMin = portal.server()->arg("onM").toInt();
    if(portal.server()->hasArg("offH")) settings.offHour = portal.server()->arg("offH").toInt();
    if(portal.server()->hasArg("offM")) settings.offMin = portal.server()->arg("offM").toInt();
    IskakStorage.save(0, settings);
    portal.server()->send(200, "text/html", "<script>alert('Jadwal Disimpan!');location.href='/';</script>");
  });

  ntp.begin(25200); // GMT+7
  updateRelay(settings.lampState, true);
  Serial.println(F("[+]\tSystem Ready."));
}

// ================== LOOP ==================
void loop() {
  portal.handle();
  ntp.update();

  // Task: Monitoring Internet & Reconnect
  if (ArduFast.every(10000, 1)) {
    if (WiFi.status() != WL_CONNECTED) {
       Serial.println(F("[!]\tWiFi Terputus! Mencoba menyambung kembali..."));
       WiFi.reconnect();
    }
  }

  // Task: Scheduler & Debug Log
  if (ArduFast.every(30000, 0)) {
    int h = ntp.getHours();
    int m = ntp.getMinutes();

    Serial.print(F("[LOG]\tTime: "));
    if(h < 10) Serial.print('0'); Serial.print(h); Serial.print(':');
    if(m < 10) Serial.print('0'); Serial.print(m);
    Serial.print(F(" | Relay: ")); Serial.println(settings.lampState ? F("ON") : F("OFF"));

    if (h == settings.onHour && m == settings.onMin && !settings.lampState) {
      updateRelay(true);
    } 
    else if (h == settings.offHour && m == settings.offMin && settings.lampState) {
      updateRelay(false);
    }
  }
}
