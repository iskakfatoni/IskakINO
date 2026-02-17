/************************************************************
 * PROJECT    : IskakINO Bel Sekolah Pro
 * BOARD      : ESP32-C3 / ESP8266
 * VERSION    : v2.0.0 (Fully Synchronized)
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

// --- PINOUT ESP32-C3 ---
#define RX_PIN 20  // Ke TX DFPlayer
#define TX_PIN 21  // Ke RX DFPlayer

// --- GLOBALS ---
IskakINO_ArduFast ArduFast;
IskakINO_WifiPortal Portal;
IskakINO_Storage IskakStorage; // Deklarasi manual karena extern di-comment
WiFiUDP udp;
IskakINO_FastNTP ntp(udp);
LiquidCrystal_I2C lcd(20, 4); // Sesuai konstruktor (cols, rows)
HardwareSerial voiceSerial(1);
IskakINO_SmartVoice IskakVoice;

// Alamat storage harus INT
const int KEY_JADWAL = 0; 
StaticJsonDocument<2048> activeDoc;
bool refreshJadwal = true;

// --- PROGMEM HTML DASHBOARD ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>IskakINO Bel</title><meta name='viewport' content='width=device-width,initial-scale=1'>
<style>body{background:#1a1a1a;color:#eee;font-family:sans-serif;padding:15px;text-align:center}.card{background:#252525;padding:20px;border-radius:10px;max-width:500px;margin:auto}
input{background:#333;border:1px solid #444;color:#fff;padding:8px;border-radius:5px;width:70px}button{width:100%;padding:12px;background:#00d1b2;border:none;color:#fff;font-weight:bold;border-radius:5px;margin-top:20px}</style></head>
<body><div class='card'><h2>IskakINO Bel Dashboard</h2>
<input id='start' placeholder='07:00'> <input id='tot' type='number' placeholder='Total'> <input id='dur' type='number' placeholder='Durasi'>
<button onclick='save()'>SIMPAN JADWAL</button></div>
<script>
async function load(){ let r=await fetch('/get_json'); let d=await r.json(); if(d.start) document.getElementById('start').value=d.start; }
async function save(){ 
    let d={start:document.getElementById('start').value,total:parseInt(document.getElementById('tot').value),dur:parseInt(document.getElementById('dur').value),vol:25};
    await fetch('/set_json',{method:'POST',body:JSON.stringify(d)}); alert('Jadwal Diperbarui!');
} window.onload=load;
</script></body></html>)rawliteral";

// --- FUNCTIONS ---

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

void updateLCD_Main() {
    lcd.setCursor(0, 0);
    lcd.print(ntp.getFormattedTime() + " " + ntp.getDayName(LANG_ID));

    lcd.setCursor(0, 1);
    if (WiFi.status() == WL_CONNECTED) {
        lcd.print("IP: " + WiFi.localIP().toString());
    } else {
        lcd.print("Mode AP: IskakINO_Bel");
    }

    lcd.setCursor(0, 2);
    String startWaktu = activeDoc["start"] | "--:--";
    lcd.print("Mulai: " + startWaktu);

    lcd.setCursor(0, 3);
    lcd.print("V:" + String(activeDoc["vol"]|25) + " Iskak Fatoni");
}

void setup() {
    ArduFast.begin(115200);
    
    // LCD Setup menggunakan fitur library baru
    lcd.begin();
    lcd.backlight();
    lcd.printCenter("IskakINO BEL", 0);
    lcd.printCenter("Sistem Siap", 1);

    // Storage Setup
    IskakStorage.begin("BelSekolah", true);
    
    // Portal WiFi
    Portal.begin("IskakINO_Bel_Portal");

    // Server Routes
    Portal.server()->on("/", HTTP_GET, [](){ Portal.server()->send_P(200, "text/html", index_html); });
    Portal.server()->on("/get_json", HTTP_GET, handleGetJson);
    Portal.server()->on("/set_json", HTTP_POST, handleSetJson);
    
    // Reset Route (Fitur baru clear)
    Portal.server()->on("/reset", HTTP_GET, [](){
        IskakStorage.clear();
        Portal.server()->send(200, "text/plain", "Reset Berhasil. Rebooting...");
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
    if (ArduFast.every(1000, 0)) updateLCD_Main();
}
