#include "IskakINO_Telegram.h"

#if defined(ISKAKINO_HAS_WIFI)

#define TELEGRAM_HOST "api.telegram.org"
#define TELEGRAM_PORT 443
#define ISKAKINO_SCHED_TELEGRAM_POLL 0

IskakINO_Telegram::IskakINO_Telegram() {
    _logger.setDebug(true);
    _client.setInsecure(); // Mengabaikan validasi root CA agar hemat RAM pada ESP
}

IskakINO_Telegram::~IskakINO_Telegram() {
    _client.stop();
}

void IskakINO_Telegram::begin(const char* botToken, const char* defaultChatId) {
    _token = botToken;
    _defaultChatId = defaultChatId;
    _client.setInsecure();
}

void IskakINO_Telegram::onCommand(const char* cmd, TelegramMsgCallback callback) {
    if (!cmd || _cmdCount >= ISKAKINO_TELEGRAM_MAX_CMDS) return;
    strncpy(_commands[_cmdCount].command, cmd, sizeof(_commands[_cmdCount].command) - 1);
    _commands[_cmdCount].command[sizeof(_commands[_cmdCount].command) - 1] = '\0';
    _commands[_cmdCount].callback = callback;
    _cmdCount++;
}

void IskakINO_Telegram::enablePolling(bool enable, uint32_t intervalMs) {
    _pollingEnabled = enable;
    _pollIntervalMs = intervalMs;
}

static String escapeJson(const String& raw) {
    String out = "";
    out.reserve(raw.length() + 8);
    for (size_t i = 0; i < raw.length(); i++) {
        char c = raw[i];
        if (c == '"') out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') continue;
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

bool IskakINO_Telegram::sendPostRequest(const char* endpoint, const String& jsonPayload) {
    if (WiFi.status() != WL_CONNECTED) {
        _lastError = IskakINO_Result::NOT_CONNECTED;
        return false;
    }

    if (!_token || strlen(_token) == 0) {
        _lastError = IskakINO_Result::INVALID_ARG;
        return false;
    }

    if (!_client.connect(TELEGRAM_HOST, TELEGRAM_PORT)) {
        _logger.log(F("[IskakINO Telegram] Gagal membuka koneksi HTTPS ke Telegram API."));
        _lastError = IskakINO_Result::NOT_CONNECTED;
        return false;
    }

    String path = "/bot" + String(_token) + "/" + String(endpoint);

    _client.print(String("POST ") + path + " HTTP/1.1\r\n" +
                  "Host: " + TELEGRAM_HOST + "\r\n" +
                  "Content-Type: application/json\r\n" +
                  "Connection: close\r\n" +
                  "Content-Length: " + String(jsonPayload.length()) + "\r\n\r\n" +
                  jsonPayload);

    // Tunggu respon singkat
    unsigned long startMs = millis();
    while (!_client.available()) {
        if (millis() - startMs > 5000) {
            _client.stop();
            _lastError = IskakINO_Result::TIMEOUT;
            return false;
        }
        delay(10);
    }

    // Baca status line
    String statusLine = _client.readStringUntil('\n');
    bool ok = (statusLine.indexOf("200 OK") >= 0);
    if (!ok) {
        _logger.logf(F("[IskakINO Telegram] HTTP status: %s"), statusLine.c_str());
        _lastError = IskakINO_Result::WRITE_FAILED;
    } else {
        _lastError = IskakINO_Result::OK;
    }

    return ok;
}

bool IskakINO_Telegram::sendMessage(const char* chatId, const char* text, const char* parseMode) {
    if (!chatId || !text) {
        _lastError = IskakINO_Result::INVALID_ARG;
        return false;
    }

    String payload = "{\"chat_id\":\"" + escapeJson(String(chatId)) + "\",";
    payload += "\"text\":\"" + escapeJson(String(text)) + "\"";
    if (parseMode && strlen(parseMode) > 0) {
        payload += ",\"parse_mode\":\"" + escapeJson(String(parseMode)) + "\"";
    }
    payload += "}";

    bool success = sendPostRequest("sendMessage", payload);
    _client.stop();
    return success;
}

bool IskakINO_Telegram::sendMessage(const char* text) {
    if (!_defaultChatId || strlen(_defaultChatId) == 0) {
        _lastError = IskakINO_Result::INVALID_ARG;
        _logger.log(F("[IskakINO Telegram] defaultChatId belum diatur pada begin()."));
        return false;
    }
    return sendMessage(_defaultChatId, text);
}

bool IskakINO_Telegram::sendAlert(const char* title, const char* message, const char* emoji) {
    String msg = String(emoji ? emoji : "⚠️") + " *" + String(title ? title : "Alert") + "*\n\n";
    if (message) msg += String(message);
    return sendMessage(msg.c_str());
}

void IskakINO_Telegram::fetchUpdates() {
    if (WiFi.status() != WL_CONNECTED || !_token) return;

    String payload = "{\"offset\":" + String(_lastUpdateId + 1) + ",\"limit\":5,\"timeout\":0}";
    if (!sendPostRequest("getUpdates", payload)) {
        _client.stop();
        return;
    }

    // Lewati HTTP Header sampai "\r\n\r\n"
    while (_client.available()) {
        String line = _client.readStringUntil('\n');
        if (line == "\r" || line.length() == 0) break;
    }

    String body = _client.readString();
    _client.stop();

    if (body.indexOf("\"ok\":true") == -1) return;

    // Parsing ringan pesan JSON Telegram getUpdates
    int idx = 0;
    while ((idx = body.indexOf("\"update_id\":", idx)) != -1) {
        idx += 12;
        int endId = body.indexOf(',', idx);
        if (endId == -1) break;
        long uId = body.substring(idx, endId).toInt();
        if (uId > _lastUpdateId) _lastUpdateId = uId;

        // Cari Chat ID
        int chatIdx = body.indexOf("\"chat\":{\"id\":", idx);
        String chatId = "";
        if (chatIdx != -1) {
            chatIdx += 13;
            int endChat = body.indexOf(',', chatIdx);
            if (endChat != -1) chatId = body.substring(chatIdx, endChat);
        }

        // Cari First Name
        String fromUser = "User";
        int nameIdx = body.indexOf("\"first_name\":\"", idx);
        if (nameIdx != -1) {
            nameIdx += 14;
            int endName = body.indexOf('"', nameIdx);
            if (endName != -1) fromUser = body.substring(nameIdx, endName);
        }

        // Cari Text
        int textIdx = body.indexOf("\"text\":\"", idx);
        if (textIdx != -1) {
            textIdx += 8;
            int endText = body.indexOf('"', textIdx);
            if (endText != -1) {
                String text = body.substring(textIdx, endText);

                // Eksekusi Command Callback
                bool handled = false;
                for (int c = 0; c < _cmdCount; c++) {
                    if (text.startsWith(_commands[c].command) && _commands[c].callback) {
                        _commands[c].callback(chatId.c_str(), fromUser.c_str(), text.c_str());
                        handled = true;
                        break;
                    }
                }
                if (!handled && _globalCallback) {
                    _globalCallback(chatId.c_str(), fromUser.c_str(), text.c_str());
                }
            }
        }
    }
}

void IskakINO_Telegram::tick() {
    if (_pollingEnabled && _scheduler.every(_pollIntervalMs, ISKAKINO_SCHED_TELEGRAM_POLL)) {
        fetchUpdates();
    }
}

#endif // defined(ISKAKINO_HAS_WIFI)
