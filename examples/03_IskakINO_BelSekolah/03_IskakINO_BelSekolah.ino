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

// --- STRUKTUR DATA ---
struct AlarmConfig {
  uint8_t jam;
  uint8_t menit;
  uint16_t track;
  bool aktif;
};

#define MAX_JADWAL 20
AlarmConfig daftarJadwal[MAX_JADWAL];

// --- INSTANCE OBJECT (PENTING: Perhatikan Nama Di Sini) ---
IskakINO_ArduFast af;
IskakINO_Storage  IskakStorage; // Membuat object IskakStorage agar dikenali
LiquidCrystal_I2C lcd(20, 4);   // Menggunakan nama class dari library Anda
IskakINO_SmartVoice voice;
WiFiUDP udp;
IskakINO_FastNTP ntp(udp);
IskakINO_WifiPortal portal;

int lastTriggerMinute = -1;

// --- HTML DASHBOARD ---
const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>
<title>Bel Dashboard</title>
<style>
  body { font-family: sans-serif; background: #f0f2f5; padding: 10px; }
  .card { background: white; padding: 15px; border-radius: 10px; max-width: 500px; margin: auto; }
  table { width: 100%; font-size: 14px; border-collapse: collapse; }
  th, td { padding: 5px; border-bottom: 1px solid #ddd; }
  input { width: 45px; }
  .btn { padding: 8px 15px; border: none; border-radius: 4px; color: white; cursor: pointer; text-decoration: none; display: inline-block; margin: 2px; }
  .save { background: #2ecc71; } .backup { background: #3498db; } .reset { background: #e74c3c; }
</style></head>
<body><div class='card'>
  <h3>🏫 Bel Pintar C3</h3>
  <form action='/save' method='POST'>
    <table><tr><th>Jam</th><th>Min</th><th>TRK</th><th>On</th></tr>
    <tbody id='tbl'></tbody></table><br>
    <center><button type='submit' class='btn save'>💾 SIMPAN</button></center>
  </form><hr>
  <center>
    <a href='/export' class='btn backup'>📤 EXP</a>
    <button onclick='document.getElementById("f").click()' class='btn backup'>📥 IMP</button>
    <form action='/import' method='POST' enctype='multipart/form-data' style='display:none'><input type='file' id='f' name='up' onchange='this.form.submit()'></form>
    <a href='/reset' class='btn reset' onclick='return confirm("Reset?")'>⚠️ RST</a>
  </center>
</div>
<script>
  let h=""; for(let i=0;i<20;i++){h+=`<tr><td><input type='number' name='h${i}'></td><td><input type='number' name='m${i}'></td><td><input type='number' name='t${i}'></td><td><input type='checkbox' name='s${i}'></td></tr>`;}
  document.getElementById('tbl').innerHTML=h;
</script></body></html>)rawliteral";

// --- HANDLERS ---
void setupWebHandlers() {
  portal._server->on("/", HTTP_GET, []() { 
    portal._server->send(200, "text/html", DASHBOARD_HTML); 
  });

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
    portal._server->send(200, "application/octet-stream", (uint8_t*)&daftarJadwal, sizeof(daftarJadwal));
  });

  portal._server->on("/import", HTTP_POST, []() { 
    portal._server->send(200, "text/html", "Done! Restarting..."); 
    delay(1000); ESP.restart(); 
  }, []() {
    HTTPUpload& up = portal._server->upload();
    if (up.status == UPLOAD_FILE_WRITE) IskakStorage.save(0, up.buf);
  });

  portal._server->on("/reset", HTTP_GET, []() { 
    IskakStorage.clear(); 
    portal._server->send(200, "text/html", "Reset! Restarting..."); 
    delay(1000); ESP.restart(); 
  });
}

void setup() {
  af.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  
  lcd.begin();
  lcd.backlight();
  lcd.printCenter("BEL PINTAR C3", 0);

  Serial1.begin(9600, SERIAL_8N1, PIN_MP3_RX, PIN_MP3_TX);
  voice.begin(Serial1);
  voice.setVolume(25);

  IskakStorage.begin(); 
  if (!IskakStorage.load(0, daftarJadwal)) {
    for(int i=0; i<MAX_JADWAL; i++) daftarJadwal[i] = {0,0,0,false};
    IskakStorage.save(0, daftarJadwal);
  }

  portal.setBrandName("Bel Pintar C3");
  setupWebHandlers();
  
  if (!portal.begin("BEL_PINTAR_C3")) {
    lcd.printCenter("AP MODE ACTIVE", 2);
  }

  ntp.begin();
  lcd.clear();
}

void loop() {
  portal.handle();
  ntp.update();
  if (af.every(1000, 0)) {
    lcd.setCursor(6, 0); lcd.print(ntp.getFormattedTime());
    lcd.printCenter(ntp.getDayName() + " " + ntp.getFormattedDate('/'), 1);
    lcd.setCursor(0, 3); lcd.print(WiFi.localIP().toString());
  }
  
  if (af.every(1000, 1)) {
    if (ntp.isTimeSet()) {
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
  }
}
