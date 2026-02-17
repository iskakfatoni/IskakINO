#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include <IskakINO_ArduFast.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_LiquidCrystal_I2C.h>
#include <IskakINO_SmartVoice.h>
#include <IskakINO_WifiPortal.h>

// Buffer untuk Parameter (Disimpan otomatis oleh WifiPortal ke LittleFS/Prefs)
char p_mode[2] = "0";      // Mode Bel 0-9
char p_vol[3] = "25";      // Volume 0-30
char p_jam_masuk[6] = "07:00";

IskakINO_ArduFast ArduFast;
IskakINO_WifiPortal Portal;
WiFiUDP udp;
IskakINO_FastNTP ntp(udp);
LiquidCrystal_I2C lcd(20, 4);
SoftwareSerial voiceSerial(D7, D8);

void setup() {
    ArduFast.begin(115200);
    lcd.begin();
    lcd.backlight();
    
    // 1. Daftarkan Parameter Bel ke Dashboard
    // Parameter ini akan muncul baik di mode AP maupun STA
    Portal.addParameter("mode", "Preset Jadwal (0-9)", p_mode, 1);
    Portal.addParameter("vol", "Volume Bel", p_vol, 2);
    Portal.addParameter("j_in", "Jam Masuk Utama", p_jam_masuk, 5);

    // 2. Mulai Portal (Otomatis Scan WiFi jika di mode AP)
    lcd.printCenter("Mencari WiFi...", 0);
    if (!Portal.begin("BelSekolah-Iskak")) {
        // Jika masuk sini, berarti sedang mode AP (Config)
        lcd.clear();
        lcd.printCenter("MODE CONFIG (AP)", 0);
        lcd.printCenter("192.168.4.1", 1);
        lcd.setCursor(0, 3);
        lcd.print("Silahkan Setting Bel");
    }

    // 3. Inisialisasi Hardware Lain
    ntp.begin();
    voiceSerial.begin(9600);
    IskakVoice.begin(voiceSerial);
    IskakVoice.setVolume(atoi(p_vol));
}

void loop() {
    Portal.handle(); // Menangani Web Dashboard & Scan WiFi List
    
    // Hanya update NTP dan Cek Bel jika sudah terkoneksi WiFi (STA)
    if (WiFi.status() == WL_CONNECTED) {
        ntp.update();
        checkSchoolBell();
    }

    if (ArduFast.every(1000, 0)) {
        updateLCD();
    }
}

void checkSchoolBell() {
    // Logika perbandingan jam ntp dengan parameter p_jam_masuk
    int h = (p_jam_masuk[0] - '0') * 10 + (p_jam_masuk[1] - '0');
    int m = (p_jam_masuk[3] - '0') * 10 + (p_jam_masuk[4] - '0');

    if (ntp.isAlarmActive(h, m, 0)) {
        IskakVoice.playTrack(1); 
        ArduFast.log(F("Bel Berbunyi!"));
    }
}

void updateLCD() {
    if (WiFi.status() == WL_CONNECTED) {
        lcd.setCursor(0, 0);
        lcd.print(ntp.getFormattedTime() + " " + ntp.getDayName(LANG_ID));
        lcd.setCursor(0, 1);
        lcd.print("IP: " + WiFi.localIP().toString());
    } else {
        // Tampilan saat mode AP
        lcd.setCursor(0, 2);
        lcd.print("WiFi Disconnected   ");
    }
}
