/*
 * Project: IskakINO_BelSekolah_C3 (Final Pro Edition)
 * Author: iskakfatoni
 * Board: ESP32-C3
 * Features: JSON Preferences, On-The-Fly Update, Dynamic 20x4 LCD
 */

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
#define TX_PIN 21  // Ke RX DFPlayer (Gunakan Resistor 1K)

// --- GLOBALS ---
IskakINO_ArduFast ArduFast;
IskakINO_WifiPortal Portal;
WiFiUDP udp;
IskakINO_FastNTP ntp(udp);
LiquidCrystal_I2C lcd(20, 4);
HardwareSerial voiceSerial(1); // Hardware Serial 1 ESP32

const char* KEY_JADWAL = "cfg_bel";
StaticJsonDocument<2048> activeDoc;
bool refreshJadwal = true;

// --- PROGMEM HTML ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>IskakINO Bel</title><meta name='viewport' content='width=device-width,initial-scale=1'>
<style>body{background:#1a1a1a;color:#eee;font-family:sans-serif;padding:15px;text-align:center}.card{background:#252525;padding:20px;border-radius:10px;max-width:500px;margin:auto}
input{background:#333;border:1px solid #444;color:#fff;padding:8px;border-radius:5px;width:70px}button{width:100%;padding:12px;background:#00d1b2;border:none;color:#fff;font-weight:bold;border-radius:5px;margin-top:20px}
table{width:100%;border-collapse:collapse;margin-top:15px}th,td{border:1px solid #444;padding:8px}th{color:#00d1b2}</style></head>
<body><div class='card'><h2>IskakINO Dashboard</h2><p>Setting Jadwal Bel On-The-Fly</p>
<input id='start' placeholder='07:00'> <input id='tot' type='number' placeholder='Total'> <input id='dur' type='number' placeholder='Durasi'>
<button onclick='save()'>SIMPAN JADWAL</button><div id='list'></div></div>
<script>
async function load(){ let r=await fetch('/get_json'); let d=await r.json(); document.getElementById('start').value=d.start; }
async function save(){ 
    let d={start:document.getElementById('start').value,total:parseInt(document.getElementById('tot').value),dur:parseInt(document.getElementById('dur').value),vol:25};
    await fetch('/set_json',{method:'POST',body:JSON.stringify(d)}); alert('Updated!');
} window.onload=load;
</script></body></html>)rawliteral";

// --- FUNCTIONS ---

void showBootScreen() {
    lcd.clear();
    lcd.printCenter("IskakINO", 0);
    lcd.printCenter("BelSekolah", 1);
    lcd.printCenter("by", 2);
    lcd.printCenter("Iskak Fatoni", 3);
    delay(3000);
}

void handleGetJson() {
    char buf[2048];
    IskakStorage.loadString(KEY_JADWAL, buf, sizeof(buf));
    Portal.server()->send(200, "application/json", buf);
}

void handleSetJson() {
    if (Portal.server()->hasArg("plain")) {
        IskakStorage.saveString(KEY_JADWAL, Portal.server()->arg("plain").c_str());
        refreshJadwal = true; 
        Portal.server()->send(200, "text/plain", "OK");
    }
}

String getNextBell() {
    if (refreshJadwal) return "--:--";
    // Logika perhitungan menit (seperti diskusi sebelumnya)
    return activeDoc["start"] | "07:00"; 
}

void updateLCD_Main() {
    lcd.setCursor(0, 0);
    lcd.print(ntp.getFormattedTime() + " " + ntp.getDayName(LANG_ID));

    lcd.setCursor(0, 1);
    if (WiFi.status() == WL_CONNECTED) {
        if ((millis() / 3000) % 2 == 0) {
            lcd.print("IP: " + WiFi.localIP().toString() + "    ");
        } else {
            lcd.print("SSID: " + WiFi.SSID().substring(0, 14) + "    ");
        }
    } else {
        lcd.print("AP: IskakINO_Config ");
    }

    lcd.setCursor(0, 2);
    lcd.print("Next: " + getNextBell() + "        ");
    
    lcd.setCursor(0, 3);
    lcd.print("V:" + String(activeDoc["vol"]|25) + " Iskak Fatoni  ");
}

void runSchedule() {
    if (refreshJadwal) {
        char buf[2048];
        IskakStorage.loadString(KEY_JADWAL, buf, sizeof(buf));
        deserializeJson(activeDoc, buf);
        refreshJadwal = false;
        IskakVoice.setVolume(activeDoc["vol"] | 25);
    }
    // Implementasi pengecekan ntp.isAlarmActive sesuai JSON...
}

void setup() {
    ArduFast.begin(115200);
    lcd.begin();
    lcd.backlight();
    showBootScreen();

    IskakStorage.begin("BelSekolah");
    
    if (!Portal.begin("IskakINO_Config")) {
        while (WiFi.status() != WL_CONNECTED) { Portal.handle(); delay(10); }
    }

    Portal.server()->on("/bel", HTTP_GET, [](){ Portal.server()->send_P(200, "text/html", index_html); });
    Portal.server()->on("/get_json", HTTP_GET, handleGetJson);
    Portal.server()->on("/set_json", HTTP_POST, handleSetJson);

    voiceSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
    IskakVoice.begin(voiceSerial);
    ntp.begin();
}

void loop() {
    Portal.handle();
    if (WiFi.status() == WL_CONNECTED) {
        ntp.update();
        if (ArduFast.every(1000, 1)) runSchedule();
    }
    if (ArduFast.every(1000, 0)) updateLCD_Main();
}
