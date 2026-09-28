#ifndef ISKAKINO_MQTT_H
#define ISKAKINO_MQTT_H

#include <Arduino.h>

#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Scheduler.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Result.h"

#if defined(ISKAKINO_HAS_WIFI)

#if defined(ISKAKINO_PLATFORM_ESP32)
  #include <WiFi.h>
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <functional>

#define ISKAKINO_MQTT_MAX_BUFFER   512
#define ISKAKINO_MQTT_MAX_SUBS     8
#define ISKAKINO_MQTT_KEEPALIVE    15 // detik

typedef std::function<void(const char* topic, const char* payload, size_t length)> IskakMQTTCallback;

struct IskakMQTTSubscription {
    char topic[64];
    IskakMQTTCallback callback;
};

enum class IskakMQTTState : uint8_t {
    DISCONNECTED,
    CONNECTING,
    CONNECTED
};

class IskakINO_MQTT {
  public:
    IskakINO_MQTT();
    virtual ~IskakINO_MQTT();

    // Inisialisasi broker & kredensial
    void begin(const char* broker, uint16_t port = 1883,
               const char* clientId = nullptr,
               const char* username = nullptr,
               const char* password = nullptr);

    // Kontrol koneksi
    bool connect();
    void disconnect();
    bool isConnected();
    IskakMQTTState state() const { return _state; }

    // Publish & Subscribe
    bool publish(const char* topic, const char* payload, bool retain = false, uint8_t qos = 0);
    bool publish(const char* topic, const uint8_t* payload, size_t length, bool retain = false, uint8_t qos = 0);
    bool subscribe(const char* topic, IskakMQTTCallback callback = nullptr, uint8_t qos = 0);
    bool unsubscribe(const char* topic);

    // Global callback untuk pesan masuk
    void onMessage(IskakMQTTCallback callback) { _globalCallback = callback; }

    // Will / LWT (Last Will and Testament)
    void setWill(const char* topic, const char* message, bool retain = false, uint8_t qos = 0);

    // Task & Loop (wajib dipanggil di loop())
    void tick();

    // Konfigurasi
    void setKeepAlive(uint16_t seconds) { _keepAliveSec = seconds; }
    void setAutoReconnect(bool autoReconnect, uint32_t intervalMs = 5000);
    void setDebug(bool debug) { _logger.setDebug(debug); }
    IskakINO_Result lastError() const { return _lastError; }

  private:
    WiFiClient _client;
    IskakINO_Logger _logger;
    IskakINO_Scheduler _scheduler{2}; // id 0 = keepalive ping, id 1 = auto reconnect
    IskakINO_Result _lastError = IskakINO_Result::OK;

    const char* _broker = nullptr;
    uint16_t    _port = 1883;
    char        _clientId[33] = {0};
    const char* _username = nullptr;
    const char* _password = nullptr;

    const char* _willTopic = nullptr;
    const char* _willMessage = nullptr;
    bool        _willRetain = false;
    uint8_t     _willQos = 0;

    IskakMQTTState _state = IskakMQTTState::DISCONNECTED;
    uint16_t _packetId = 1;
    uint16_t _keepAliveSec = ISKAKINO_MQTT_KEEPALIVE;
    bool     _autoReconnect = true;
    uint32_t _reconnectIntervalMs = 5000;

    uint8_t  _buffer[ISKAKINO_MQTT_MAX_BUFFER];
    IskakMQTTSubscription _subs[ISKAKINO_MQTT_MAX_SUBS];
    uint8_t  _subCount = 0;
    IskakMQTTCallback _globalCallback = nullptr;

    // Helper protokol MQTT
    bool writePacket(uint8_t header, const uint8_t* varHeader, size_t varLen, const uint8_t* payload, size_t payLen);
    void handleIncoming();
    void sendPing();
    void resubscribeAll();
    uint16_t nextPacketId();
};

#endif // defined(ISKAKINO_HAS_WIFI)

#endif // ISKAKINO_MQTT_H
