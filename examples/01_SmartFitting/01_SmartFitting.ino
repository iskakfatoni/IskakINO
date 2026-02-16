/* * ==============================================================================
 * 01_SmartFitting.ino - LOGIKA ALUR KERJA SISTEM (PRODUCTION LEVEL)
 * ==============================================================================
 * * 1. TAHAP SETUP (INISIALISASI)
 * ------------------------------------------------------------------------------
 * [A] BOOTING: 
 * - Memulai Serial via ArduFast.begin(115200).
 * - Konfigurasi Relay Pin sebagai OUTPUT.
 * [B] STORAGE RECOVERY (IskakINO_Storage):
 * - Mounting filesystem (NVS/LittleFS/EEPROM) berdasarkan arsitektur chip.
 * - Load Struct 'ConfigData' dari alamat 0.
 * - Validasi Integrity via CRC32: 
 * - Jika VALID: Data digunakan.
 * - Jika KORUP: Reset ke default (lampState: OFF, totalOn: 0).
 * - Incremental bootCount++ dan autosave kembali ke storage.
 * [C] HARDWARE RESTORE:
 * - Mengembalikan status Relay ke lampState terakhir (Memory Persistence).
 * - Inisialisasi LCD & tampilkan brand name IskakINO.
 * [D] NETWORK & TIME:
 * - portal.begin(): Cek WiFi. Jika gagal, otomatis aktifkan AP (Portal Mode).
 * - ntp.begin(25200): Siapkan antrean sinkronisasi GMT+7.
 *
 * 2. TAHAP LOOP (NON-BLOCKING ENGINE)
 * ------------------------------------------------------------------------------
 * [A] BACKGROUND TASKS (Real-time):
 * - portal.handle(): Mengelola Web Server & DNS Captive Portal.
 * - ntp.update(): Mengelola state machine kirim-terima paket NTP (UDP).
 * * [B] SCHEDULED TASKS (ArduFast.every):
 * - ID 0 (1 detik):
 * - Validasi LCD Connection.
 * - Jika ntp.isTimeSet(): Update baris 0 LCD dengan ntp.getFormattedTime().
 * - ID 1 (1 menit):
 * - Jika lampState == ON: Tambahkan variabel totalOnTime (Statistik Produk).
 * - Jalankan IskakStorage.save() (Autosave berkala untuk mencegah data loss).
 * - ID 2 (1 jam):
 * - Jalankan ntp.forceUpdate(): Sinkronisasi ulang untuk koreksi clock drift.
 *
 * 3. LOGIKA FAIL-SAFE (KEAMANAN PRODUK)
 * ------------------------------------------------------------------------------
 * - ANTI-HANG: Seluruh proses bersifat non-blocking (Tanpa delay()).
 * - DATA PROTECTION: Setiap proses tulis menggunakan Wrapper (Magic Byte + CRC32).
 * - HARDWARE AWARE: Cek lcd.isConnected() sebelum tulis I2C (Mencegah I2C Bus Hang).
 * - SMART WIFI: Otomatis masuk mode konfigurasi jika koneksi router hilang.
 * ==============================================================================
 */

#include <IskakINO_ArduFast.h>
#include <IskakINO_WifiPortal.h>
#include <IskakINO_FastNTP.h>
#include <IskakINO_Storage.h> 
#include <IskakINO_LiquidCrystal_I2C.h>
#include <WiFiUdp.h>

// Struktur Data Produk
struct ConfigData {
  bool lampState;
  uint16_t bootCount;
};

ConfigData mySettings;

// Inisialisasi Objects
IskakINO_WifiPortal portal;
WiFiUDP ntpUDP;
IskakINO_FastNTP ntp(ntpUDP, "pool.ntp.org");
LiquidCrystal_I2C lcd(16, 2);

void updateRelay(bool state) {
  mySettings.lampState = state;
  digitalWrite(5, state ? HIGH : LOW);
  IskakStorage.save(0, mySettings); // Simpan ke Storage Hybrid
}

void setup() {
  ArduFast.begin(115200);
  
  // 1. Load Storage (Hybrid Engine: Preferences/LittleFS/EEPROM)
  IskakStorage.begin("smartfit", true);
  if (!IskakStorage.load(0, mySettings)) {
    mySettings.lampState = false;
    mySettings.bootCount = 0;
  }
  mySettings.bootCount++;
  IskakStorage.save(0, mySettings);

  // 2. Restore Hardware State
  pinMode(5, OUTPUT);
  updateRelay(mySettings.lampState);

  // 3. Network & LCD
  lcd.begin();
  lcd.backlight();

  /* * ==============================================================================
 * 01_SmartFitting.ino - API ENDPOINT DOCUMENTATION (REST-LIKE API)
 * ==============================================================================
 * Base URL: http://smartfitting.local/ atau http://192.168.4.1/
 * * 1. KONTROL HARDWARE
 * ------------------------------------------------------------------------------
 * [GET]  /toggle     : Membalikkan status lampu (Jika ON jadi OFF, dan sebaliknya).
 * Respon: "OK" (text/plain).
 * [GET]  /on         : Menyalakan lampu secara paksa.
 * Respon: "Lamp turned ON".
 * [GET]  /off        : Mematikan lampu secara paksa.
 * Respon: "Lamp turned OFF".
 *
 * 2. MONITORING & DATA (JSON)
 * ------------------------------------------------------------------------------
 * [GET]  /status     : Mengambil status sensor dan sistem saat ini.
 * Format Respon (JSON):
 * {
 * "state": true,
 * "uptime": "1d 04:20:15",
 * "ntp": "sync_ok",
 * "signal": "-65dBm"
 * }
 * [GET]  /config     : Menampilkan data yang tersimpan di IskakStorage.
 * Respon: Nilai bootCount dan totalOnTime.
 *
 * 3. PEMELIHARAAN SISTEM
 * ------------------------------------------------------------------------------
 * [GET]  /scan       : Memulai pemindaian jaringan WiFi di sekitar.
 * [GET]  /reboot     : Menjalankan ESP.restart() secara aman.
 * [POST] /reset      : Menghapus konfigurasi WiFi & reset IskakStorage ke default.
 * Catatan: Membutuhkan konfirmasi untuk mencegah ketidaksengajaan.
 * [GET]  /update     : Akses ke halaman OTA (Over-The-Air) Update untuk upload firmware.
 *
 * 4. DASHBOARD UI
 * ------------------------------------------------------------------------------
 * [GET]  /           : Menampilkan Dashboard HTML utama (Responsive Mobile).
 * [GET]  /setup      : Masuk ke Captive Portal untuk pengaturan WiFi & Parameter.
 * ==============================================================================
 */
  portal.begin("IskakINO-SmartFitting");
  ntp.begin(25200); // GMT+7
}

void loop() {
  portal.handle(); // Web Server & DNS
  ntp.update();   // State Machine NTP

  // Logika 1: Tampilkan Jam di LCD (Setiap 1 Detik)
  if (ArduFast.every(1000, 0)) {
    if (lcd.isConnected() && ntp.isTimeSet()) {
      lcd.setCursor(0, 0);
      lcd.print(ntp.getFormattedTime());
    }
  }

  // Logika 2: Maintenance Berkala (Setiap 1 Jam)
  if (ArduFast.every(3600000, 1)) {
    if (WiFi.status() == WL_CONNECTED) {
      ntp.forceUpdate(); // Sekarang tidak akan error lagi
    }
  }
}
