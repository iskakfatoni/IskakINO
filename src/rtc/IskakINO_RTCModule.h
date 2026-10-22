#ifndef ISKAKINO_RTC_MODULE_H
#define ISKAKINO_RTC_MODULE_H

#include "../core/IskakINO_Platform.h"
#include "../core/IskakINO_Module.h"
#include "IskakINO_RTC.h"

class IskakINO_RTCModule : public IskakINO_Module {
  private:
    IskakINO_RTC& _rtc;
    TwoWire& _wire;
    IskakRTCType _type;

  public:
    explicit IskakINO_RTCModule(IskakINO_RTC& rtc, TwoWire& wire = Wire, IskakRTCType type = IskakRTCType::AUTO)
        : _rtc(rtc), _wire(wire), _type(type) {}

    void begin() override {
        _rtc.begin(_wire, _type);
    }

    void update() override {
        _rtc.tick();
    }

    const char* moduleName() const override { return "RTC"; }
};

#endif // ISKAKINO_RTC_MODULE_H
