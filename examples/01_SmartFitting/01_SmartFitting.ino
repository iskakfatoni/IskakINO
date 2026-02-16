#include <IskakINO_ArduFast.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_Storage.h> // Library baru Anda

// Struktur data untuk disimpan
struct SystemConfig {
  bool lampState;
  int bootCount;
};

SystemConfig config;

void setup() {
  IskakStorage.begin("smartfit", true);
  
  // Load data dari alamat 0
  if (!IskakStorage.load(0, config)) {
    // Jika gagal (data baru/kosong), set default
    config.lampState = false;
    config.bootCount = 0;
  }
  
  config.bootCount++;
  IskakStorage.save(0, config);
}
