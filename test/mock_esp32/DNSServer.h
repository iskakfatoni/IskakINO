#ifndef MOCK_DNSSERVER_H
#define MOCK_DNSSERVER_H
#include <Arduino.h>
#include "ArduinoExtra.h"
#include "WiFi.h"
class DNSServer {
public:
    void start(int, const String&, const IPAddress&) {}
    void processNextRequest() {}
};
#endif
