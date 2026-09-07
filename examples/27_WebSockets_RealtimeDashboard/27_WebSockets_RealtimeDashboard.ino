/*
 * examples/27_WebSockets_RealtimeDashboard/27_WebSockets_RealtimeDashboard.ino
 *
 * Contoh Demonstrasi Modul IskakINO_WebSockets:
 * Real-Time Bidirectional Streaming antara ESP32/ESP8266 dan Web Dashboard Browser.
 *
 * Fitur:
 * 1. WebServer (Port 80): Menyajikan antarmuka Dashboard HTML5 + CSS modern.
 * 2. WebSockets (Port 81): Jalur streaming full-duplex (<10ms latensi) tanpa HTTP polling.
 * 3. Broadcasting telemetry sensor periodik menggunakan IskakINO_JSON.
 * 4. Kontrol relay / LED instan dari tombol web ke mikrokontroler.
 *
 * Hardware:
 * - ESP32 atau ESP8266
 * - LED / Relay di pin GPIO 2 (LED_BUILTIN)
 */

#include <IskakINO.h>

#if defined(ISKAKINO_HAS_WIFI)

#if defined(ISKAKINO_PLATFORM_ESP32)
  #include <WiFi.h>
  #include <WebServer.h>
  typedef WebServer IskakHttpServer;
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESP8266WebServer.h>
  typedef ESP8266WebServer IskakHttpServer;
#endif

// Konfigurasi WiFi
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

IskakHttpServer server(80);
IskakINO_WebSocketsServer ws(81);

const int RELAY_PIN = 2; // LED_BUILTIN / Relay
bool relayState = false;
unsigned long lastTelemetryMs = 0;

// Halaman HTML Dashboard Modern
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IskakINO Real-Time WebSocket Dashboard</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Segoe UI', Tahoma, sans-serif; }
        body { background: #0f172a; color: #f8fafc; display: flex; justify-content: center; padding: 20px; }
        .container { width: 100%; max-width: 600px; }
        .header { text-align: center; margin-bottom: 24px; }
        .header h1 { font-size: 24px; color: #818cf8; margin-bottom: 6px; }
        .badge { display: inline-block; padding: 4px 12px; border-radius: 20px; font-size: 12px; font-weight: 600; }
        .badge-online { background: #065f46; color: #34d399; }
        .badge-offline { background: #881337; color: #fb7185; }
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 16px; margin-bottom: 16px; }
        .card { background: #1e293b; border: 1px solid #334155; border-radius: 12px; padding: 18px; text-align: center; }
        .card-val { font-size: 32px; font-weight: 700; color: #38bdf8; margin: 8px 0; }
        .card-label { font-size: 13px; color: #94a3b8; text-transform: uppercase; letter-spacing: 0.5px; }
        .btn-toggle { width: 100%; padding: 14px; border: none; border-radius: 10px; font-size: 16px; font-weight: 700; cursor: pointer; transition: 0.2s; }
        .btn-off { background: #334155; color: #94a3b8; }
        .btn-on { background: #4f46e5; color: #ffffff; box-shadow: 0 0 15px rgba(99, 102, 241, 0.5); }
        .log-box { background: #020617; border: 1px solid #1e293b; border-radius: 8px; padding: 12px; font-family: monospace; font-size: 12px; color: #a5f3fc; height: 120px; overflow-y: auto; text-align: left; }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>⚡ IskakINO WebSockets</h1>
            <span id="wsStatus" class="badge badge-offline">Connecting...</span>
        </div>
        <div class="grid">
            <div class="card">
                <div class="card-label">Suhu Sensor</div>
                <div id="tempVal" class="card-val">--.- °C</div>
            </div>
            <div class="card">
                <div class="card-label">Kelembapan</div>
                <div id="humVal" class="card-val">-- %</div>
            </div>
        </div>
        <div class="card" style="margin-bottom: 16px;">
            <div class="card-label" style="margin-bottom: 12px;">Kontrol Relay / LED</div>
            <button id="btnRelay" class="btn-toggle btn-off" onclick="toggleRelay()">RELAY: OFF</button>
        </div>
        <div class="card">
            <div class="card-label" style="margin-bottom: 8px;">Live Stream Log (< 10 ms Latency)</div>
            <div id="logBox" class="log-box"></div>
        </div>
    </div>

    <script>
        let ws;
        const wsStatus = document.getElementById('wsStatus');
        const tempVal = document.getElementById('tempVal');
        const humVal = document.getElementById('humVal');
        const btnRelay = document.getElementById('btnRelay');
        const logBox = document.getElementById('logBox');
        let isRelayOn = false;

        function log(msg) {
            logBox.innerHTML += '<div>[' + new Date().toLocaleTimeString() + '] ' + msg + '</div>';
            logBox.scrollTop = logBox.scrollHeight;
        }

        function initWebSocket() {
            ws = new WebSocket('ws://' + window.location.hostname + ':81/');
            ws.onopen = function() {
                wsStatus.className = 'badge badge-online';
                wsStatus.innerText = 'WS CONNECTED';
                log('Terhubung ke IskakINO WebSocket Server!');
            };
            ws.onclose = function() {
                wsStatus.className = 'badge badge-offline';
                wsStatus.innerText = 'DISCONNECTED';
                log('Koneksi terputus. Mencoba reconnect...');
                setTimeout(initWebSocket, 2000);
            };
            ws.onmessage = function(event) {
                try {
                    const data = JSON.parse(event.data);
                    if (data.temp !== undefined) tempVal.innerText = data.temp.toFixed(1) + ' °C';
                    if (data.hum !== undefined) humVal.innerText = data.hum.toFixed(0) + ' %';
                    if (data.relay !== undefined) {
                        isRelayOn = data.relay;
                        btnRelay.className = isRelayOn ? 'btn-toggle btn-on' : 'btn-toggle btn-off';
                        btnRelay.innerText = isRelayOn ? 'RELAY: ON' : 'RELAY: OFF';
                    }
                    log('RX: ' + event.data);
                } catch(e) {
                    log('RX String: ' + event.data);
                }
            };
        }

        function toggleRelay() {
            isRelayOn = !isRelayOn;
            const payload = JSON.stringify({ action: "setRelay", state: isRelayOn });
            ws.send(payload);
            log('TX: ' + payload);
        }

        window.onload = initWebSocket;
    </script>
</body>
</html>
)rawliteral";

void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    Serial.println("\n[IskakINO] Memulai 27_WebSockets_RealtimeDashboard...");

    // Hubungkan WiFi (atau gunakan IskakINO_WifiPortal)
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    Serial.print("Menghubungkan ke WiFi");
    unsigned long startMs = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startMs < 10000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Terhubung! IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\n[WiFi] Gagal konek, memulai Access Point...");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("IskakINO-WS-Demo", "12345678");
        Serial.println("[AP] IP: " + WiFi.softAPIP().toString());
    }

    // 1. Setup HTTP Web Server (Port 80)
    server.on("/", []() {
        server.send_P(200, "text/html", INDEX_HTML);
    });
    server.begin();
    Serial.println("[HTTP] Server siap di port 80");

    // 2. Setup WebSocket Server (Port 81)
    ws.onEvent([](uint8_t clientId, IskakWSEvent type, uint8_t* payload, size_t len) {
        if (type == IskakWSEvent::CONNECTED) {
            Serial.printf("[WS] Client #%u Terhubung!\n", clientId);
            // Kirim status awal relay ke client baru
            IskakJSONBuilder initJson;
            initJson.beginObject();
            initJson.add("relay", relayState);
            initJson.endObject();
            ws.sendText(clientId, initJson.toString());
        } else if (type == IskakWSEvent::DISCONNECTED) {
            Serial.printf("[WS] Client #%u Terputus.\n", clientId);
        } else if (type == IskakWSEvent::TEXT) {
            Serial.printf("[WS] Client #%u Pesan: %s\n", clientId, (char*)payload);

            // Parse perintah JSON menggunakan IskakJSONReader
            IskakJSONReader cmd((char*)payload);
            String action = cmd.getString("action");
            if (action == "setRelay") {
                relayState = cmd.getBool("state");
                digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);

                // Broadcast status baru ke semua client yang terhubung
                IskakJSONBuilder resp;
                resp.beginObject();
                resp.add("relay", relayState);
                resp.endObject();
                ws.broadcastText(resp.toString());
            }
        }
    });

    ws.begin(81);
    Serial.println("[WS] WebSocket Server siap di port 81");
}

void loop() {
    server.handleClient();
    ws.tick(); // Non-blocking event pump

    // Streaming telemetri sensor setiap 500 ms secara real-time
    if (millis() - lastTelemetryMs >= 500) {
        lastTelemetryMs = millis();

        if (ws.connectedClientsCount() > 0) {
            // Simulasi pembacaan sensor (atau ganti dengan sensor DHT/DS18B20 nyata)
            float temp = 28.5 + (random(-15, 15) / 10.0);
            float hum = 60.0 + (random(-50, 50) / 10.0);

            IskakJSONBuilder tele;
            tele.beginObject();
            tele.add("temp", temp, 1);
            tele.add("hum", hum, 1);
            tele.add("relay", relayState);
            tele.endObject();

            ws.broadcastText(tele.toString());
        }
    }
}

#else

void setup() {
    Serial.begin(115200);
    Serial.println(F("[IskakINO] Modul WebSockets khusus untuk platform ESP32 & ESP8266."));
}

void loop() {
}

#endif
