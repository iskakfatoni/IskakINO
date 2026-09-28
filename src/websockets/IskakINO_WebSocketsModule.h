/*
 * src/websockets/IskakINO_WebSocketsModule.h
 *
 * Adapter IskakINO_WebSocketsServer untuk IskakINO_Kernel.
 * update() secara otomatis memanggil ws.tick() di setiap putaran loop.
 *
 * Hanya aktif pada platform yang memiliki konektivitas WiFi (ESP32 / ESP8266).
 */

#ifndef ISKAKINO_WEBSOCKETS_MODULE_H
#define ISKAKINO_WEBSOCKETS_MODULE_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_HAS_WIFI)

#include "../core/IskakINO_Module.h"
#include "IskakINO_WebSockets.h"

class IskakINO_WebSocketsModule : public IskakINO_Module {
private:
    IskakINO_WebSocketsServer& _ws;
    uint16_t _port;

public:
    explicit IskakINO_WebSocketsModule(IskakINO_WebSocketsServer& ws, uint16_t port = 81)
        : _ws(ws), _port(port) {}

    void begin() override {
        _ws.begin(_port);
    }

    void update() override {
        _ws.tick();
    }

    const char* moduleName() const override {
        return "WebSockets";
    }
};

#endif // defined(ISKAKINO_HAS_WIFI)
#endif // ISKAKINO_WEBSOCKETS_MODULE_H
