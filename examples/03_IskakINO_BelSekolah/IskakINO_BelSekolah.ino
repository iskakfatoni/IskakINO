/*
 * Project: IskakINO_BelSekolah_C3 (Example 03 - Professional Edition)
 * Hardware: ESP32-C3 + DFPlayer Mini + LCD 20x4 I2C
 */

#include <WiFiUdp.h>
#include <IskakINO_ArduFast.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_LiquidCrystal_I2C.h>
#include <IskakINO_SmartVoice.h>
#include <IskakINO_WifiPortal.h>

// --- KONFIGURASI PIN ESP32-C3 ---
#define RX_PIN 20  // Hubungkan ke TX DFPlayer
#define TX_PIN 21  // Hubungkan ke RX DFPlayer (Gunakan Resistor 1K)

// --- PARAMETER DASHBOARD ---
char p_total_jam[3] = "12";     
char p_durasi[3]    = "45";     
char p_jam_mulai[6] = "07:00";  
char p_ist1_dur[3]  = "15"; 
char p_ist1_pos[2]  = "4";  
char p_ist2_dur[3]  = "15"; 
char p_ist2_pos[2]  = "8";
char p_vol[3]       = "25";

IskakINO_ArduFast ArduFast;
IskakINO_WifiPortal Portal;
WiFiUDP udp;
IskakINO_FastNTP ntp(udp);
LiquidCrystal_I2C lcd(20, 4);

// Pada ESP32, kita gunakan HardwareSerial 1 (Serial1)
HardwareSerial voiceSerial(1); 

void setup() {
    ArduFast.begin(115200);
    lcd.begin();
    lcd.backlight();
    
    // Registrasi Parameter
    Portal.addParameter("start", "Jam Mulai", p_jam_mulai, 5);
    Portal.addParameter("tot", "Total Jam", p_total_jam, 2);
    Portal.addParameter("dur", "Durasi (Min)", p_durasi, 2);
    Portal.addParameter("i1p", "Istirahat1 (Jam Ke-)", p_ist1_pos, 1);
    Portal.addParameter("i1d", "Durasi Istirahat1", p_ist1_dur, 2);
    Portal.addParameter("i2p", "Istirahat2 (Jam Ke-)", p_ist2_pos, 1);
    Portal.addParameter("i2d", "Durasi Istirahat2", p_ist2_dur, 2);
    Portal.addParameter("vol", "Volume (0-30)", p_vol, 2);

    if (!Portal.begin("Iskak-BelC3")) {
        lcd.printCenter("Connect to WiFi", 0);
        while (WiFi.status() != WL_CONNECTED) { Portal.handle(); delay(10); }
    }

    // Inisialisasi Serial Hardware untuk ESP32-C3
    voiceSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
    IskakVoice.begin(voiceSerial);
    IskakVoice.setVolume(atoi(p_vol));
    ntp.begin();

    lcd.clear();
}

void loop() {
    Portal.handle();
    if (WiFi.status() == WL_CONNECTED) {
        ntp.update();
        if (ArduFast.every(1000, 1)) runScheduleLogic();
    }
    if (ArduFast.every(1000, 0)) updateLCD();
}

void runScheduleLogic() {
    int startH = (p_jam_mulai[0] - '0') * 10 + (p_jam_mulai[1] - '0');
    int startM = (p_jam_mulai[3] - '0') * 10 + (p_jam_mulai[4] - '0');
    int currentMins = startH * 60 + startM;

    // Cek Masuk Jam 1
    if (ntp.isAlarmActive(startH, startM, 0)) { IskakVoice.playTrack(1); return; }

    for (int i = 1; i <= atoi(p_total_jam); i++) {
        currentMins += atoi(p_durasi);

        // Logika Istirahat 1
        if (i == atoi(p_ist1_pos)) {
            if (ntp.isAlarmActive(currentMins/60, currentMins%60, 0)) { IskakVoice.playTrack(2); return; }
            currentMins += atoi(p_ist1_dur);
            if (ntp.isAlarmActive(currentMins/60, currentMins%60, 0)) { IskakVoice.playTrack(5); return; }
        }
        // Logika Istirahat 2
        else if (i == atoi(p_ist2_pos)) {
            if (ntp.isAlarmActive(currentMins/60, currentMins%60, 0)) { IskakVoice.playTrack(2); return; }
            currentMins += atoi(p_ist2_dur);
            if (ntp.isAlarmActive(currentMins/60, currentMins%60, 0)) { IskakVoice.playTrack(5); return; }
        }
        // Pergantian Jam & Pulang
        else {
            if (ntp.isAlarmActive(currentMins/60, currentMins%60, 0)) {
                if (i == atoi(p_total_jam)) IskakVoice.playTrack(3);
                else IskakVoice.playTrack(4);
                return;
            }
        }
    }
}

void updateLCD() {
    lcd.setCursor(0, 0);
    lcd.print(ntp.getFormattedTime() + " ESP32-C3");
    lcd.setCursor(0, 1);
    lcd.print(ntp.getDayName(LANG_ID) + ", " + ntp.getFormattedDate());
    lcd.setCursor(0, 3);
    lcd.print("IP: " + WiFi.localIP().toString());
}
