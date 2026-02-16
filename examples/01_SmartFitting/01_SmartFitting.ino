#include <IskakINO_ArduFast.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_Storage.h> 
#include <WiFiUdp.h>

// Struktur data untuk disimpan
struct SystemConfig {
  bool lampState;
  int bootCount;
};

SystemConfig config;

// Inisialisasi Objects berdasarkan header library Anda
IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org"); // Membutuhkan UDP

void setup() {
  // ArduFast menggunakan instance global 'ArduFast'
  ArduFast.begin(115200); 
  
  // IskakStorage menggunakan instance global 'IskakStorage'
  IskakStorage.begin("smartfit", true);
  
  // Load data dari alamat 0
  if (!IskakStorage.load(0, config)) {
    config.lampState = false;
    config.bootCount = 0;
  }
  
  config.bootCount++;
  IskakStorage.save(0, config);

  // Inisialisasi Portal & NTP
  portal.begin("IskakINO-Project01");
  ntp.begin();
}

void loop() {
  // WAJIB ADA agar tidak error 'undefined reference to loop'
  portal.handle(); // Menangani WebServer & DNS
  ntp.update();   // Update waktu secara non-blocking
  
  // Contoh penggunaan ArduFast scheduler
  if (ArduFast.every(5000, 0)) {
     ArduFast.log(F("System Uptime (s)"), ntp.getUptimeSeconds());
  }
}
