#ifndef MOCK_WEBSERVER_H
#define MOCK_WEBSERVER_H

#include <cstdint>
#include <cstddef>
#include <functional>
#include <string>

#if __has_include(<Arduino.h>)
#include <Arduino.h>
#else
#include "../native_check/Arduino.h"
#endif

#include "ArduinoExtra.h"

#ifndef HTTP_GET
#define HTTP_GET 0
#endif

#ifndef HTTP_POST
#define HTTP_POST 1
#endif

#ifndef HTTP_ANY
#define HTTP_ANY 2
#endif

enum HTTPUploadStatus {
    UPLOAD_FILE_START,
    UPLOAD_FILE_WRITE,
    UPLOAD_FILE_END,
    UPLOAD_FILE_ABORTED
};

struct HTTPUpload {
    HTTPUploadStatus status = UPLOAD_FILE_START;
    uint8_t* buf = nullptr;
    size_t currentSize = 0;
    size_t totalSize = 0;
};

class WebServer {
public:
    explicit WebServer(int port = 80) { (void)port; }

    void on(const String& uri, std::function<void()> handler) {
        (void)uri; (void)handler;
    }
    void on(const char* uri, std::function<void()> handler) {
        (void)uri; (void)handler;
    }

    void on(const String& uri, int method, std::function<void()> handler) {
        (void)uri; (void)method; (void)handler;
    }
    void on(const char* uri, int method, std::function<void()> handler) {
        (void)uri; (void)method; (void)handler;
    }

    void on(const String& uri, int method, std::function<void()> handler, std::function<void()> uploadHandler) {
        (void)uri; (void)method; (void)handler; (void)uploadHandler;
    }
    void on(const char* uri, int method, std::function<void()> handler, std::function<void()> uploadHandler) {
        (void)uri; (void)method; (void)handler; (void)uploadHandler;
    }

    void onNotFound(std::function<void()> handler) {
        (void)handler;
    }

    void begin() {}
    void begin(uint16_t port) { (void)port; }
    void handleClient() {}
    void close() {}
    void stop() {}

    String arg(const String& name) { (void)name; return String(""); }
    String arg(const char* name) { (void)name; return String(""); }
    String arg(int i) { (void)i; return String(""); }
    String argName(int i) { (void)i; return String(""); }
    int args() { return 0; }

    bool hasArg(const String& name) { (void)name; return false; }
    bool hasArg(const char* name) { (void)name; return false; }

    void send(int code) {
        (void)code;
    }
    void send(int code, const char* content_type) {
        (void)code; (void)content_type;
    }
    void send(int code, const char* content_type, const char* content) {
        (void)code; (void)content_type; (void)content;
    }
    void send(int code, const char* content_type, const String& content) {
        (void)code; (void)content_type; (void)content;
    }
    void send(int code, const String& content_type, const String& content) {
        (void)code; (void)content_type; (void)content;
    }

    void send_P(int code, const char* content_type, const char* content) {
        (void)code; (void)content_type; (void)content;
    }
    void send_P(int code, const char* content_type, const char* content, size_t contentLength) {
        (void)code; (void)content_type; (void)content; (void)contentLength;
    }

    void sendHeader(const String& name, const String& value, bool first = false) {
        (void)name; (void)value; (void)first;
    }
    void sendHeader(const char* name, const char* value, bool first = false) {
        (void)name; (void)value; (void)first;
    }

    HTTPUpload& upload() {
        static HTTPUpload u;
        return u;
    }
};

#endif // MOCK_WEBSERVER_H
