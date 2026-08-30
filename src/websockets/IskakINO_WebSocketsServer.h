/*
 * src/websockets/IskakINO_WebSocketsServer.h
 *
 * Server WebSocket multi-client non-blocking zero-dependency untuk ESP32 & ESP8266.
 * Mendukung broadcasting telemetri sensor dan kontrol dua arah real-time.
 *
 * Hanya aktif pada platform yang memiliki konektivitas WiFi (ESP32 / ESP8266).
 */

#ifndef ISKAKINO_WEBSOCKETS_SERVER_H
#define ISKAKINO_WEBSOCKETS_SERVER_H

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

#ifndef ISKAKINO_WS_MAX_CLIENTS
#define ISKAKINO_WS_MAX_CLIENTS 8
#endif

#ifndef ISKAKINO_WS_BUFFER_SIZE
#define ISKAKINO_WS_BUFFER_SIZE 1024
#endif

struct IskakWSClientSlot {
    WiFiClient client;
    IskakWSClientState state = IskakWSClientState::DISCONNECTED;
    unsigned long connectTime = 0;
    uint8_t rxBuffer[ISKAKINO_WS_BUFFER_SIZE];
    size_t rxLen = 0;
};

class IskakINO_WebSocketsServer {
private:
    uint16_t _port;
    WiFiServer* _server;
    IskakWSClientSlot _clients[ISKAKINO_WS_MAX_CLIENTS];
    IskakWSEventCallback _eventCallback;
    bool _running;

    void handleHandshake(uint8_t slotIdx);
    void handleFrames(uint8_t slotIdx);
    bool sendRawFrame(WiFiClient& client, IskakWSOpcode opcode, const uint8_t* payload, size_t len);

public:
    explicit IskakINO_WebSocketsServer(uint16_t port = 81);
    ~IskakINO_WebSocketsServer();

    bool begin(uint16_t port = 0);
    void stop();
    void tick();
    void loop() { tick(); }

    void onEvent(IskakWSEventCallback callback) {
        _eventCallback = callback;
    }

    bool broadcastText(const String& text);
    bool broadcastText(const char* text);
    bool broadcastBinary(const uint8_t* data, size_t len);

    bool sendText(uint8_t clientId, const String& text);
    bool sendText(uint8_t clientId, const char* text);
    bool sendBinary(uint8_t clientId, const uint8_t* data, size_t len);

    void disconnect(uint8_t clientId);
    void disconnectAll();

    uint8_t connectedClientsCount() const;
    bool isClientConnected(uint8_t clientId);
    uint16_t getPort() const { return _port; }
};

#endif // defined(ISKAKINO_HAS_WIFI)
#endif // ISKAKINO_WEBSOCKETS_SERVER_H
