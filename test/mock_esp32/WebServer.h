#ifndef MOCK_WEBSERVER_H
#define MOCK_WEBSERVER_H
#include <Arduino.h>
#include "ArduinoExtra.h"
#include <functional>

#define HTTP_GET 0
#define HTTP_POST 1

enum HTTPUploadStatus { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END, UPLOAD_FILE_ABORTED };

struct HTTPUpload {
    HTTPUploadStatus status = UPLOAD_FILE_START;
    uint8_t* buf = nullptr;
    size_t currentSize = 0;
    size_t totalSize = 0;
};

class WebServer {
public:
    explicit WebServer(int port) { (void)port; }
    void on(const String&, std::function<void()> handler) { (void)handler; }
    void on(const String&, int method, std::function<void()> handler) { (void)method; (void)handler; }
    void on(const String&, int method, std::function<void()> handler, std::function<void()> uploadHandler) {
        (void)method; (void)handler; (void)uploadHandler;
    }
    void onNotFound(std::function<void()> handler) { (void)handler; }
    void begin() {}
    void handleClient() {}
    String arg(const String&) { return String(""); }
    void send(int, const String&, const String&) {}
    void sendHeader(const String&, const String&) {}
    HTTPUpload& upload() { static HTTPUpload u; return u; }
};
#endif
