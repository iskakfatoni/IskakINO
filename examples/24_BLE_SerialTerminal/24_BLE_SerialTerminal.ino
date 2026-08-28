/*
 * 24_BLE_SerialTerminal.ino
 * Modul: IskakINO_BLE & IskakINO_ArduFast (HANYA ESP32)
 *
 * Menunjukkan:
 *   1. Bluetooth Low Energy (BLE) Nordic UART Service (NUS)
 *   2. Komunikasi dua arah nirkabel antara ESP32 dan Smartphone (Android / iOS)
 *   3. Pengiriman telemetri realtime via BLE notify
 *   4. Penanganan perintah interaktif via onCommand (/relay, /status, /help)
 */

#include <IskakINO.h>

#if !defined(ISKAKINO_PLATFORM_ESP32)
  #error "Sketsa ini hanya mendukung board ESP32."
#endif

IskakINO_ArduFast fast;
IskakINO_BLE      ble;

// Handler perintah /relay [ON/OFF]
void onRelayCommand(const String& cmd, const String& args) {
    fast.logf(F("[BLE Cmd] Perintah: %s | Argumen: %s"), cmd.c_str(), args.c_str());

    if (args.equalsIgnoreCase("ON")) {
        ble.send("[Relay] Relay dinyalakan: ON\n");
    } else if (args.equalsIgnoreCase("OFF")) {
        ble.send("[Relay] Relay dimatikan: OFF\n");
    } else {
        ble.send("[Relay] Gunakan: /relay ON atau /relay OFF\n");
    }
}

// Handler perintah /status
void onStatusCommand(const String& cmd, const String& args) {
    String reply = "\n=== STATUS ESP32 ===\n";
    reply += "Uptime: " + String(millis() / 1000) + " detik\n";
    reply += "Free RAM: " + String(ESP.getFreeHeap() / 1024) + " KB\n";
    ble.send(reply);
}

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("   IskakINO - ESP32 BLE UART Terminal   "));
    fast.log(F("========================================"));

    // Inisialisasi BLE dengan nama perangkat
    ble.begin("IskakINO-ESP32");

    // Daftarkan handler perintah BLE
    ble.onCommand("/relay", onRelayCommand);
    ble.onCommand("/status", onStatusCommand);

    // Callback untuk menerima raw text apa pun
    ble.onData([](const String& data) {
        fast.logf(F("[BLE Data In] %s"), data.c_str());
    });
}

void loop() {
    // Jalankan loop BLE untuk reconnect handling
    ble.tick();

    // Kirim heartbeat / uptime ke smartphone setiap 5 detik saat terhubung
    if (ble.isConnected() && fast.every(5000, 0)) {
        ble.sendf("[Heartbeat] Uptime: %lu s | Free RAM: %d KB\n",
                  millis() / 1000, ESP.getFreeHeap() / 1024);
    }
}
