#include "IskakINO_MQTT.h"

#if defined(ISKAKINO_HAS_WIFI)

#define MQTT_CONNECT     0x10
#define MQTT_CONNACK     0x20
#define MQTT_PUBLISH     0x30
#define MQTT_PUBACK      0x40
#define MQTT_SUBSCRIBE   0x82
#define MQTT_SUBACK      0x90
#define MQTT_UNSUBSCRIBE 0xA2
#define MQTT_UNSUBACK    0xB0
#define MQTT_PINGREQ     0xC0
#define MQTT_PINGRESP    0xD0
#define MQTT_DISCONNECT  0xE0

#define ISKAKINO_SCHED_PING     0
#define ISKAKINO_SCHED_RECONN   1

IskakINO_MQTT::IskakINO_MQTT() {
    _logger.setDebug(true);
}

IskakINO_MQTT::~IskakINO_MQTT() {
    disconnect();
}

void IskakINO_MQTT::begin(const char* broker, uint16_t port,
                          const char* clientId,
                          const char* username,
                          const char* password) {
    _broker = broker;
    _port = port;
    _username = username;
    _password = password;

    if (clientId && strlen(clientId) > 0) {
        strncpy(_clientId, clientId, sizeof(_clientId) - 1);
        _clientId[sizeof(_clientId) - 1] = '\0';
    } else {
        // Auto-generate unique client ID dari MAC address
        uint8_t mac[6];
        WiFi.macAddress(mac);
        snprintf(_clientId, sizeof(_clientId), "IskakINO_%02X%02X%02X", mac[3], mac[4], mac[5]);
    }
}

void IskakINO_MQTT::setAutoReconnect(bool autoReconnect, uint32_t intervalMs) {
    _autoReconnect = autoReconnect;
    _reconnectIntervalMs = intervalMs;
}

void IskakINO_MQTT::setWill(const char* topic, const char* message, bool retain, uint8_t qos) {
    _willTopic = topic;
    _willMessage = message;
    _willRetain = retain;
    _willQos = qos;
}

uint16_t IskakINO_MQTT::nextPacketId() {
    _packetId++;
    if (_packetId == 0) _packetId = 1;
    return _packetId;
}

bool IskakINO_MQTT::writePacket(uint8_t header, const uint8_t* varHeader, size_t varLen, const uint8_t* payload, size_t payLen) {
    if (!_client.connected()) {
        _lastError = IskakINO_Result::NOT_CONNECTED;
        return false;
    }

    size_t remLen = varLen + payLen;
    uint8_t lenBuf[4];
    uint8_t lenBytes = 0;
    do {
        uint8_t d = remLen % 128;
        remLen /= 128;
        if (remLen > 0) d |= 0x80;
        lenBuf[lenBytes++] = d;
    } while (remLen > 0);

    _client.write(header);
    _client.write(lenBuf, lenBytes);
    if (varLen > 0 && varHeader) {
        _client.write(varHeader, varLen);
    }
    if (payLen > 0 && payload) {
        _client.write(payload, payLen);
    }
    _client.flush();
    _lastError = IskakINO_Result::OK;
    return true;
}

bool IskakINO_MQTT::connect() {
    if (!_broker || strlen(_broker) == 0) {
        _lastError = IskakINO_Result::INVALID_ARG;
        return false;
    }

    if (WiFi.status() != WL_CONNECTED) {
        _lastError = IskakINO_Result::NOT_CONNECTED;
        return false;
    }

    if (_client.connected()) {
        _state = IskakMQTTState::CONNECTED;
        return true;
    }

    _state = IskakMQTTState::CONNECTING;
    _logger.logf(F("[IskakINO MQTT] Menghubungkan ke broker %s:%d..."), _broker, _port);

    if (!_client.connect(_broker, _port)) {
        _state = IskakMQTTState::DISCONNECTED;
        _lastError = IskakINO_Result::NOT_CONNECTED;
        _logger.log(F("[IskakINO MQTT] Gagal membuka koneksi TCP ke broker."));
        return false;
    }

    // Bangun Paket CONNECT
    // Variable Header
    uint8_t varHeader[10];
    varHeader[0] = 0x00; varHeader[1] = 0x04; // Protocol name length
    varHeader[2] = 'M';  varHeader[3] = 'Q';  varHeader[4] = 'T'; varHeader[5] = 'T';
    varHeader[6] = 0x04; // Protocol level (MQTT 3.1.1)

    uint8_t connFlags = 0x02; // Clean session
    if (_willTopic && strlen(_willTopic) > 0) {
        connFlags |= 0x04; // Will flag
        connFlags |= ((_willQos & 0x03) << 3);
        if (_willRetain) connFlags |= 0x20;
    }
    if (_username && strlen(_username) > 0) connFlags |= 0x80;
    if (_password && strlen(_password) > 0) connFlags |= 0x40;
    varHeader[7] = connFlags;

    varHeader[8] = (uint8_t)(_keepAliveSec >> 8);
    varHeader[9] = (uint8_t)(_keepAliveSec & 0xFF);

    // Payload (Client ID, Will Topic/Msg, Username, Password)
    size_t payloadCap = 512;
    uint8_t* payBuf = (uint8_t*)malloc(payloadCap);
    if (!payBuf) {
        _client.stop();
        _state = IskakMQTTState::DISCONNECTED;
        _lastError = IskakINO_Result::WRITE_FAILED;
        return false;
    }

    size_t pIdx = 0;
    // Client ID
    uint16_t cLen = strlen(_clientId);
    payBuf[pIdx++] = (uint8_t)(cLen >> 8);
    payBuf[pIdx++] = (uint8_t)(cLen & 0xFF);
    memcpy(payBuf + pIdx, _clientId, cLen);
    pIdx += cLen;

    // Will
    if (_willTopic && strlen(_willTopic) > 0) {
        uint16_t wtLen = strlen(_willTopic);
        payBuf[pIdx++] = (uint8_t)(wtLen >> 8);
        payBuf[pIdx++] = (uint8_t)(wtLen & 0xFF);
        memcpy(payBuf + pIdx, _willTopic, wtLen);
        pIdx += wtLen;

        uint16_t wmLen = _willMessage ? strlen(_willMessage) : 0;
        payBuf[pIdx++] = (uint8_t)(wmLen >> 8);
        payBuf[pIdx++] = (uint8_t)(wmLen & 0xFF);
        if (wmLen > 0) memcpy(payBuf + pIdx, _willMessage, wmLen);
        pIdx += wmLen;
    }

    // Username
    if (_username && strlen(_username) > 0) {
        uint16_t uLen = strlen(_username);
        payBuf[pIdx++] = (uint8_t)(uLen >> 8);
        payBuf[pIdx++] = (uint8_t)(uLen & 0xFF);
        memcpy(payBuf + pIdx, _username, uLen);
        pIdx += uLen;
    }

    // Password
    if (_password && strlen(_password) > 0) {
        uint16_t passLen = strlen(_password);
        payBuf[pIdx++] = (uint8_t)(passLen >> 8);
        payBuf[pIdx++] = (uint8_t)(passLen & 0xFF);
        memcpy(payBuf + pIdx, _password, passLen);
        pIdx += passLen;
    }

    bool ok = writePacket(MQTT_CONNECT, varHeader, 10, payBuf, pIdx);
    free(payBuf);
    if (!ok) {
        _client.stop();
        _state = IskakMQTTState::DISCONNECTED;
        return false;
    }

    // Tunggu CONNACK (timeout 3s)
    unsigned long startMs = millis();
    while (_client.available() < 4) {
        if (millis() - startMs > 3000) {
            _logger.log(F("[IskakINO MQTT] Timeout menunggu CONNACK dari broker."));
            _client.stop();
            _state = IskakMQTTState::DISCONNECTED;
            _lastError = IskakINO_Result::TIMEOUT;
            return false;
        }
        delay(10);
    }

    uint8_t respHead = _client.read();
    uint8_t respLen  = _client.read();
    uint8_t ackFlags = _client.read();
    uint8_t retCode  = _client.read();
    (void)respLen;
    (void)ackFlags;

    if (respHead == MQTT_CONNACK && retCode == 0x00) {
        _state = IskakMQTTState::CONNECTED;
        _logger.logf(F("[IskakINO MQTT] Terhubung ke broker! Client ID: %s"), _clientId);
        _scheduler.reset(ISKAKINO_SCHED_PING);
        resubscribeAll();
        return true;
    } else {
        _logger.logf(F("[IskakINO MQTT] Ditolak oleh broker (return code: 0x%02X)"), retCode);
        _client.stop();
        _state = IskakMQTTState::DISCONNECTED;
        _lastError = IskakINO_Result::WRITE_FAILED;
        return false;
    }
}

void IskakINO_MQTT::disconnect() {
    if (_client.connected()) {
        uint8_t d = 0;
        writePacket(MQTT_DISCONNECT, nullptr, 0, nullptr, 0);
        _client.stop();
    }
    _state = IskakMQTTState::DISCONNECTED;
}

bool IskakINO_MQTT::isConnected() {
    return (_state == IskakMQTTState::CONNECTED && _client.connected());
}

bool IskakINO_MQTT::publish(const char* topic, const char* payload, bool retain, uint8_t qos) {
    if (!payload) return publish(topic, (const uint8_t*)"", 0, retain, qos);
    return publish(topic, (const uint8_t*)payload, strlen(payload), retain, qos);
}

bool IskakINO_MQTT::publish(const char* topic, const uint8_t* payload, size_t length, bool retain, uint8_t qos) {
    if (!isConnected() || !topic || strlen(topic) == 0) {
        _lastError = IskakINO_Result::NOT_CONNECTED;
        return false;
    }

    uint8_t header = MQTT_PUBLISH;
    if (retain) header |= 0x01;
    if (qos > 0) header |= ((qos & 0x03) << 1);

    uint16_t tLen = strlen(topic);
    uint8_t varHeader[68];
    size_t vLen = 0;

    varHeader[vLen++] = (uint8_t)(tLen >> 8);
    varHeader[vLen++] = (uint8_t)(tLen & 0xFF);
    memcpy(varHeader + vLen, topic, tLen);
    vLen += tLen;

    if (qos > 0) {
        uint16_t pId = nextPacketId();
        varHeader[vLen++] = (uint8_t)(pId >> 8);
        varHeader[vLen++] = (uint8_t)(pId & 0xFF);
    }

    return writePacket(header, varHeader, vLen, payload, length);
}

bool IskakINO_MQTT::subscribe(const char* topic, IskakMQTTCallback callback, uint8_t qos) {
    if (!topic || strlen(topic) == 0) return false;

    // Simpan ke tabel langganan
    int slot = -1;
    for (int i = 0; i < _subCount; i++) {
        if (strcmp(_subs[i].topic, topic) == 0) {
            slot = i;
            break;
        }
    }
    if (slot == -1 && _subCount < ISKAKINO_MQTT_MAX_SUBS) {
        slot = _subCount++;
    }

    if (slot >= 0) {
        strncpy(_subs[slot].topic, topic, sizeof(_subs[slot].topic) - 1);
        _subs[slot].topic[sizeof(_subs[slot].topic) - 1] = '\0';
        _subs[slot].callback = callback;
    }

    if (!isConnected()) return true; // Akan di-subscribe ulang otomatis saat connect

    uint16_t pId = nextPacketId();
    uint16_t tLen = strlen(topic);

    uint8_t varHeader[2];
    varHeader[0] = (uint8_t)(pId >> 8);
    varHeader[1] = (uint8_t)(pId & 0xFF);

    uint8_t payload[70];
    size_t pIdx = 0;
    payload[pIdx++] = (uint8_t)(tLen >> 8);
    payload[pIdx++] = (uint8_t)(tLen & 0xFF);
    memcpy(payload + pIdx, topic, tLen);
    pIdx += tLen;
    payload[pIdx++] = (qos & 0x03);

    return writePacket(MQTT_SUBSCRIBE, varHeader, 2, payload, pIdx);
}

bool IskakINO_MQTT::unsubscribe(const char* topic) {
    if (!topic) return false;

    for (int i = 0; i < _subCount; i++) {
        if (strcmp(_subs[i].topic, topic) == 0) {
            for (int j = i; j < _subCount - 1; j++) {
                _subs[j] = _subs[j + 1];
            }
            _subCount--;
            break;
        }
    }

    if (!isConnected()) return true;

    uint16_t pId = nextPacketId();
    uint16_t tLen = strlen(topic);

    uint8_t varHeader[2];
    varHeader[0] = (uint8_t)(pId >> 8);
    varHeader[1] = (uint8_t)(pId & 0xFF);

    uint8_t payload[68];
    size_t pIdx = 0;
    payload[pIdx++] = (uint8_t)(tLen >> 8);
    payload[pIdx++] = (uint8_t)(tLen & 0xFF);
    memcpy(payload + pIdx, topic, tLen);
    pIdx += tLen;

    return writePacket(MQTT_UNSUBSCRIBE, varHeader, 2, payload, pIdx);
}

void IskakINO_MQTT::resubscribeAll() {
    for (int i = 0; i < _subCount; i++) {
        uint16_t pId = nextPacketId();
        uint16_t tLen = strlen(_subs[i].topic);
        uint8_t varHeader[2] = { (uint8_t)(pId >> 8), (uint8_t)(pId & 0xFF) };
        uint8_t payload[70];
        size_t pIdx = 0;
        payload[pIdx++] = (uint8_t)(tLen >> 8);
        payload[pIdx++] = (uint8_t)(tLen & 0xFF);
        memcpy(payload + pIdx, _subs[i].topic, tLen);
        pIdx += tLen;
        payload[pIdx++] = 0x00; // QoS 0
        writePacket(MQTT_SUBSCRIBE, varHeader, 2, payload, pIdx);
    }
}

void IskakINO_MQTT::sendPing() {
    if (isConnected()) {
        writePacket(MQTT_PINGREQ, nullptr, 0, nullptr, 0);
    }
}

void IskakINO_MQTT::handleIncoming() {
    if (!_client.available()) return;

    uint8_t header = _client.read();
    uint8_t type = header & 0xF0;

    // Remaining length decode
    size_t remLen = 0;
    size_t multiplier = 1;
    uint8_t digit;
    do {
        if (!_client.available()) delay(2);
        digit = _client.read();
        remLen += (digit & 127) * multiplier;
        multiplier *= 128;
    } while ((digit & 128) != 0);

    if (type == MQTT_PUBLISH) {
        // Baca Topic
        uint8_t tLenH = _client.read();
        uint8_t tLenL = _client.read();
        uint16_t topicLen = (tLenH << 8) | tLenL;
        char topic[65] = {0};
        for (uint16_t i = 0; i < topicLen && i < 64; i++) {
            topic[i] = (char)_client.read();
        }
        topic[64] = '\0';
        if (topicLen > 64) {
            for (uint16_t i = 64; i < topicLen; i++) _client.read();
        }

        size_t bytesRead = 2 + topicLen;
        uint8_t qos = (header >> 1) & 0x03;
        if (qos > 0) {
            uint8_t pIdH = _client.read();
            uint8_t pIdL = _client.read();
            bytesRead += 2;
            // Kirim PUBACK untuk QoS 1
            if (qos == 1) {
                uint8_t ackVar[2] = { pIdH, pIdL };
                writePacket(MQTT_PUBACK, ackVar, 2, nullptr, 0);
            }
        }

        size_t payLen = remLen - bytesRead;
        size_t readLen = (payLen < ISKAKINO_MQTT_MAX_BUFFER - 1) ? payLen : (ISKAKINO_MQTT_MAX_BUFFER - 1);
        for (size_t i = 0; i < readLen; i++) {
            _buffer[i] = _client.read();
        }
        _buffer[readLen] = '\0';
        // Buang sisa jika payload lebih besar dari buffer
        if (payLen > readLen) {
            for (size_t i = readLen; i < payLen; i++) _client.read();
        }

        // Panggil callback spesifik topic
        bool handled = false;
        for (int i = 0; i < _subCount; i++) {
            if (strcmp(_subs[i].topic, topic) == 0 && _subs[i].callback) {
                _subs[i].callback(topic, (const char*)_buffer, readLen);
                handled = true;
                break;
            }
        }
        if (!handled && _globalCallback) {
            _globalCallback(topic, (const char*)_buffer, readLen);
        }
    } else if (type == MQTT_PINGRESP) {
        // Ping response diterima
    } else {
        // Lewati sisa paket yang tidak ditangani khusus
        for (size_t i = 0; i < remLen; i++) {
            if (_client.available()) _client.read();
        }
    }
}

void IskakINO_MQTT::tick() {
    if (WiFi.status() != WL_CONNECTED) {
        _state = IskakMQTTState::DISCONNECTED;
        return;
    }

    if (!isConnected()) {
        _state = IskakMQTTState::DISCONNECTED;
        if (_autoReconnect && _scheduler.every(_reconnectIntervalMs, ISKAKINO_SCHED_RECONN)) {
            connect();
        }
        return;
    }

    // Handle pesan masuk
    while (_client.available() > 0) {
        handleIncoming();
    }

    // Keepalive Ping
    if (_scheduler.every((unsigned long)_keepAliveSec * 1000UL, ISKAKINO_SCHED_PING)) {
        sendPing();
    }
}

#endif // defined(ISKAKINO_HAS_WIFI)
