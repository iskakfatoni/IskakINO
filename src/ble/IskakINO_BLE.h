#ifndef ISKAKINO_BLE_H
#define ISKAKINO_BLE_H

#include <Arduino.h>
#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Result.h"

#if defined(ISKAKINO_PLATFORM_ESP32)

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <functional>

#define ISKAKINO_BLE_MAX_CMDS 8

typedef std::function<void(const String& data)> BLEDataCallback;
typedef std::function<void(const String& cmd, const String& args)> BLECmdCallback;

struct IskakBLECommand {
    char command[32];
    BLECmdCallback callback;
};

class IskakINO_BLE : public BLEServerCallbacks, public BLECharacteristicCallbacks {
  public:
    IskakINO_BLE();
    virtual ~IskakINO_BLE();

    // Inisialisasi BLE Device & Nordic UART Service
    bool begin(const char* deviceName = "IskakINO-BLE");

    // Status Koneksi
    bool isConnected() const { return _deviceConnected; }

    // Kirim Data / Telemetri ke Smartphone
    void send(const char* text);
    void send(const String& text);
    void sendf(const char* format, ...);

    // Registrasi Handler Pesan & Perintah
    void onData(BLEDataCallback callback) { _dataCallback = callback; }
    void onCommand(const char* cmd, BLECmdCallback callback);

    // Loop & Scheduler
    void tick();

    void setDebug(bool debug) { _logger.setDebug(debug); }
    IskakINO_Result lastError() const { return _lastError; }

  private:
    const char* _deviceName = "IskakINO-BLE";
    bool _deviceConnected = false;
    bool _oldDeviceConnected = false;

    BLEServer*         _pServer = nullptr;
    BLECharacteristic* _pTxCharacteristic = nullptr;
    BLECharacteristic* _pRxCharacteristic = nullptr;

    IskakINO_Logger _logger;
    IskakINO_Result _lastError = IskakINO_Result::OK;

    BLEDataCallback _dataCallback = nullptr;
    IskakBLECommand _commands[ISKAKINO_BLE_MAX_CMDS];
    uint8_t _cmdCount = 0;

    // BLE Callbacks
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;
    void onWrite(BLECharacteristic* pCharacteristic) override;

    void dispatchCommand(const String& incoming);
};

#endif // defined(ISKAKINO_PLATFORM_ESP32)

#endif // ISKAKINO_BLE_H
