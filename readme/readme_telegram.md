# 🤖 Modul: IskakINO_Telegram

Bot client dan notifier **Telegram** mandiri (*zero-dependency*) via HTTPS REST API khusus untuk platform mikrokontroler WiFi (**ESP32** & **ESP8266**).

Menggunakan `WiFiClientSecure` dengan mode *insecure TLS* otomatis untuk efisiensi RAM tanpa perlu instalasi sertifikat CA yang rumit. Mendukung pengiriman pesan notifikasi cepat, alert sistem terformat, serta *command dispatcher* interaktif *non-blocking*.

---

## 🛠️ Fitur Utama

1. **Pengiriman Notifikasi Instan:**
   * Kirim pesan Markdown / HTML langsung ke chat personal maupun grup Telegram.
   * Helper `sendAlert(title, message, emoji)` untuk mengirim format kartu peringatan sistem.
2. **Command Dispatcher Interaktif Non-Blocking:**
   * Daftarkan perintah bot (misal: `/status`, `/relay_on`, `/help`) via `onCommand("/cmd", callback)`.
   * Polling berkala *non-blocking* (`tick()`) tanpa mengganggu proses loop utama.
3. **Zero External Dependency:**
   * Beroperasi langsung di atas `WiFiClientSecure` bawaan core ESP32/ESP8266 tanpa library Telegram pihak ketiga.

---

## 💻 Contoh Penggunaan Singkat

### 1. Bot Notifier & Command Handler
```cpp
#include <IskakINO.h>

IskakINO_Telegram bot;

const char* BOT_TOKEN = "1234567890:ABCdefGHIjklMNOpqrSTUvwxYZ";
const char* CHAT_ID   = "987654321";

void handleStatus(const char* chatId, const char* fromUser, const char* text) {
    String reply = "Halo " + String(fromUser) + "!\nSistem berjalan normal. Uptime: " + String(millis() / 1000) + "s";
    bot.sendMessage(chatId, reply.c_str());
}

void setup() {
    Serial.begin(115200);
    WiFi.begin("SSID_WIFI", "PASSWORD_WIFI");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    // Inisialisasi token & default chat ID
    bot.begin(BOT_TOKEN, CHAT_ID);

    // Daftarkan perintah interaktif
    bot.onCommand("/status", handleStatus);

    // Aktifkan polling pesan masuk setiap 3 detik
    bot.enablePolling(true, 3000);

    // Kirim notifikasi awal
    bot.sendAlert("Sistem Dimulai", "Node ESP32 berhasil terhubung ke jaringan.", "🚀");
}

void loop() {
    // WAJIB: Panggil tick() di loop untuk memproses polling pesan masuk
    bot.tick();
}
```

### 2. Pola Framework Kernel
```cpp
#include <IskakINO.h>

IskakINO_Telegram bot;
IskakINO_TelegramModule botMod(bot, "BOT_TOKEN", "CHAT_ID", true, 3000);

void setup() {
    IskakINO.registerModule(&botMod);
    IskakINO.begin();
}

void loop() {
    IskakINO.update();
}
```

---

## 📖 Referensi API Publik

### Inisialisasi
* `void begin(const char* botToken, const char* defaultChatId = nullptr)`: Konfigurasi token API Bot Telegram dan default Chat ID.

### Pengiriman Pesan
* `bool sendMessage(const char* chatId, const char* text, const char* parseMode = "Markdown")`: Mengirim pesan ke Chat ID tertentu.
* `bool sendMessage(const char* text)`: Mengirim pesan ke default Chat ID yang telah dikonfigurasi saat `begin()`.
* `bool sendAlert(const char* title, const char* message, const char* emoji = "⚠️")`: Mengirim pesan terformat dengan emoji dan judul tebal.

### Interaksi & Polling
* `void onCommand(const char* cmd, TelegramMsgCallback callback)`: Mendaftarkan fungsi penanganan perintah bot (contoh: `"/relay_on"`).
* `void onMessage(TelegramMsgCallback callback)`: Callback untuk menangani semua pesan teks yang bukan merupakan perintah terdaftar.
* `void enablePolling(bool enable, uint32_t intervalMs = 3000)`: Mengaktifkan atau menonaktifkan polling pesan baru.
* `void tick()`: Memproses scheduler polling pesan masuk.
