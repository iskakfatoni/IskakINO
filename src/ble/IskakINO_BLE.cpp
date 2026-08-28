#include "IskakINO_BLE.h"

#if defined(ISKAKINO_PLATFORM_ESP32)

// Standar Nordic UART Service (NUS) UUIDs
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

IskakINO_BLE::IskakINO_BLE() {
    _logger.setDebug(true);
}

IskakINO_BLE::~IskakINO_BLE() {
    if (_deviceConnected && _pServer) {
        BLEDevice::deinit();
    }
}

bool IskakINO_BLE::begin(const char* deviceName) {
    if (deviceName && strlen(deviceName) > 0) {
        _deviceName = deviceName;
    }

    _logger.logf(F("[IskakINO BLE] Menginisialisasi BLE Device: '%s'"), _deviceName);

    // 1. Inisialisasi BLE Device
    BLEDevice::init(_deviceName);

    // 2. Buat BLE Server
    _pServer = BLEDevice::createServer();
    _pServer->setCallbacks(this);

    // 3. Buat BLE Service (Nordic UART Service)
    BLEService* pService = _pServer->createService(SERVICE_UUID);

    // 4. Buat TX Characteristic (Notify dari ESP32 ke Smartphone)
    _pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    _pTxCharacteristic->addDescriptor(new BLE2902());

    // 5. Buat RX Characteristic (Write dari Smartphone ke ESP32)
    _pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    _pRxCharacteristic->setCallbacks(this);

    // 6. Mulai Service & Advertising
    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    _logger.log(F("[IskakINO BLE] Advertising aktif. Menunggu koneksi smartphone..."));
    _lastError = IskakINO_Result::OK;
    return true;
}

void IskakINO_BLE::onConnect(BLEServer* pServer) {
    _deviceConnected = true;
    _logger.log(F("[IskakINO BLE] Smartphone terhubung!"));
}

void IskakINO_BLE::onDisconnect(BLEServer* pServer) {
    _deviceConnected = false;
    _logger.log(F("[IskakINO BLE] Smartphone terputus."));
}

void IskakINO_BLE::onWrite(BLECharacteristic* pCharacteristic) {
    String rxValue = pCharacteristic->getValue().c_str();

    if (rxValue.length() > 0) {
        // Bersihkan whitespace/newline di akhir
        rxValue.trim();

        // Panggil global data callback
        if (_dataCallback) {
            _dataCallback(rxValue);
        }

        // Cek command dispatcher
        dispatchCommand(rxValue);
    }
}

void IskakINO_BLE::onCommand(const char* cmd, BLECmdCallback callback) {
    if (!cmd || _cmdCount >= ISKAKINO_BLE_MAX_CMDS) return;
    strncpy(_commands[_cmdCount].command, cmd, sizeof(_commands[_cmdCount].command) - 1);
    _commands[_cmdCount].command[sizeof(_commands[_cmdCount].command) - 1] = '\0';
    _commands[_cmdCount].callback = callback;
    _cmdCount++;
}

void IskakINO_BLE::dispatchCommand(const String& incoming) {
    int spaceIdx = incoming.indexOf(' ');
    String cmd = (spaceIdx != -1) ? incoming.substring(0, spaceIdx) : incoming;
    String args = (spaceIdx != -1) ? incoming.substring(spaceIdx + 1) : "";

    for (int i = 0; i < _cmdCount; i++) {
        if (cmd.equalsIgnoreCase(_commands[i].command) && _commands[i].callback) {
            _commands[i].callback(cmd, args);
            break;
        }
    }
}

void IskakINO_BLE::send(const char* text) {
    if (!_deviceConnected || !_pTxCharacteristic || !text) return;
    _pTxCharacteristic->setValue(text);
    _pTxCharacteristic->notify();
}

void IskakINO_BLE::send(const String& text) {
    send(text.c_str());
}

void IskakINO_BLE::sendf(const char* format, ...) {
    if (!_deviceConnected || !_pTxCharacteristic || !format) return;
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    send(buffer);
}

void IskakINO_BLE::tick() {
    // Auto-restart advertising saat perangkat terputus
    if (!_deviceConnected && _oldDeviceConnected) {
        delay(500); // Beri jeda sebentar untuk BLE stack
        _pServer->startAdvertising();
        _logger.log(F("[IskakINO BLE] Advertising ulang dimulai..."));
        _oldDeviceConnected = _deviceConnected;
    }

    if (_deviceConnected && !_oldDeviceConnected) {
        _oldDeviceConnected = _deviceConnected;
    }
}

#endif // defined(ISKAKINO_PLATFORM_ESP32)
