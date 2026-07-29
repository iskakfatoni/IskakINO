// Syntax-only check untuk IskakINO_WifiPortal.cpp jalur ESP32, memakai
// mock minimal WiFi/WebServer/DNSServer/Preferences/Update. Tujuannya
// memverifikasi kode yang disentuh migrasi (macro platform, Scheduler,
// Logger, Result, validasi File::open()) benar-benar valid secara C++,
// bukan cuma "kelihatan benar" saat dibaca manual.
#include "../../src/wifi/IskakINO_WifiPortal.cpp"

int main() { return 0; }
