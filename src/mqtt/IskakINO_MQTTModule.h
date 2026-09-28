#ifndef ISKAKINO_MQTT_MODULE_H
#define ISKAKINO_MQTT_MODULE_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_HAS_WIFI)

#include "../core/IskakINO_Module.h"
#include "IskakINO_MQTT.h"

class IskakINO_MQTTModule : public IskakINO_Module {
  private:
    IskakINO_MQTT& _mqtt;
    const char* _broker;
    uint16_t _port;
    const char* _clientId;
    const char* _username;
    const char* _password;

  public:
    explicit IskakINO_MQTTModule(IskakINO_MQTT& mqtt,
                                 const char* broker = nullptr,
                                 uint16_t port = 1883,
                                 const char* clientId = nullptr,
                                 const char* username = nullptr,
                                 const char* password = nullptr)
        : _mqtt(mqtt), _broker(broker), _port(port), _clientId(clientId),
          _username(username), _password(password) {}

    void begin() override {
        if (_broker) {
            _mqtt.begin(_broker, _port, _clientId, _username, _password);
        }
    }

    void update() override {
        _mqtt.tick();
    }

    const char* moduleName() const override { return "MQTT"; }
};

#endif // defined(ISKAKINO_HAS_WIFI)

#endif // ISKAKINO_MQTT_MODULE_H
