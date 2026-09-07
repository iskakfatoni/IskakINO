/*
 * src/relay/IskakINO_RelayModule.h
 * Adapter modular supaya IskakINO_Relay bisa didaftarkan ke IskakINO_Kernel.
 */

#ifndef ISKAKINO_RELAY_MODULE_H
#define ISKAKINO_RELAY_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_Relay.h"

class IskakINO_RelayModule : public IskakINO_Module {
  private:
    IskakINO_Relay& _relay;

  public:
    explicit IskakINO_RelayModule(IskakINO_Relay& relay)
        : _relay(relay) {}

    void begin() override {
        _relay.begin();
    }

    void update() override {
        _relay.update();
    }

    const char* moduleName() const override {
        return "Relay";
    }
};

#endif // ISKAKINO_RELAY_MODULE_H
