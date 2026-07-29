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
 *                                 (semua platform: AVR/ESP32/ESP8266/RP2040)
 *   - IskakINO_Storage         : storage hybrid EEPROM/Preferences/LittleFS
 *                                 (semua platform)
 *   - LiquidCrystal_I2C        : LCD karakter I2C, non-blocking typewriter/
 *                                 scroll/progress bar (semua platform, perlu Wire)
 *   - IskakINO_SmartVoice      : kontrol DFPlayer Mini MP3 (semua platform,
 *                                 perlu Stream/Serial)
 *   - IskakINO_WifiPortal      : captive portal WiFi + custom parameter
 *                                 (HANYA ESP32/ESP8266 — otomatis kosong di
 *                                 board lain, lihat catatan di bawah)
 *   - IskakINO_FastNTP         : sinkronisasi waktu NTP non-blocking
 *                                 (HANYA ESP32/ESP8266 — sama seperti di atas)
 *
 * CATATAN PENTING soal WifiPortal & FastNTP di board non-WiFi (mis. AVR
 * Uno/Nano): kedua header modul itu SUDAH otomatis membungkus seluruh
 * isinya dengan guard platform (lihat komentar di masing-masing header),
 * jadi #include <IskakINO.h> di board AVR TETAP AMAN — modul WiFi-only
 * itu cuma "menghilang" (tidak ter-generate kode apa pun, bukan error).
 * Anda hanya perlu berhati-hati: JANGAN menulis kode yang menggunakan
 * class IskakINO_WifiPortal / IskakINO_FastNTP di board non-WiFi, karena
 * class-nya sendiri tidak akan ada (undefined) di platform itu.
 *
 * --- Framework (opsional) ---
 * Selain dipakai satu-satu secara manual (tiap modul begin()/tick()/
 * update() sendiri-sendiri seperti contoh 07_Unified_SmartClock), modul
 * juga bisa didaftarkan ke kernel global `IskakINO` lewat kelas adapter
 * (IskakINO_WifiPortalModule, IskakINO_FastNTPModule, IskakINO_LCDModule,
 * IskakINO_StorageModule, IskakINO_SmartVoiceModule, IskakINO_ArduFastModule)
 * supaya begin()/update() semuanya terpanggil otomatis lewat satu
 * IskakINO.begin() dan satu IskakINO.update(). Lihat
 * examples/08_Framework_Kernel dan src/core/IskakINO_Kernel.h. Pola
 * manual TETAP didukung penuh -- kernel ini murni kenyamanan opsional,
 * bukan keharusan. Trigger
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

// --- Modul universal (semua platform: AVR/ESP32/ESP8266/RP2040) ---
#include "ardufast/IskakINO_ArduFast.h"
#include "ardufast/IskakINO_ArduFastModule.h"
#include "storage/IskakINO_Storage.h"
#include "storage/IskakINO_StorageModule.h"
#include "lcd/IskakINO_LiquidCrystal_I2C.h"
#include "lcd/IskakINO_LCDModule.h"
#include "voice/IskakINO_SmartVoice.h"
#include "voice/IskakINO_SmartVoiceModule.h"

// --- Modul khusus WiFi (ESP32/ESP8266 saja) ---
// Header-header ini AMAN di-include tanpa syarat di sini — masing-masing
// sudah membungkus SELURUH isinya dengan `#if defined(ISKAKINO_HAS_WIFI)`
// secara internal, jadi otomatis jadi no-op di board non-WiFi.
#include "wifi/IskakINO_WifiPortal.h"
#include "wifi/IskakINO_WifiPortalModule.h"
#include "ntp/IskakINO_FastNTP.h"
#include "ntp/IskakINO_FastNTPModule.h"

#endif
