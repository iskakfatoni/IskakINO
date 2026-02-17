#include <Arduino.h>
#include <Wire.h>
#include <WiFiUdp.h>

// Include IskakINO Framework
#include "IskakINO_ArduFast.h"
#include "IskakINO_Storage.h"
#include "IskakINO_FastNTP.h"
#include "IskakINO_SmartVoice.h"
#include "IskakINO_LiquidCrystal_I2C.h"
#include "IskakINO_WifiPortal.h"

// --- KONFIGURASI HARDWARE ESP32-C3 ---
#define PIN_SDA      8
#define PIN_SCL      9
#define PIN_MP3_TX   21
#define PIN_MP3_RX   20
#define PIN_BUSY     3

// --- STRUKTUR DATA ---
struct AlarmConfig {
  uint8_t jam;
  uint8_t menit;
  uint16_t track;
  bool aktif;
};

#define MAX_JADWAL 20
AlarmConfig daftarJadwal[MAX_JADWAL];

// --- INSTANCE ---
IskakINO_ArduFast af;
IskakINO_LiquidCrystal_I2C lcd(20, 4);
IskakINO_SmartVoice voice;
WiFiUDP udp;
IskakINO_FastNTP ntp(udp);
IskakINO_WifiPortal portal;

int lastTriggerMinute = -1;

// --- HTML DASHBOARD (PROGMEM) ---
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>
<title>IskakINO Bel Dashboard</title>
<style>
  body { font-family: sans-serif; background: #f0f2f5; padding: 20px; }
  .card { background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); max-width: 600px; margin: auto; }
  table { width: 100%; border-collapse: collapse; margin: 20px 0; }
  th, td { padding: 10px; border-bottom: 1px solid #ddd; text-align: center; }
  .btn { padding: 10px 20px; border: none; border-radius: 5px; cursor: pointer; color: white; text-decoration: none; display: inline-block; }
  .save { background: #2ecc71; } .backup { background: #3498db; } .reset { background: #e74c3c; }
</style></head>
<body><div class='card'>
  <h2>🏫 Bel Sekolah Pintar</h2>
  <form action='/save' method='POST'>
    <table><tr><th>Jam</th><th>Min</th><th>Track</th><th>On</th></tr>
    <tbody id='tbl'></tbody></table>
    <center><button type='submit' class='btn save'>💾 SIMPAN JADWAL</button></center>
  </form>
  <hr>
  <center>
    <a href='/export' class='btn backup'>📤 EXPORT</a>
    <button onclick='document.getElementById("f").click()' class='btn backup'>📥 IMPORT</button>
    <form action='/import' method='POST' enctype='multipart/form-data' style='display:none'>
      <input type='file' id='f' name='up' onchange='this.form.submit()'>
    </form>
    <a href='/reset' class='btn reset' onclick='return confirm("Hapus semua?")'>⚠️ RESET</a>
  </center>
</div>
<script>
  let html = "";
  for(let i=0; i<20; i++) {
    html += `<tr>
      <td><input type='number' name='h${i}' min='0' max='23' style='width:40px'></td>
      <td><input type='number' name='m${i}' min='0' max='59' style='width:40px'></td>
      <td><input type='number' name='t${i}' min='1' max='255' style='width:40px'></td>
      <td><input type='checkbox' name='s${i}'></td>
    </tr>`;
  }
  document.getElementById('tbl').innerHTML = html;
</script>
</body></html>)rawliteral";

void setup() {
  af.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  
  // 1. Setup LCD
  lcd.begin();
  lcd.backlight();
  lcd.printCenter("ISKANKINO PROJECT", 0);
  lcd.printCenter("BEL PINTAR C3", 1);

  // 2. Setup Audio
  Serial1.begin(9600, SERIAL_8N1, PIN_MP3_RX, PIN_MP3_TX);
  voice.begin(Serial1);
  voice.setVolume(25);

  // 3. Load Storage
  IskakStorage.begin();
  if (!IskakStorage.load(0, daftarJadwal)) {
    for(int i=0; i<MAX_JADWAL; i++) daftarJadwal[i] = {0,0,0,false};
    IskakStorage.save(0, daftarJadwal);
  }

  // 4. Setup Portal & WiFi
  portal.setBrandName("Bel Pintar C3");
  setupWebHandlers();
  if (!portal.begin("BEL_PINTAR_C3")) {
    lcd.clear();
    lcd.printCenter("MODE SETUP AP", 0);
    lcd.printCenter("IP: 192.168.4.1", 3);
  }

  // 5. Setup NTP
  ntp.begin();
  lcd.clear();
}

void loop() {
  portal.handle();
  ntp.update();

  if (af.every(1000, 0)) updateLCD();
  if (af.every(1000, 1)) checkAlarm();
}

void checkAlarm() {
  if (!ntp.isTimeSet()) return;
  int h = ntp.getHours();
  int m = ntp.getMinutes();

  if (m != lastTriggerMinute) {
    for (int i = 0; i < MAX_JADWAL; i++) {
      if (daftarJadwal[i].aktif && daftarJadwal[i].jam == h && daftarJadwal[i].menit == m) {
        voice.announce(daftarJadwal[i].track);
        lastTriggerMinute = m;
        break;
      }
    }
  }
}

void updateLCD() {
  lcd.setCursor(6, 0); lcd.print(ntp.getFormattedTime());
  lcd.printCenter(ntp.getDayName() + ", " + ntp.getFormattedDate('/'), 1);
  lcd.setCursor(0, 2);
  lcd.print(WiFi.status() == WL_CONNECTED ? "WiFi: OK " : "WiFi: NO ");
  lcd.print(WiFi.localIP().toString());
  lcd.printCenter("IskakINO Smart Bell", 3);
}

void setupWebHandlers() {
  portal._server->on("/", HTTP_GET, []() { portal._server->send(200, "text/html", DASHBOARD_HTML); });

  portal._server->on("/save", HTTP_POST, []() {
    for (int i = 0; i < MAX_JADWAL; i++) {
      if(portal._server->hasArg("h"+String(i))) {
        daftarJadwal[i].jam = portal._server->arg("h"+String(i)).toInt();
        daftarJadwal[i].menit = portal._server->arg("m"+String(i)).toInt();
        daftarJadwal[i].track = portal._server->arg("t"+String(i)).toInt();
        daftarJadwal[i].aktif = portal._server->hasArg("s"+String(i));
      }
    }
    IskakStorage.save(0, daftarJadwal);
    portal._server->send(200, "text/html", "Saved! <a href='/'>Back</a>");
  });

  portal._server->on("/export", HTTP_GET, []() {
    portal._server->sendHeader("Content-Disposition", "attachment; filename=jadwal.bin");
    portal._server->send(200, "application/octet-stream", (const char*)&daftarJadwal, sizeof(daftarJadwal));
  });

  portal._server->on("/import", HTTP_POST, []() { portal._server->send(200, "text/html", "Done! Restarting..."); delay(1000); ESP.restart(); }, []() {
    HTTPUpload& up = portal._server->upload();
    if (up.status == UPLOAD_FILE_WRITE) IskakStorage.save(0, up.buf);
  });

  portal._server->on("/reset", HTTP_GET, []() { 
    IskakStorage.clear();
