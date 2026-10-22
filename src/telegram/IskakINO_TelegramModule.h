#ifndef ISKAKINO_TELEGRAM_MODULE_H
#define ISKAKINO_TELEGRAM_MODULE_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_HAS_WIFI)

#include "../core/IskakINO_Module.h"
#include "IskakINO_Telegram.h"

class IskakINO_TelegramModule : public IskakINO_Module {
  private:
    IskakINO_Telegram& _telegram;
    const char* _botToken;
    const char* _defaultChatId;
    bool _polling;
    uint32_t _pollIntervalMs;

  public:
    explicit IskakINO_TelegramModule(IskakINO_Telegram& telegram,
                                     const char* botToken = nullptr,
                                     const char* defaultChatId = nullptr,
                                     bool polling = false,
                                     uint32_t pollIntervalMs = 3000)
        : _telegram(telegram), _botToken(botToken), _defaultChatId(defaultChatId),
          _polling(polling), _pollIntervalMs(pollIntervalMs) {}

    void begin() override {
        if (_botToken) {
            _telegram.begin(_botToken, _defaultChatId);
            if (_polling) {
                _telegram.enablePolling(true, _pollIntervalMs);
            }
        }
    }

    void update() override {
        _telegram.tick();
    }

    const char* moduleName() const override { return "Telegram"; }
};

#endif // defined(ISKAKINO_HAS_WIFI)

#endif // ISKAKINO_TELEGRAM_MODULE_H
