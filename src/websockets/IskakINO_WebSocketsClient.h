/*
 * src/websockets/IskakINO_WebSocketsClient.h
 *
 * Client WebSocket RFC 6455 zero-dependency untuk ESP32 & ESP8266.
 * Mendukung auto-reconnect, streaming telemetri ke Cloud backend, dan frame masking.
 *
 * Hanya aktif pada platform yang memiliki konektivitas WiFi (ESP32 / ESP8266).
 */

#ifndef ISKAKINO_WEBSOCKETS_CLIENT_H
#define ISKAKINO_WEBSOCKETS_CLIENT_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_HAS_WIFI)

#include <Arduino.h>
#if defined(ISKAKINO_PLATFORM_ESP32)
  #include <WiFi.h>
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include "IskakINO_WS_Frame.h"
#include "IskakINO_WS_SHA1.h"

#ifndef ISKAKINO_WS_CLIENT_BUFFER_SIZE
#define ISKAKINO_WS_CLIENT_BUFFER_SIZE 1024
#endif

class IskakINO_WebSocketsClient {
private:
    String _host;
    uint16_t _port;
    String _url;
    WiFiClient _client;
    IskakWSClientState _state;
    IskakWSEventCallback _eventCallback;

    unsigned long _lastReconnectAttempt;
    unsigned long _reconnectInterval;
    bool _autoReconnect;

    uint8_t _rxBuffer[ISKAKINO_WS_CLIENT_BUFFER_SIZE];
    String _expectedAcceptKey;

    bool performHandshake();
    void handleFrames();
    bool sendRawFrame(IskakWSOpcode opcode, const uint8_t* payload, size_t len);

public:
    IskakINO_WebSocketsClient();
    ~IskakINO_WebSocketsClient();

    void begin(const char* host, uint16_t port = 80, const char* url = "/");
    void begin(const String& host, uint16_t port = 80, const String& url = "/") {
        begin(host.c_str(), port, url.c_str());
    }

    void disconnect();
    void tick();
    void loop() { tick(); }

    bool isConnected() {
        return _state == IskakWSClientState::CONNECTED && _client.connected();
    }

    void onEvent(IskakWSEventCallback callback) {
        _eventCallback = callback;
    }

    bool sendText(const String& text);
    bool sendText(const char* text);
    bool sendBinary(const uint8_t* data, size_t len);

    void setReconnectInterval(unsigned long intervalMs) {
        _reconnectInterval = intervalMs;
    }

    void setAutoReconnect(bool enable) {
        _autoReconnect = enable;
    }
};

#endif // defined(ISKAKINO_HAS_WIFI)
#endif // ISKAKINO_WEBSOCKETS_CLIENT_H
