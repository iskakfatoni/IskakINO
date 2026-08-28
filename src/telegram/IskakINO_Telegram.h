#ifndef ISKAKINO_TELEGRAM_H
#define ISKAKINO_TELEGRAM_H

#include <Arduino.h>

#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Scheduler.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Result.h"

#if defined(ISKAKINO_HAS_WIFI)

#if defined(ISKAKINO_PLATFORM_ESP32)
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
#elif defined(ISKAKINO_PLATFORM_ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecure.h>
#endif

#include <functional>

#define ISKAKINO_TELEGRAM_MAX_CMDS 8

typedef std::function<void(const char* chatId, const char* fromUser, const char* text)> TelegramMsgCallback;

struct IskakTelegramCommand {
    char command[32];
    TelegramMsgCallback callback;
};

class IskakINO_Telegram {
  public:
    IskakINO_Telegram();
    virtual ~IskakINO_Telegram();

    // Inisialisasi bot token & default Chat ID
    void begin(const char* botToken, const char* defaultChatId = nullptr);

    // Pengiriman Pesan
    bool sendMessage(const char* chatId, const char* text, const char* parseMode = "Markdown");
    bool sendMessage(const char* text);
    bool sendAlert(const char* title, const char* message, const char* emoji = "⚠️");

    // Penerimaan / Bot Command Handler
    void onCommand(const char* cmd, TelegramMsgCallback callback);
    void onMessage(TelegramMsgCallback callback) { _globalCallback = callback; }
    void enablePolling(bool enable, uint32_t intervalMs = 3000);

    // Loop & scheduler (wajib di loop())
    void tick();

    void setDebug(bool debug) { _logger.setDebug(debug); }
    IskakINO_Result lastError() const { return _lastError; }

  private:
    WiFiClientSecure _client;
    IskakINO_Logger _logger;
    IskakINO_Scheduler _scheduler{1}; // id 0 = polling updates
    IskakINO_Result _lastError = IskakINO_Result::OK;

    const char* _token = nullptr;
    const char* _defaultChatId = nullptr;

    bool     _pollingEnabled = false;
    uint32_t _pollIntervalMs = 3000;
    long     _lastUpdateId = 0;

    IskakTelegramCommand _commands[ISKAKINO_TELEGRAM_MAX_CMDS];
    uint8_t  _cmdCount = 0;
    TelegramMsgCallback _globalCallback = nullptr;

    bool sendPostRequest(const char* endpoint, const String& jsonPayload);
    void fetchUpdates();
    String urlEncode(const String& str);
};

#endif // defined(ISKAKINO_HAS_WIFI)

#endif // ISKAKINO_TELEGRAM_H
