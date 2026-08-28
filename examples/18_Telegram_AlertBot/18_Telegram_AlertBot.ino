/*
 * 18_Telegram_AlertBot.ino
 * Modul: IskakINO_Telegram & IskakINO_ArduFast (HANYA ESP32 & ESP8266)
 *
 * Menunjukkan:
 *   1. Zero-dependency Telegram Bot Client via HTTPS
 *   2. Pengiriman notifikasi alert terformat (sendAlert & sendMessage)
 *   3. Penanganan perintah interaktif dari chat Telegram (/status, /ping, /help)
 *   4. Polling pesan masuk non-blocking
 */

#include <IskakINO.h>

#if !defined(ISKAKINO_HAS_WIFI)
  #error "Sketsa ini hanya mendukung board ESP32 atau ESP8266."
#endif

// Konfigurasi WiFi & Telegram Bot
const char* WIFI_SSID   = "YOUR_WIFI_SSID";
const char* WIFI_PASS   = "YOUR_WIFI_PASS";
const char* BOT_TOKEN   = "123456789:ABCdefGHIjklMNOpqrSTUvwxYZ";
const char* CHAT_ID     = "123456789"; // Chat ID tujuan alert

IskakINO_ArduFast fast;
IskakINO_Telegram bot;

// Handler perintah /status
void onStatusCommand(const char* chatId, const char* fromUser, const char* text) {
    fast.logf(F("[Telegram] Perintah /status dari %s"), fromUser);

    String reply = "🤖 *Status Perangkat IskakINO*\n\n";
    reply += "• *Uptime:* " + String(millis() / 1000) + " detik\n";
    reply += "• *IP:* `" + WiFi.localIP().toString() + "`\n";
    reply += "• *Free RAM:* " + String(ESP.getFreeHeap() / 1024) + " KB";

    bot.sendMessage(chatId, reply.c_str(), "Markdown");
}

// Handler perintah /ping
void onPingCommand(const char* chatId, const char* fromUser, const char* text) {
    fast.logf(F("[Telegram] Perintah /ping dari %s"), fromUser);
    bot.sendMessage(chatId, "🏓 Pong! Sistem aktif dan responsif.");
}

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - Telegram Alert Bot Demo    "));
    fast.log(F("========================================"));

    // Hubungkan ke WiFi
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    fast.log(F("[WiFi] Menghubungkan ke jaringan..."));

    // Inisialisasi Telegram Bot
    bot.begin(BOT_TOKEN, CHAT_ID);
    bot.onCommand("/status", onStatusCommand);
    bot.onCommand("/ping", onPingCommand);
    bot.enablePolling(true, 3000); // Poll setiap 3 detik secara non-blocking

    // Kirim notifikasi boot
    bot.sendAlert("Sistem Dimulai", "Perangkat IskakINO berhasil booting dan terhubung.", "🚀");
}

void loop() {
    // Jalankan scheduler Telegram untuk polling pesan
    bot.tick();

    // Contoh: Kirim laporan heartbeat setiap 60 detik
    if (WiFi.status() == WL_CONNECTED && fast.every(60000, 0)) {
        bot.sendAlert("Heartbeat", "Sistem IoT beroperasi normal.", "💚");
    }
}
