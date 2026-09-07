/*
 * src/voice/IskakINO_SmartVoiceModule.h
 * Adapter tipis supaya IskakINO_SmartVoice bisa didaftarkan ke
 * IskakINO_Kernel. Protokol DFPlayer bersifat request/response (tidak ada
 * state machine yang perlu di-"tick" tiap loop()), jadi update() sengaja
 * kosong -- adapter ini murni supaya voice.begin(serial, busyPin, bootDelay)
 * ikut terpanggil otomatis lewat IskakINO.begin().
 */

#ifndef ISKAKINO_SMARTVOICE_MODULE_H
#define ISKAKINO_SMARTVOICE_MODULE_H

#include "../core/IskakINO_Module.h"
#include "IskakINO_SmartVoice.h"

class IskakINO_SmartVoiceModule : public IskakINO_Module {
  private:
    IskakINO_SmartVoice& _voice;
    Stream& _serial;
    uint8_t _busyPin;
    uint16_t _bootDelayMs;

  public:
    IskakINO_SmartVoiceModule(IskakINO_SmartVoice& voice, Stream& serial,
                               uint8_t busyPin = 255, uint16_t bootDelayMs = 500)
        : _voice(voice), _serial(serial), _busyPin(busyPin), _bootDelayMs(bootDelayMs) {}

    void begin() override { _voice.begin(_serial, _busyPin, _bootDelayMs); }
    // update() sengaja tidak di-override (default no-op dari IskakINO_Module)
    const char* moduleName() const override { return "SmartVoice"; }
};

#endif
