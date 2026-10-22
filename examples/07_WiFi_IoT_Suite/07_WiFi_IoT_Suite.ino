/*
 * 07_WiFi_IoT_Suite.ino
 * Modul: IskakINO_WifiPortal, IskakINO_WebSockets, IskakINO_MQTT, & IskakINO_Telegram
 * (Khusus Board WiFi: ESP32 & ESP8266)
 *
 * Demonstrasi arsitektur IoT lengkap:
 *  1. WifiPortal  : Konfigurasi SSID & Password WiFi via AP Captive Portal Web.
 *  2. WebSockets  : Streaming telemetri real-time dua arah ke browser web (port 81).
 *  3. MQTT Client : Publish telemetri ke broker cloud & penanganan remote subscribe.
 *  4. Telegram Bot: Pengiriman alert status & perintah interaktif (/status, /ping).
 *
 * Kompatibel: ESP32 & ESP8266.
 */

#include <IskakINO.h>

#if !defined(ISKAKINO_HAS_WIFI)
  #error "Sketsa ini hanya mendukung board ESP32 atau ESP8266."
#endif

#if defined(ISKAKINO_PLATFORM_ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  typedef WebServer IskakHttpServer;
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  typedef ESP8266WebServer IskakHttpServer;
#endif

IskakINO_ArduFast        fast;
IskakINO_WifiPortal      portal;
IskakHttpServer          httpServer(80);
IskakINO_WebSocketsServer wsServer(81);
IskakINO_MQTT            mqtt;
IskakINO_Telegram        telegramBot;

// Konfigurasi Cloud IoT (Ubah sesuai akun Anda)
const char* MQTT_BROKER   = "broker.hivemq.com";
const uint16_t MQTT_PORT  = 1883;
const char* TELEGRAM_BOT_TOKEN = "123456789:ABCdefGHIjklMNOpqrSTUvwxYZ";
const char* TELEGRAM_CHAT_ID   = "123456789";

bool relayState = false;

// HTML Dashboard Ringkas
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8"><title>IskakINO IoT Hub</title>
    <style>
        body { background:#0f172a; color:#f8fafc; font-family:sans-serif; text-align:center; padding:30px; }
        .card { background:#1e293b; padding:20px; border-radius:12px; display:inline-block; min-width:300px; }
        button { background:#3b82f6; color:#fff; border:none; padding:10px 20px; border-radius:8px; cursor:pointer; font-size:16px; }
    </style>
</head>
<body>
    <div class="card">
        <h2>IskakINO IoT Hub</h2>
        <p>Status WebSocket: <span id="wsStatus">Terhubung</span></p>
        <p>Uptime: <span id="uptime">0</span> s</p>
        <button onclick="toggleRelay()">Toggle Relay</button>
    </div>
    <script>
        var ws = new WebSocket('ws://' + location.hostname + ':81/');
        ws.onmessage = function(e) {
            var data = JSON.parse(e.data);
            if(data.uptime) document.getElementById('uptime').innerText = data.uptime;
        };
        function toggleRelay() { ws.send(JSON.stringify({cmd: "TOGGLE"})); }
    </script>
</body>
</html>
)rawliteral";

void setup() {
    fast.begin(115200);

    fast.log(F("========================================================="));
    fast.log(F("     IskakINO - WiFi, WebSockets, MQTT & Telegram Suite  "));
    fast.log(F("========================================================="));

    // 1. Inisialisasi Captive Portal
    portal.setPortalTimeout(180);
    portal.beginAsync("IskakINO-IoT-Node");

    // 2. HTTP Server untuk Web Dashboard
    httpServer.on("/", []() {
        httpServer.send_P(200, "text/html", INDEX_HTML);
    });
    httpServer.begin();

    // 3. WebSockets Server
    wsServer.begin();
    wsServer.onEvent([](uint8_t num, IskakWSEventType type, uint8_t* payload, size_t length) {
        if (type == IskakWSEventType::TEXT) {
            fast.logf(F("[WebSocket Client %u] Pesan: %s"), num, (char*)payload);
        }
    });

    // 4. Inisialisasi MQTT & Telegram
    mqtt.begin(MQTT_BROKER, MQTT_PORT);
    telegramBot.begin(TELEGRAM_BOT_TOKEN, TELEGRAM_CHAT_ID);

    fast.log(F("[System] Server siap. Jika belum terhubung WiFi, sambungkan ke AP 'IskakINO-IoT-Node'."));
}

void loop() {
    portal.tick();
    httpServer.handleClient();
    wsServer.loop();
    mqtt.tick();
    telegramBot.tick();

    // Broadcast telemetri tiap 2 detik jika sudah terhubung ke WiFi
    if (WiFi.status() == WL_CONNECTED && fast.every(2000, 0)) {
        IskakJSONBuilder builder;
        builder.beginObject();
        builder.add("uptime", (long)(millis() / 1000));
        builder.add("freeHeap", (long)ESP.getFreeHeap());
        builder.add("relay", relayState);
        builder.endObject();

        // Broadcast ke semua client WebSocket yang aktif
        wsServer.broadcastTXT(builder.c_str());

        // Publish ke MQTT Broker jika terhubung
        if (mqtt.isConnected()) {
            mqtt.publish("iskakino/telemetry", builder.c_str());
        }
    }
}
