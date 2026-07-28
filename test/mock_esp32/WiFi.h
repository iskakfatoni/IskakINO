#ifndef MOCK_WIFI_H
#define MOCK_WIFI_H
#include <Arduino.h>
#include "ArduinoExtra.h"
#define WIFI_STA 1
#define WIFI_AP 2
#define WL_CONNECTED 3
#define UPDATE_SIZE_UNKNOWN 0xFFFFFFFF

struct IPAddress {
    String toString() const { return String("0.0.0.0"); }
};

class WiFiClass {
public:
    void mode(int) {}
    int scanNetworks(bool async = false) { (void)async; return 0; }
    int scanComplete() { return 0; }
    void scanDelete() {}
    String SSID(int) { return String(""); }
    int RSSI(int) { return -50; }
    bool begin(const char*, const char* = nullptr) { return true; }
    int status() { return WL_CONNECTED; }
    bool reconnect() { return true; }
    IPAddress localIP() { return IPAddress(); }
    bool softAP(const char*, const char* = nullptr) { return true; }
    IPAddress softAPIP() { return IPAddress(); }
};
extern WiFiClass WiFi;
#endif
