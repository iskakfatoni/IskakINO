/*
 * src/IskakINO.h
 * Entry point tunggal library IskakINO gabungan.
 *
 * Cukup satu baris ini di sketch untuk memakai semua modul:
 *   #include <IskakINO.h>
 *
 * Modul yang tersedia (namespace kelas tidak berubah dari versi standalone
 * masing-masing, supaya kode lama tetap kompatibel):
 *   - IskakINO_ArduFast        : FastPin<P> (register I/O langsung), task
 *                                 manager, logging, EMA filter, dll.
 *                                 (semua platform: AVR/ESP32/ESP8266)
 *   - IskakINO_Storage         : storage hybrid EEPROM/Preferences/LittleFS
 *                                 (semua platform)
 *   - LiquidCrystal_I2C        : LCD karakter I2C, non-blocking typewriter/
 *                                 scroll/progress bar (semua platform, perlu Wire)
 *   - IskakINO_SmartVoice      : kontrol DFPlayer Mini MP3 (semua platform,
 *                                 perlu Stream/Serial)
 *   - IskakINO_Buzzer          : driver buzzer non-blocking, nada status,
 *                                 dan RTTTL melody player (semua platform)
 *   - IskakINO_OLED            : driver layar OLED I2C (SSD1306 & SH1106)
 *                                 ultra-hemat RAM dengan animasi teks & ikon (semua platform)
 *   - IskakINO_Button          : driver tombol pintar gesture (single, double, hold) non-blocking (semua platform)
 *   - IskakINO_Relay           : driver relay pintar dengan auto-off pulse & blink cadence (semua platform)
 *   - IskakINO_Filter          : filter sinyal (Kalman 1D, Median, EMA, Linear Calibrator) (semua platform)
 *   - IskakINO_RTC             : driver hardware RTC (DS3231/DS1307/PCF8563) & Hybrid NTP Sync (semua platform, perlu Wire)
 *   - IskakINO_Sensors         : pembacaan terpadu sensor DHT11/22, DS18B20, Ultrasonik HC-SR04, LDR (semua platform)
 *   - IskakINO_JSON            : pembuat & pembaca JSON zero-dependency, ultra-hemat RAM & Flash (semua platform)
 *   - IskakINO_Cam             : driver modul kamera ESP32 (ESP32-CAM, OV2640, PSRAM, Flash LED) (HANYA ESP32)
 *   - IskakINO_MQTT            : client MQTT v3.1.1 zero-dependency (pub/sub, keepalive, auto-reconnect) (HANYA ESP32/ESP8266)
 *   - IskakINO_Telegram        : bot Telegram notifier & remote control via HTTPS REST (HANYA ESP32/ESP8266)
 *   - IskakINO_WifiPortal      : captive portal WiFi + custom parameter
 *                                 (HANYA ESP32/ESP8266 — otomatis kosong di
 *                                 board lain, lihat catatan di bawah)
 *   - IskakINO_FastNTP         : sinkronisasi waktu NTP non-blocking
 *                                 (HANYA ESP32/ESP8266 — sama seperti di atas)
 *   - IskakINO_BasicIOShield   : driver modul EMS Basic I/O Shield (LED,
 *                                 button, potensiometer, 7-segment non-blocking,
 *                                 DAC AD5612 I2C) (HANYA platform AVR)
 *
 * CATATAN PENTING soal WifiPortal, FastNTP, MQTT, Telegram, & Cam di board non-WiFi (mis. AVR
 * Uno/Nano) serta BasicIOShield di board non-AVR: header-header modul
 * khusus itu SUDAH otomatis membungkus seluruh isinya dengan guard platform,
 * jadi #include <IskakINO.h> di platform mana pun TETAP AMAN (otomatis jadi
 * no-op di board yang tidak didukung).
 *
 * --- Framework (opsional) ---
 * Selain dipakai satu-satu secara manual (tiap modul begin()/tick()/
 * update() sendiri-sendiri seperti contoh 07_Unified_SmartClock), modul
 * juga bisa didaftarkan ke kernel global `IskakINO` lewat kelas adapter
 * (IskakINO_WifiPortalModule, IskakINO_FastNTPModule, IskakINO_MQTTModule,
 * IskakINO_TelegramModule, IskakINO_RTCModule, IskakINO_LCDModule, IskakINO_OLEDModule,
 * IskakINO_ButtonModule, IskakINO_RelayModule, IskakINO_CamModule,
 * IskakINO_StorageModule, IskakINO_SmartVoiceModule, IskakINO_BuzzerModule,
 * IskakINO_ArduFastModule, IskakINO_BasicIOShieldModule)
 * supaya begin()/update() semuanya terpanggil otomatis lewat satu
 * IskakINO.begin() dan satu IskakINO.update(). Lihat
 * examples/08_Framework_Kernel dan src/core/IskakINO_Kernel.h. Pola
 * manual TETAP didukung penuh -- kernel ini murni kenyamanan opsional,
 * bukan keharusan.
 */

#ifndef ISKAKINO_H
#define ISKAKINO_H

// --- Core (selalu ada, semua platform) ---
#include "core/IskakINO_Version.h"
#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Result.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Scheduler.h"
#include "core/IskakINO_Module.h"
#include "core/IskakINO_Kernel.h"
#include "core/IskakINO_TaskCore.h"

// --- Modul universal (semua platform: AVR/ESP32/ESP8266) ---
#include "ardufast/IskakINO_ArduFast.h"
#include "ardufast/IskakINO_ArduFastModule.h"
#include "storage/IskakINO_Storage.h"
#include "storage/IskakINO_StorageModule.h"
#include "lcd/IskakINO_LiquidCrystal_I2C.h"
#include "lcd/IskakINO_LCDModule.h"
#include "voice/IskakINO_SmartVoice.h"
#include "voice/IskakINO_SmartVoiceModule.h"
#include "buzzer/IskakINO_Pitches.h"
#include "buzzer/IskakINO_Buzzer.h"
#include "buzzer/IskakINO_BuzzerModule.h"
#include "oled/IskakINO_OLEDFonts.h"
#include "oled/IskakINO_OLED.h"
#include "oled/IskakINO_OLEDModule.h"
#include "button/IskakINO_Button.h"
#include "button/IskakINO_ButtonModule.h"
#include "relay/IskakINO_Relay.h"
#include "relay/IskakINO_RelayModule.h"
#include "filter/IskakINO_Filter.h"
#include "rtc/IskakINO_RTC.h"
#include "rtc/IskakINO_RTCModule.h"
#include "sensors/IskakINO_Sensors.h"
#include "json/IskakINO_JSON.h"
#include "prayertimes/IskakINO_PrayerTimes.h"
#include "prayertimes/IskakINO_PrayerTimesModule.h"

// --- Modul khusus AVR (Arduino Uno/Nano/Mega saja) ---
#include "shield/IskakINO_BasicIOShield.h"
#include "shield/IskakINO_BasicIOShieldModule.h"

// --- Modul khusus WiFi & ESP32 (ESP32/ESP8266 saja) ---
// Header-header ini AMAN di-include tanpa syarat di sini — masing-masing
// sudah membungkus SELURUH isinya dengan guard platform internal,
// jadi otomatis jadi no-op di board yang tidak didukung.
#include "wifi/IskakINO_WifiPortal.h"
#include "wifi/IskakINO_WifiPortalModule.h"
#include "ntp/IskakINO_FastNTP.h"
#include "ntp/IskakINO_FastNTPModule.h"
#include "mqtt/IskakINO_MQTT.h"
#include "mqtt/IskakINO_MQTTModule.h"
#include "telegram/IskakINO_Telegram.h"
#include "telegram/IskakINO_TelegramModule.h"
#include "websockets/IskakINO_WebSockets.h"
#include "websockets/IskakINO_WebSocketsModule.h"
#include "cam/IskakINO_CamPins.h"
#include "cam/IskakINO_Cam.h"
#include "cam/IskakINO_CamModule.h"
#include "ble/IskakINO_BLE.h"
#include "ble/IskakINO_BLEModule.h"

#endif
