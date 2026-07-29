# Changelog — IskakINO

Semua modul IskakINO kini berada dalam satu library dengan satu versi
global (`ISKAKINO_VERSION`, lihat `src/core/IskakINO_Version.h`). Library
lama (`IskakINO_ArduFast`, `IskakINO_Storage`, `IskakINO_LiquidCrystal_I2C`,
`IskakINO_WifiPortal`, `IskakINO_FastNTP`, `IskakINO_SmartVoice`) resmi
**deprecated** per rilis ini — histori versi masing-masing sebelum
penggabungan tetap didokumentasikan di bawah untuk keperluan migrasi.

## [1.0.0] — Rilis pertama IskakINO gabungan

Penggabungan 6 library standalone menjadi satu library dengan shared
`src/core/` (Platform, Result, Logger, Scheduler, Version). Signature
seluruh fungsi publik tiap modul **tidak berubah** dari versi standalone
terakhirnya — lihat catatan per-modul di bawah untuk detail perubahan
internal dan penambahan opsional.

### core/ (baru)
- `IskakINO_Platform.h` — deteksi platform & `FastPin<P>` terpusat (diambil dari ArduFast).
- `IskakINO_Result.h` — enum status operasi terpusat; `IskakStorageResult` kini alias darinya.
- `IskakINO_Logger.h` — logging printf-style dengan flag debug per-instance (diekstrak dari ArduFast).
  - **Bug fix (ditemukan lewat CI sungguhan `arduino:avr:uno`):** avr-libc `<math.h>` (ke-include transitif lewat `Arduino.h`) mendefinisikan `log`/`logf` sebagai macro saling terhubung (arahnya `#define logf log`, dikonfirmasi lewat 2 iterasi CI karena dugaan awal arahnya terbalik). Ini bikin method `logf()` (variadic) di `IskakINO_Logger` (dan `IskakINO_ArduFast`, yang punya pola sama) diam-diam "dibajak" jadi `log()` saat preprocessing, bentrok-ambigu dengan method `log()` yang sudah ada — `error: call of overloaded 'log(...)' is ambiguous`. Diperbaiki dengan `#undef log` **dan** `#undef logf` (di-guard `#ifdef`, aman di platform yang tidak punya macro ini) di `core/IskakINO_Logger.h` — fix terpusat di sini otomatis berlaku ke semua modul yang compose Logger (Storage, LCD, SmartVoice), bukan cuma ArduFast.
- `IskakINO_Scheduler.h` — task manager non-blocking `every()`/`once()`/`reset()`/`cancel()` (diekstrak dari ArduFast).
- `IskakINO_Version.h` — satu sumber versi (`ISKAKINO_VERSION`) untuk seluruh library.
- `IskakINO_Module.h` + `IskakINO_Kernel.h` — **lapisan framework opsional**: interface `begin()`/`update()` seragam + kernel/registry global (`IskakINO`) yang otomatis memanggil `begin()`/`update()` semua modul terdaftar. Tiap modul asli (ArduFast/Storage/LCD/WifiPortal/FastNTP/SmartVoice) TIDAK diubah sama sekali — didaftarkan lewat kelas adapter tipis (`IskakINO_*Module`) satu per modul, supaya API publik yang sudah diverifikasi lintas 6 pilot migrasi tetap 100% backward-compatible. Pola manual (panggil `begin()`/`tick()`/`update()` tiap modul sendiri-sendiri) tetap didukung penuh — kernel murni kenyamanan opsional. Lihat `examples/08_Framework_Kernel`.

### ardufast/ (dari IskakINO_ArduFast v1.1.0)
- Kini compose `IskakINO_Scheduler` & `IskakINO_Logger` dari core, bukan duplikasi kode sendiri.
- **Baru:** `setDebug(bool)` — bungkam logging tanpa hapus baris `logf()` di sketsa.
- Perilaku publik 100% identik (diverifikasi lewat re-run test suite asli, 20/20 lolos).

### storage/ (dari IskakINO_Storage v1.1.0)
- `enum class IskakStorageResult` lokal dihapus, kini alias dari `core/IskakINO_Result.h` (nilai numerik identik).
- Logging (`_printDebug`) kini lewat `IskakINO_Logger`.
- **Baru:** `setDebug(bool)` — ubah mode debug tanpa `begin()` ulang.
- **Bug fix (ditemukan lewat CI sungguhan `arduino:avr:uno`, bukan mock):** `#include <type_traits>` gagal compile di avr-gcc lama milik core `arduino:avr` (`fatal error: type_traits: No such file or directory`). Diganti builtin compiler GCC (`__has_trivial_copy`/`__has_trivial_destructor`) yang tidak butuh header apa pun, tersedia di semua toolchain berbasis GCC tanpa kecuali — `static_assert` trivially-copyable di `save()`/`load()` tetap berfungsi sama persis.

### wifi/ (dari IskakINO_WifiPortal v1.1.0)
- Macro platform (`#if defined(ESP32)`, dll.) terpusat lewat `core/IskakINO_Platform.h`.
- Portal timeout & auto-reconnect 30 detik kini pakai `IskakINO_Scheduler` bersama (dulu masing-masing variabel `millis()` manual).
- **Bug fix:** varian ESP8266 di `saveParams()`/`saveWifiList()`/`loadParams()`/`loadWifiList()` tidak pernah memvalidasi `LittleFS.open()` — sekarang divalidasi.
- **Baru:** `lastError()` (`IskakINO_Result`), `setDebug(bool)`.
- **Perbaikan struktural:** seluruh header/cpp kini dibungkus `#if defined(ISKAKINO_HAS_WIFI)` supaya aman di-include dari `IskakINO.h` di board non-WiFi (AVR) tanpa gagal compile.

### examples/
- **Bug fix (ditemukan lewat CI sungguhan `arduino:avr:uno`):** `04_SmartVoice_PlayTrack` hardcode `Serial2` untuk komunikasi ke DFPlayer Mini — gagal compile di Arduino Uno/Nano (`error: 'Serial2' was not declared in this scope`) karena ATmega328P cuma punya SATU UART hardware. Diperbaiki jadi portable: `SoftwareSerial` (pin 10/11) untuk board tanpa UART kedua, `Serial2` asli tetap dipakai di ESP32 (punya UART hardware ekstra, lebih baik daripada SoftwareSerial).

### storage, lcd, voice, ntp, wifi — semua modul (rangkuman tambahan opsional)
- `lcd/` (dari IskakINO_LiquidCrystal_I2C v1.1.0): backlight auto-timeout & interval typewriter/scroll kini pakai `IskakINO_Scheduler` bersama. `LCD_ENABLE_SERIAL_DEBUG` (compile-time) tetap didukung, plus `setDebug()` runtime baru.
- `voice/` (dari IskakINO_SmartVoice, rilis sebelumnya belum bernomor versi resmi): **baru** `IskakINO_Logger` (default nonaktif) dan `lastError()` (`IskakINO_Result`) — banyak fungsi yang dulu diam-diam gagal validasi kini punya `NOT_CONNECTED`/`INVALID_ARG`/`TIMEOUT`/`WRITE_FAILED` yang bisa dicek.
- `ntp/` (dari IskakINO_FastNTP v1.1.0): **baru** `IskakINO_Logger` opsional (default nonaktif). State machine backoff/rotate sengaja **tidak** dipetakan ke Scheduler (lihat komentar di header) karena logikanya terlalu spesifik.
- `ntp/` dan `wifi/`: kedua header dibungkus `#if defined(ISKAKINO_HAS_WIFI)` secara menyeluruh (silent-skip di board non-WiFi, bukan `#error`) — penting karena Arduino mengkompilasi semua `.cpp` di `src/` terlepas dari pemakaian sketch.

---

## Histori versi sebelum penggabungan (per-modul, standalone)

### IskakINO_ArduFast
- **1.1.0** — Runtime-configurable task slot, `once()`/`reset()`/`cancel()`, `readEMA()`, `logFloat()`, `logf()`, `library.json`, native unit test.
- **1.0.1** — `FastPin<P>` diperbaiki jadi direct register access sungguhan (sebelumnya cuma wrapper `digitalWrite()`).
- **1.0.0** — Rilis awal.

### IskakINO_Storage
- **1.1.0** — `reserve()`, `exists()`/`remove()`, `IskakStorageResult`+`lastError()`, callback korup/migrasi, `saveString()`/`loadString()`, mode log ring-buffer, enkripsi XOR, dukungan RP2040.
- **1.0.1** — Fix instance global `IskakStorage` yang tidak pernah didefinisikan, `save()` selalu return true, validasi `File::open()` ESP8266, bounds-check EEPROM AVR.
- **1.0.0** — Rilis awal.

### IskakINO_WifiPortal
- **1.1.0** — Multi-WiFi dengan fallback RSSI, `beginAsync()` non-blocking, `isConnected()`/`state()`, deteksi OS captive portal, `setAdminPin()`.
- **1.0.1** — Rilis pertama di Arduino Library Manager.
- **1.0.0** — Rilis awal.

### IskakINO_FastNTP
- **1.1.0** — Multi-server failover, `getUtcEpoch()`/`setUtcEpoch()`, event callback, deteksi Kiss-of-Death (RFC 5905), dukungan Ethernet shield.
- **1.0.1** — Fix `isAlarmActive()` one-shot, validasi ukuran paket UDP, overflow `isTimeReliable()`.
- **1.0.0** — Rilis awal.

### IskakINO_LiquidCrystal_I2C
- **1.1.0** — Typewriter/scroll non-blocking, `printCenter`/`typewriter` overload `const char*`, backlight auto-timeout, progress bar, safe multi-LCD init, `printFormatted()`, `IskakINO_LCD_Icons.h`.
- **1.0.3** — Rilis di Arduino Library Manager.
- **1.0.0** — Rilis awal.

### IskakINO_SmartVoice
- **1.0.0** — Rilis awal: kontrol DFPlayer Mini via `Stream&`, fix `isSDCardReady()` false positive, null pointer guard, checksum di `readResponse()`.
