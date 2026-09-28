/*
 * src/websockets/IskakINO_WebSockets.h
 *
 * Header gabungan untuk seluruh modul IskakINO_WebSockets.
 * Menyediakan IskakINO_WebSocketsServer dan IskakINO_WebSocketsClient.
 *
 * Hanya aktif pada platform yang memiliki konektivitas WiFi (ESP32 / ESP8266).
 */

#ifndef ISKAKINO_WEBSOCKETS_H
#define ISKAKINO_WEBSOCKETS_H

#include "../core/IskakINO_Platform.h"

#if defined(ISKAKINO_HAS_WIFI)

#include "IskakINO_WS_SHA1.h"
#include "IskakINO_WS_Frame.h"
#include "IskakINO_WebSocketsServer.h"
#include "IskakINO_WebSocketsClient.h"

// Alias kelas utama default
typedef IskakINO_WebSocketsServer IskakINO_WebSockets;

#endif // defined(ISKAKINO_HAS_WIFI)
#endif // ISKAKINO_WEBSOCKETS_H
