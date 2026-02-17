/************************************************************
 * PROJECT    : Smart Fitting Lamp IskakINO
 * BOARD      : ESP32-C3 / ESP8266
 * VERSION    : v1.5.5 (Stable Build)
 * AUTHOR     : iskakfatoni
 ************************************************************/

#include <IskakINO_ArduFast.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_Storage.h>
#include <WiFiUdp.h>

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
IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org");

unsigned long lastTransitionMs = 0;
const unsigned long MIN_INTERVAL = 3000; 

// ================== LOGIK RELAY ==================
void updateRelay(bool state, bool force = false) {
  unsigned long current = millis();
  if (!force && (current - lastTransitionMs < MIN_INTERVAL)) {
    // Gunakan F() dan pastikan argumen kedua adalah INT
    ArduFast.log(F("Safety"), -1); 
    return;
  }
  
  digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  settings.lampState = state;
  lastTransitionMs = current;
  
  ArduFast.log(F("Relay"), state ? 1 : 0);
  IskakStorage.save(0, settings);
}

// ================== DASHBOARD HTML ==================
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>IskakINO SmartFitting</title>
<style>
  body { font-family: sans-serif; background: #121212; color: white; text-align: center; }
  .card { background: #1e1e1e; padding: 20px; border-radius: 15px; display: inline-block; margin-top: 50px; }
  .btn { padding: 15px 30px; font-size: 18px; cursor: pointer; border: none; border-radius: 10px; background: #03dac6; }
</style>
</head><body>
  <div class="card">
    <h2>💡 Lampu: <span id="st">...</span></h2>
    <button class="btn" onclick="fetch('/toggle').then(()=>location.reload())">ON / OFF</button>
    <hr>
    <form action="/setsched">
      ON: <input type="number" name="onH" style="width:40px">:<input type="number" name="onM" style="width:40px"><br><br>
      OFF: <input type="number" name="offH" style="width:40px">:<input type="number" name="offM" style="width:40px"><br><br>
      <button type="submit">Simpan Jadwal</button>
    </form>
  </div>
  <script>
    fetch('/status').then(r=>r.json()).then(d=>{ document.getElementById('st').innerText = d.s ? 'NYALA' : 'MATI'; });
  </script>
</body></html>
)rawliteral";

// ================== SETUP & LOOP ==================
void setup() {
  ArduFast.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);

  // 1. Inisialisasi Storage & Load Data
  IskakStorage.begin("smartfit", true);
  if (!IskakStorage.load(0, settings)) {
    settings = {18, 0, 5, 0, false, 0};
  }
  settings.bootCount++;
  IskakStorage.save(0, settings);

  // 2. WiFi Portal
  portal.begin("IskakINO-SmartFit");

  // 3. Custom Routes (Gunakan portal._server->)
  portal._server->on("/", HTTP_GET, []() {
    portal._server->send(200, "text/html", DASHBOARD_HTML);
  });

  portal._server->on("/status", HTTP_GET, []() {
    String j = "{\"s\":" + String(settings.lampState) + "}";
    portal._server->send(200, "application/json", j);
  });

  portal._server->on("/toggle", HTTP_GET, []() {
    updateRelay(!settings.lampState);
    portal._server->send(200, "text/plain", "OK");
  });

  portal._server->on("/setsched", HTTP_GET, []() {
    settings.onHour = portal._server->arg("onH").toInt();
    settings.onMin = portal._server->arg("onM").toInt();
    settings.offHour = portal._server->arg("offH").toInt();
    settings.offMin = portal._server->arg("offM").toInt();
    IskakStorage.save(0, settings);
    portal._server->send(200, "text/html", "<script>alert('Jadwal disimpan!');location.href='/';</script>");
  });

  ntp.begin(25200); // GMT+7
  updateRelay(settings.lampState, true);
}

void loop() {
  portal.handle();
  ntp.update();

  // Cek Jadwal setiap 30 detik (ID task: 0)
  if (ArduFast.every(30000, 0)) {
    int h = ntp.getHours();
    int m = ntp.getMinutes();

    if (h == settings.onHour && m == settings.onMin && !settings.lampState) {
      updateRelay(true);
    } 
    else if (h == settings.offHour && m == settings.offMin && settings.lampState) {
      updateRelay(false);
    }
  }
}
