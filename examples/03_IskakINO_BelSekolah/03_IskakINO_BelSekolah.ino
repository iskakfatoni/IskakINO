/************************************************************
 * PROJECT    : IskakINO Bel Sekolah Pro
 * BOARD      : ESP32-C3 / ESP8266
 * VERSION    : v2.1.0 (Import/Export & Sync)
 * AUTHOR     : iskakfatoni
 ************************************************************/

#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <IskakINO_ArduFast.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_LiquidCrystal_I2C.h>
#include <IskakINO_SmartVoice.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_Storage.h>

// --- PINOUT ---
#define RX_PIN 20
#define TX_PIN 21

// --- GLOBALS ---
IskakINO_ArduFast ArduFast;
IskakINO_WifiPortal Portal;
IskakINO_Storage IskakStorage; // Deklarasi manual karena extern di-comment
WiFiUDP udp;
IskakINO_FastNTP ntp(udp);
LiquidCrystal_I2C lcd(20, 4); // Sesuai konstruktor library
HardwareSerial voiceSerial(1);
IskakINO_SmartVoice IskakVoice;

const int KEY_JADWAL = 0; // Alamat storage tipe INT
StaticJsonDocument<2048> activeDoc;
bool refreshJadwal = true;

// --- DASHBOARD HTML TERBARU (Support Import/Export) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>IskakINO Bel</title><meta name='viewport' content='width=device-width,initial-scale=1'>
<style>
  body{background:#121212;color:#e0e0e0;font-family:sans-serif;padding:10px;text-align:center}
  .card{background:#1e1e1e;padding:20px;border-radius:15px;max-width:450px;margin:auto;box-shadow:0 4px 15px rgba(0,0,0,0.5)}
  input, select{background:#2c2c2c;border:1px solid #444;color:#fff;padding:10px;border-radius:8px;width:80%;margin-bottom:10px}
  button{width:100%;padding:12px;background:#00d1b2;border:none;color:#fff;font-weight:bold;border-radius:8px;margin-top:10px;cursor:pointer}
  .btn-alt{background:#4a4a4a}.btn-warn{background:#ff4757}
</style></head>
<body><div class='card'>
  <h2>🔔 IskakINO Bel Pro</h2>
  <input id='start' placeholder='Jam Mulai (07:00)'>
  <input id='vol' type='number' placeholder='Volume (0-30)'>
  <button onclick='saveData()'>SIMPAN JADWAL</button>
  <hr style='border:0.5px solid #444;margin:20px 0'>
  <button class='btn-alt' onclick="location.href='/export'">EXPORT JADWAL (.json)</button>
  <p style='font-size:12px'>Import Data:</p>
  <input type="file" id="fileIn" accept=".json" style="width:100%;font-size:12px">
  <button class='btn-alt' onclick='importData()'>IMPORT JADWAL</button>
  <button class='btn-warn' onclick='if(confirm("Reset Total?"))location.href="/reset"'>FACTORY RESET</button>
</div>
<script>
async function load(){ 
  let r=await fetch('/get_json'); let d=await r.json(); 
  if(d.start) document.getElementById('start').value=d.start;
  if(d.vol) document.getElementById('vol').value=d.vol;
}
async function saveData(){
  let d={start:document.getElementById('start').value, vol:parseInt(document.getElementById('vol').value)};
  await fetch('/set_json',{method:'POST',body:JSON.stringify(d)}); alert('Tersimpan!');
}
async function importData(){
  let f=document.getElementById('fileIn').files[0]; if(!f) return alert('Pilih file!');
  let t=await f.text(); await fetch('/set_json',{method:'POST',body:t}); 
  alert('Import Berhasil!'); location.reload();
} window.onload=load;
</script></body></html>)rawliteral";

// --- HANDLERS ---

void handleGetJson() {
    char buf[2048];
    memset(buf, 0, sizeof(buf));
    if (IskakStorage.load(KEY_JADWAL, buf)) {
        Portal.server()->send(200, "application/json", buf);
    } else {
        Portal.server()->send(200, "application/json", "{}");
    }
}

void handleSetJson() {
    if (Portal.server()->hasArg("plain")) {
        String data = Portal.server()->arg("plain");
        char buf[2048];
        memset(buf, 0, sizeof(buf));
        strncpy(buf, data.c_str(), sizeof(buf) - 1);
        IskakStorage.save(KEY_JADWAL, buf);
        refreshJadwal = true; 
        Portal.server()->send(200, "text/plain", "OK");
    }
}

void handleExport() {
    char buf[2048];
    memset(buf, 0, sizeof(buf));
    if (IskakStorage.load(KEY_JADWAL, buf)) {
        Portal.server()->sendHeader("Content-Disposition", "attachment; filename=bel_config.json");
        Portal.server()->send(200, "application/json", buf);
    }
}

void runSchedule() {
    if (refreshJadwal) {
        char buf[2048];
        if (IskakStorage.load(KEY_JADWAL, buf)) {
            deserializeJson(activeDoc, buf);
            IskakVoice.setVolume(activeDoc["vol"] | 25);
        }
        refreshJadwal = false;
    }
}

void updateLCD() {
    lcd.setCursor(0, 0);
    lcd.print(ntp.getFormattedTime() + " " + ntp.getDayName(LANG_ID));
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString() + "  ");
    lcd.setCursor(0, 2);
    String s = activeDoc["start"] | "--:--";
    lcd.print("Jadwal: " + s + "   ");
    lcd.setCursor(0, 3);
    lcd.print("V:" + String(activeDoc["vol"]|25) + " Iskak Fatoni  ");
}

void setup() {
    ArduFast.begin(115200);
    
    // LCD
    lcd.begin();
    lcd.backlight();
    lcd.printCenter("IskakINO BEL", 1);

    // Storage
    IskakStorage.begin("BelPro", true);
    
    // Portal
    Portal.begin("IskakINO_Bel_Portal");

    // Server Routes
    Portal.server()->on("/", HTTP_GET, [](){ Portal.server()->send_P(200, "text/html", index_html); });
    Portal.server()->on("/get_json", HTTP_GET, handleGetJson);
    Portal.server()->on("/set_json", HTTP_POST, handleSetJson);
    Portal.server()->on("/export", HTTP_GET, handleExport);
    Portal.server()->on("/reset", HTTP_GET, [](){
        IskakStorage.clear();
        Portal.server()->send(200, "text/plain", "Reset OK. Rebooting...");
        delay(1000); ESP.restart();
    });

    voiceSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
    IskakVoice.begin(voiceSerial);
    ntp.begin();
    
    delay(2000);
    lcd.clear();
}

void loop() {
    Portal.handle();
    if (WiFi.status() == WL_CONNECTED) {
        ntp.update();
        if (ArduFast.every(1000, 1)) runSchedule();
    }
    if (ArduFast.every(1000, 0)) updateLCD();
}
