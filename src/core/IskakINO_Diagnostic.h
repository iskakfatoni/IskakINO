/*
 * src/core/IskakINO_Diagnostic.h
 * Utilitas diagnostik hardware, pemantau RAM (Free Heap / Stack),
 * deteksi fragmentasi memori, dan alasan reset (Reset Reason) lintas platform.
 *
 * Mendukung: AVR (Uno/Nano/Mega), ESP8266 (NodeMCU), dan ESP32.
 */

#ifndef ISKAKINO_DIAGNOSTIC_H
#define ISKAKINO_DIAGNOSTIC_H

#include <Arduino.h>
#include "IskakINO_Platform.h"

#if defined(ISKAKINO_PLATFORM_ESP32)
  #include <esp_system.h>
#endif

#if defined(ISKAKINO_PLATFORM_AVR)
  #include <avr/wdt.h>
#endif

class IskakINO_Diagnostic {
  public:
    // Mengembalikan estimasi sisa memori RAM (byte) yang masih dapat dialokasikan.
    static uint32_t getFreeRAM() {
#if defined(ISKAKINO_PLATFORM_AVR)
        extern int __heap_start, *__brkval;
        int v;
        return (uint32_t)((int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval));
#elif defined(ISKAKINO_PLATFORM_ESP8266)
        return (uint32_t)ESP.getFreeHeap();
#elif defined(ISKAKINO_PLATFORM_ESP32)
        return (uint32_t)ESP.getFreeHeap();
#else
        return 0;
#endif
    }

    // Alias untuk getFreeRAM()
    static inline uint32_t getFreeHeap() {
        return getFreeRAM();
    }

    // Mengembalikan sisa memori terendah yang pernah dicapai (High Water Mark).
    static uint32_t getMinFreeHeap() {
#if defined(ISKAKINO_PLATFORM_ESP32)
        return (uint32_t)ESP.getMinFreeHeap();
#else
        return getFreeRAM();
#endif
    }

    // Mengembalikan blok alokasi RAM terbesar yang dapat dialokasikan sekaligus tanpa OOM.
    static uint32_t getMaxAllocHeap() {
#if defined(ISKAKINO_PLATFORM_ESP32)
        return (uint32_t)ESP.getMaxAllocHeap();
#elif defined(ISKAKINO_PLATFORM_ESP8266)
        return (uint32_t)ESP.getMaxFreeBlockSize();
#else
        return getFreeRAM();
#endif
    }

    // Mengembalikan persentase fragmentasi heap (0% = tidak terfragmentasi, 100% = parah).
    static uint8_t getHeapFragmentation() {
#if defined(ISKAKINO_PLATFORM_ESP8266)
        return (uint8_t)ESP.getHeapFragmentation();
#elif defined(ISKAKINO_PLATFORM_ESP32)
        uint32_t freeH = ESP.getFreeHeap();
        if (freeH == 0) return 0;
        uint32_t maxB = ESP.getMaxAllocHeap();
        if (maxB >= freeH) return 0;
        return (uint8_t)(100 - ((uint64_t)maxB * 100 / freeH));
#else
        return 0; // AVR tidak memiliki kalkulator fragmentasi bawaan
#endif
    }

    // Mengembalikan sisa memori PSRAM eksternal (byte) jika tersedia pada ESP32.
    static uint32_t getFreePSRAM() {
#if defined(ISKAKINO_PLATFORM_ESP32)
        return (uint32_t)ESP.getFreePsram();
#else
        return 0;
#endif
    }

    // Mengembalikan alasan mengapa mikrokontroler terakhir kali melakukan reset/reboot.
    static const char* getResetReason() {
#if defined(ISKAKINO_PLATFORM_ESP32)
        esp_reset_reason_t reason = esp_reset_reason();
        switch (reason) {
            case ESP_RST_POWERON:   return "Power-On Reset";
            case ESP_RST_EXT:       return "External Pin Reset";
            case ESP_RST_SW:        return "Software Reset";
            case ESP_RST_PANIC:     return "Exception / Panic";
            case ESP_RST_INT_WDT:   return "Interrupt Watchdog";
            case ESP_RST_TASK_WDT:  return "Task Watchdog";
            case ESP_RST_WDT:       return "Other Watchdog";
            case ESP_RST_DEEPSLEEP: return "Deep Sleep Wakeup";
            case ESP_RST_BROWNOUT:  return "Brownout Reset";
            case ESP_RST_SDIO:      return "SDIO Reset";
            default:                return "Unknown Reset";
        }
#elif defined(ISKAKINO_PLATFORM_ESP8266)
        static char esp8266Reason[32];
        strncpy(esp8266Reason, ESP.getResetReason().c_str(), sizeof(esp8266Reason) - 1);
        esp8266Reason[sizeof(esp8266Reason) - 1] = '\0';
        return esp8266Reason;
#elif defined(ISKAKINO_PLATFORM_AVR)
        return "Normal / Power-On";
#else
        return "Generic Reset";
#endif
    }

    // Durasi mikroprosesor berjalan sejak boot (detik).
    static inline uint32_t getUptimeSeconds() {
        return millis() / 1000UL;
    }

    // Mencetak ringkasan status diagnostik hardware ke Stream (Serial).
    static void printSummary(Stream& out = Serial) {
        out.println(F("=== IskakINO Diagnostic Summary ==="));
        out.print(F("Platform      : "));
#if defined(ISKAKINO_PLATFORM_ESP32)
        out.println(F("ESP32"));
#elif defined(ISKAKINO_PLATFORM_ESP8266)
        out.println(F("ESP8266"));
#elif defined(ISKAKINO_PLATFORM_AVR)
        out.println(F("Arduino AVR"));
#else
        out.println(F("Unknown / Other"));
#endif
        out.print(F("Uptime        : "));
        out.print(getUptimeSeconds());
        out.println(F(" detik"));

        out.print(F("Reset Reason  : "));
        out.println(getResetReason());

        out.print(F("Free RAM/Heap : "));
        out.print(getFreeRAM());
        out.println(F(" bytes"));

#if defined(ISKAKINO_PLATFORM_ESP32) || defined(ISKAKINO_PLATFORM_ESP8266)
        out.print(F("Max Alloc Block: "));
        out.print(getMaxAllocHeap());
        out.println(F(" bytes"));

        out.print(F("Fragmentation : "));
        out.print(getHeapFragmentation());
        out.println(F("%"));
#endif

#if defined(ISKAKINO_PLATFORM_ESP32)
        uint32_t psram = getFreePSRAM();
        if (psram > 0) {
            out.print(F("Free PSRAM    : "));
            out.print(psram);
            out.println(F(" bytes"));
        }
#endif
        out.println(F("==================================="));
    }
};

// Alias untuk kenyamanan penamaan
typedef IskakINO_Diagnostic IskakINO_Memory;

#endif // ISKAKINO_DIAGNOSTIC_H
