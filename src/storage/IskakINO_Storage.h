/*
 * src/storage/IskakINO_Storage.h
 * Hybrid & Advanced Storage Engine (EEPROM, LittleFS, Preferences)
 *
 * PILOT REFACTOR #3 — bagian dari penggabungan ekosistem IskakINO (setelah
 * ArduFast & WifiPortal). Signature semua fungsi PUBLIK di bawah TIDAK
 * BERUBAH dari v1.1.0 standalone. Yang berubah cuma internal:
 *   - #if defined(ESP32)/ESP8266||RP2040 mentah kini pakai
 *     ISKAKINO_HAS_PREFS/ISKAKINO_HAS_LITTLEFS dari core/IskakINO_Platform.h
 *     (grouping ESP8266+RP2040 ke LittleFS tetap sama persis).
 *   - `enum class IskakStorageResult` lokal DIHAPUS, digantikan alias dari
 *     core/IskakINO_Result.h (`IskakINO_Result` + typedef IskakStorageResult).
 *     Urutan & nilai numerik tiap kode SAMA PERSIS (OK=0, NOT_FOUND=1,
 *     CRC_MISMATCH=2, OUT_OF_BOUNDS=3, WRITE_FAILED=4, VERSION_MISMATCH=5)
 *     — sengaja didesain begitu sejak core/IskakINO_Result.h dibuat, supaya
 *     modul ini bisa migrasi tanpa mengubah makna nilai lastError() yang
 *     mungkin sudah dibandingkan/disimpan di kode konsumen.
 *   - _printDebug() lokal digantikan IskakINO_Logger (_logger), pola sama
 *     seperti ArduFast & WifiPortal.
 *
 * v1.1.0 - Penambahan fitur besar (backward-compatible dengan v1.0.x):
 *   - reserve()            : auto-allocator alamat slot
 *   - exists() / remove()  : cek & hapus satu slot tanpa load penuh
 *   - lastError()          : status detail (IskakStorageResult) selain bool
 *   - onCorrupt() / onVersionMismatch() : callback untuk data korup & migrasi skema
 *   - saveString()/loadString() : dukungan Arduino String (panjang variabel, aman)
 *   - beginLog()/addLog()/readLog()/logCount()/clearLog() : mode ring-buffer/log
 *   - beginEncrypted()     : enkripsi ringan XOR-stream opsional
 *   - Dukungan platform RP2040 (LittleFS, mengikuti jalur ESP8266)
 *   - static_assert menolak tipe non trivially-copyable di save()/load()
 *
 * v1.0.1 - Bug fixes from traits problem (lihat CHANGELOG.md)
 * v1.0.0 - Rilis awal
 */

#ifndef ISKAKINO_STORAGE_H
#define ISKAKINO_STORAGE_H

#include <Arduino.h>

// PENTING: <type_traits> TIDAK tersedia di avr-gcc lama yang dipakai core
// arduino:avr (terkonfirmasi gagal compile di CI sungguhan, lihat
// CHANGELOG.md). Diganti dengan builtin compiler GCC (__has_trivial_copy /
// __has_trivial_destructor) yang TIDAK butuh header apa pun -- tersedia di
// semua toolchain berbasis GCC (avr-gcc, xtensa-esp32-elf-gcc, dst.) tanpa
// kecuali. Hasilnya semantically setara dengan std::is_trivially_copyable
// untuk kebutuhan static_assert di bawah (menolak tipe dengan
// constructor/destructor non-trivial, mis. yang mengandung String/pointer
// dgn ownership).
#define ISKAKINO_IS_TRIVIALLY_COPYABLE(T) (__has_trivial_copy(T) && __has_trivial_destructor(T))

#include "core/IskakINO_Platform.h"
#include "core/IskakINO_Result.h"
#include "core/IskakINO_Logger.h"
#include "core/IskakINO_Version.h"

#if defined(ISKAKINO_HAS_PREFS)
  #include <Preferences.h>
#elif defined(ISKAKINO_HAS_LITTLEFS)
  #include <LittleFS.h>
#else
  #include <EEPROM.h>
#endif

#define ISKAK_STORAGE_MAGIC 0x49
#define ISKAK_STORAGE_VERSION 0x03

class IskakINO_Storage {
  private:
    IskakINO_Logger _logger;
    const char* _namespace = "iskak_store";
    int _nextAddress = 0;
    IskakStorageResult _lastError = IskakStorageResult::OK;

    // --- Enkripsi ringan opsional (XOR-stream, BUKAN pengganti AES) ---
    uint8_t _encKey[16];
    size_t _encKeyLen = 0;
    bool _encrypted = false;

    // --- Callback opsional ---
    void (*_onCorruptCb)(int address) = nullptr;
    bool (*_onVersionMismatchCb)(uint8_t storedVersion, uint8_t currentVersion) = nullptr;

    // --- Mode log (ring buffer) ---
    int _logMetaAddr = -1;
    int _logDataBaseAddr = -1;
    uint16_t _logMaxEntries = 0;

    #if defined(ISKAKINO_HAS_PREFS)
      Preferences _prefs;
    #endif

    struct DataWrapper {
      uint8_t magic;
      uint8_t version;
      uint32_t crc;
    };

    struct LogMeta {
      uint16_t writeIndex;
      uint16_t count;
    };

    uint32_t _calculateCRC32(const uint8_t *data, size_t length) {
      uint32_t crc = 0xFFFFFFFF;
      for (size_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
          if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
          else crc >>= 1;
        }
      }
      return ~crc;
    }

    // Wrapper tipis ke _logger supaya SEMUA call site _printDebug(F(...))
    // yang sudah ada di bawah tidak perlu diubah satu-satu. Tag "[IskakStorage]"
    // dipertahankan di sini (bukan "[LOG]" generik milik Logger) supaya teks
    // yang tampil di Serial persis sama seperti v1.1.0 lama.
    void _printDebug(const __FlashStringHelper* msg) {
      if (!_logger.isDebug()) return;
      Serial.print(F("[IskakStorage] "));
      Serial.println(msg);
    }

    void _xorCrypt(uint8_t* buffer, size_t len) {
      if (!_encrypted || _encKeyLen == 0) return;
      for (size_t i = 0; i < len; i++) {
        buffer[i] ^= _encKey[i % _encKeyLen];
      }
    }

    // Menulis satu slot (header + data mentah berukuran dataLen) ke storage aktif.
    bool _writeSlot(int address, const uint8_t* data, size_t dataLen) {
      DataWrapper header;
      header.magic = ISKAK_STORAGE_MAGIC;
      header.version = ISKAK_STORAGE_VERSION;
      header.crc = _calculateCRC32(data, dataLen);

      size_t totalLen = sizeof(DataWrapper) + dataLen;
      uint8_t* buffer = new uint8_t[totalLen];
      memcpy(buffer, &header, sizeof(DataWrapper));
      memcpy(buffer + sizeof(DataWrapper), data, dataLen);
      _xorCrypt(buffer, totalLen);

      bool ok = false;

      #if defined(ISKAKINO_HAS_PREFS)
        char key[15]; sprintf(key, "a%d", address);
        size_t written = _prefs.putBytes(key, buffer, totalLen);
        ok = (written == totalLen);
        _lastError = ok ? IskakStorageResult::OK : IskakStorageResult::WRITE_FAILED;

      #elif defined(ISKAKINO_HAS_LITTLEFS)
        char path[20]; sprintf(path, "/s%d.bin", address);
        File f = LittleFS.open(path, "w");
        if (!f) {
          _lastError = IskakStorageResult::WRITE_FAILED;
          delete[] buffer;
          return false;
        }
        size_t written = f.write(buffer, totalLen);
        f.close();
        ok = (written == totalLen);
        _lastError = ok ? IskakStorageResult::OK : IskakStorageResult::WRITE_FAILED;

      #else
        if (address < 0 || (size_t)address + totalLen > (size_t)EEPROM.length()) {
          _lastError = IskakStorageResult::OUT_OF_BOUNDS;
          _printDebug(F("AVR: Address out of EEPROM bounds."));
          delete[] buffer;
          return false;
        }
        bool changed = false;
        for (size_t i = 0; i < totalLen; i++) {
          if (EEPROM.read(address + i) != buffer[i]) { changed = true; break; }
        }
        if (changed) {
          for (size_t i = 0; i < totalLen; i++) EEPROM.write(address + i, buffer[i]);
          _printDebug(F("AVR: EEPROM Updated."));
        } else {
          _printDebug(F("AVR: Skipped (No Change)."));
        }
        ok = true;
        _lastError = IskakStorageResult::OK;
      #endif

      delete[] buffer;
      return ok;
    }

    // Membaca satu slot dari storage aktif ke buffer outData (harus berukuran dataLen).
    bool _readSlot(int address, uint8_t* outData, size_t dataLen) {
      size_t totalLen = sizeof(DataWrapper) + dataLen;
      uint8_t* buffer = new uint8_t[totalLen];
      bool readOk = true;

      #if defined(ISKAKINO_HAS_PREFS)
        char key[15]; sprintf(key, "a%d", address);
        if (_prefs.getBytes(key, buffer, totalLen) != totalLen) readOk = false;

      #elif defined(ISKAKINO_HAS_LITTLEFS)
        char path[20]; sprintf(path, "/s%d.bin", address);
        if (!LittleFS.exists(path)) {
          readOk = false;
        } else {
          File f = LittleFS.open(path, "r");
          if (!f) {
            readOk = false;
          } else {
            size_t r = f.read(buffer, totalLen);
            f.close();
            if (r != totalLen) readOk = false;
          }
        }

      #else
        if (address < 0 || (size_t)address + totalLen > (size_t)EEPROM.length()) {
          _lastError = IskakStorageResult::OUT_OF_BOUNDS;
          delete[] buffer;
          return false;
        }
        for (size_t i = 0; i < totalLen; i++) buffer[i] = EEPROM.read(address + i);
      #endif

      if (!readOk) {
        _lastError = IskakStorageResult::NOT_FOUND;
        delete[] buffer;
        return false;
      }

      _xorCrypt(buffer, totalLen);

      DataWrapper header;
      memcpy(&header, buffer, sizeof(DataWrapper));
      if (header.magic != ISKAK_STORAGE_MAGIC) {
        _lastError = IskakStorageResult::NOT_FOUND;
        delete[] buffer;
        return false;
      }

      if (header.version != ISKAK_STORAGE_VERSION) {
        bool accept = true;
        if (_onVersionMismatchCb) accept = _onVersionMismatchCb(header.version, ISKAK_STORAGE_VERSION);
        if (!accept) {
          _lastError = IskakStorageResult::VERSION_MISMATCH;
          delete[] buffer;
          return false;
        }
        _printDebug(F("Versi data berbeda, diterima via callback migrasi."));
      }

      memcpy(outData, buffer + sizeof(DataWrapper), dataLen);
      uint32_t currentCrc = _calculateCRC32(outData, dataLen);
      delete[] buffer;

      if (currentCrc != header.crc) {
        _lastError = IskakStorageResult::CRC_MISMATCH;
        if (_onCorruptCb) _onCorruptCb(address);
        return false;
      }

      _lastError = IskakStorageResult::OK;
      return true;
    }

  public:
    void begin(const char* name = "iskak_store", bool debugMode = false) {
      _namespace = name;
      _logger.setDebug(debugMode);
      #if defined(ISKAKINO_HAS_PREFS)
        _prefs.begin(_namespace, false);
      #elif defined(ISKAKINO_HAS_LITTLEFS)
        LittleFS.begin();
      #endif
      _printDebug(F("Hybrid Engine Ready with CRC32 Protection."));
    }

    // Sama seperti begin(), tapi mengaktifkan enkripsi XOR-stream ringan.
    // CATATAN: ini BUKAN enkripsi kriptografis kuat (bukan AES) — cukup untuk
    // mempersulit orang awam membaca data mentah di EEPROM/flash secara langsung,
    // TIDAK untuk melindungi data sangat sensitif dari penyerang yang punya akses fisik.
    void beginEncrypted(const char* key, const char* name = "iskak_store", bool debugMode = false) {
      begin(name, debugMode);
      _encKeyLen = strlen(key);
      if (_encKeyLen > sizeof(_encKey)) _encKeyLen = sizeof(_encKey);
      memcpy(_encKey, key, _encKeyLen);
      _encrypted = (_encKeyLen > 0);
      _printDebug(F("Encryption enabled (XOR-stream)."));
    }

    // Dipanggil saat load() mendeteksi CRC mismatch (data korup).
    void onCorrupt(void (*callback)(int address)) {
      _onCorruptCb = callback;
    }

    // Dipanggil saat load() menemukan header.version berbeda dari versi library saat ini.
    // Return true dari callback untuk tetap menerima & lanjut proses data (mis. setelah
    // migrasi manual di sketch), return false untuk menolak (load() akan gagal).
    // Jika callback tidak diset, data versi lama tetap diterima secara default.
    void onVersionMismatch(bool (*callback)(uint8_t storedVersion, uint8_t currentVersion)) {
      _onVersionMismatchCb = callback;
    }

    // Status detail dari operasi save()/load() terakhir.
    IskakStorageResult lastError() const {
      return _lastError;
    }

    // BARU (pilot refactor): ubah mode debug setelah begin() dipanggil,
    // tanpa perlu begin() ulang. Sebelumnya debugMode hanya bisa diset
    // sekali lewat parameter begin()/beginEncrypted().
    void setDebug(bool debugMode) { _logger.setDebug(debugMode); }

    // Auto-allocator alamat slot sederhana: berguna supaya Anda tidak perlu
    // menghitung offset manual antar beberapa save() dengan tipe berbeda.
    //   int addrConfig = IskakStorage.reserve(sizeof(Config));
    //   int addrStats  = IskakStorage.reserve(sizeof(Stats));
    int reserve(size_t dataSize) {
      int addr = _nextAddress;
      _nextAddress += (int)(dataSize + sizeof(DataWrapper));
      return addr;
    }

    // Cek apakah ada data (valid secara magic-byte) di suatu slot, tanpa load penuh.
    bool exists(int address) {
      #if defined(ISKAKINO_HAS_PREFS)
        char key[15]; sprintf(key, "a%d", address);
        return _prefs.isKey(key);
      #elif defined(ISKAKINO_HAS_LITTLEFS)
        char path[20]; sprintf(path, "/s%d.bin", address);
        return LittleFS.exists(path);
      #else
        if (address < 0 || (size_t)address >= (size_t)EEPROM.length()) return false;
        return EEPROM.read(address) == ISKAK_STORAGE_MAGIC;
      #endif
    }

    // Menghapus satu slot saja (bukan seluruh storage seperti clear()).
    bool remove(int address) {
      #if defined(ISKAKINO_HAS_PREFS)
        char key[15]; sprintf(key, "a%d", address);
        return _prefs.remove(key);
      #elif defined(ISKAKINO_HAS_LITTLEFS)
        char path[20]; sprintf(path, "/s%d.bin", address);
        if (!LittleFS.exists(path)) return false;
        return LittleFS.remove(path);
      #else
        if (address < 0 || (size_t)address >= (size_t)EEPROM.length()) return false;
        EEPROM.write(address, 0xFF); // cukup invalidasi magic byte
        return true;
      #endif
    }

    // --- FUNGSI: CLEAR (FACTORY RESET) — menghapus SEMUA data ---
    void clear() {
      _printDebug(F("Clearing all data (Factory Reset)..."));
      #if defined(ISKAKINO_HAS_PREFS)
        _prefs.clear();
      #elif defined(ISKAKINO_HAS_LITTLEFS)
        Dir dir = LittleFS.openDir("/");
        while (dir.next()) {
          LittleFS.remove(dir.fileName());
        }
      #else
        for (int i = 0; i < (int)EEPROM.length(); i++) {
          if (EEPROM.read(i) != 0xFF) EEPROM.write(i, 0xFF);
        }
      #endif
      _printDebug(F("Storage cleared successfully."));
    }

    template <typename T>
    bool save(int address, const T& value) {
      static_assert(ISKAKINO_IS_TRIVIALLY_COPYABLE(T),
        "IskakINO_Storage::save() hanya mendukung tipe trivially-copyable "
        "(struct/array/primitif tanpa pointer/String). Gunakan saveString() untuk teks.");
      return _writeSlot(address, (const uint8_t*)&value, sizeof(T));
    }

    template <typename T>
    bool load(int address, T& value) {
      static_assert(ISKAKINO_IS_TRIVIALLY_COPYABLE(T),
        "IskakINO_Storage::load() hanya mendukung tipe trivially-copyable "
        "(struct/array/primitif tanpa pointer/String). Gunakan loadString() untuk teks.");
      return _readSlot(address, (uint8_t*)&value, sizeof(T));
    }

    // Simpan Arduino String dengan panjang variabel (aman, tidak seperti save<String>()
    // yang salah karena hanya menyalin objek String, bukan isi teksnya).
    // 'maxLen' menentukan kapasitas slot — gunakan nilai yang SAMA persis saat
    // saveString() maupun loadString() untuk slot address yang sama.
    bool saveString(int address, const String& value, size_t maxLen = 256) {
      size_t len = value.length();
      if (len > maxLen) len = maxLen;
      size_t totalDataSize = sizeof(uint16_t) + maxLen;
      uint8_t* dataBuf = new uint8_t[totalDataSize];
      memset(dataBuf, 0, totalDataSize);
      uint16_t len16 = (uint16_t)len;
      memcpy(dataBuf, &len16, sizeof(uint16_t));
      memcpy(dataBuf + sizeof(uint16_t), value.c_str(), len);
      bool ok = _writeSlot(address, dataBuf, totalDataSize);
      delete[] dataBuf;
      return ok;
    }

    bool loadString(int address, String& value, size_t maxLen = 256) {
      size_t totalDataSize = sizeof(uint16_t) + maxLen;
      uint8_t* dataBuf = new uint8_t[totalDataSize];
      bool ok = _readSlot(address, dataBuf, totalDataSize);
      if (!ok) { delete[] dataBuf; return false; }

      uint16_t len16;
      memcpy(&len16, dataBuf, sizeof(uint16_t));
      if (len16 > maxLen) {
        _lastError = IskakStorageResult::OUT_OF_BOUNDS;
        delete[] dataBuf;
        return false;
      }
      value = "";
      value.reserve(len16);
      for (size_t i = 0; i < len16; i++) value += (char)dataBuf[sizeof(uint16_t) + i];
      delete[] dataBuf;
      return true;
    }

    // --- Mode Log (Ring Buffer) ---
    // Cocok untuk data-logging (histori sensor, event, dll.) yang otomatis
    // menimpa entri terlama saat penuh (wrap-around).
    //   IskakStorage.beginLog(500, 510, 20); // metaAddr, dataBaseAddr, maxEntries
    void beginLog(int metaAddress, int dataBaseAddress, uint16_t maxEntries) {
      _logMetaAddr = metaAddress;
      _logDataBaseAddr = dataBaseAddress;
      _logMaxEntries = maxEntries;
    }

    template <typename T>
    bool addLog(const T& entry) {
      static_assert(ISKAKINO_IS_TRIVIALLY_COPYABLE(T),
        "IskakINO_Storage::addLog() hanya mendukung tipe trivially-copyable.");
      if (_logMetaAddr < 0 || _logMaxEntries == 0) return false;

      LogMeta meta;
      if (!load(_logMetaAddr, meta)) { meta.writeIndex = 0; meta.count = 0; }

      int slotAddr = _logDataBaseAddr + meta.writeIndex * (int)(sizeof(DataWrapper) + sizeof(T));
      bool ok = save(slotAddr, entry);
      if (ok) {
        meta.writeIndex = (meta.writeIndex + 1) % _logMaxEntries;
        if (meta.count < _logMaxEntries) meta.count++;
        save(_logMetaAddr, meta);
      }
      return ok;
    }

    // indexFromOldest: 0 = entri paling lama yang masih tersimpan, count()-1 = entri terbaru.
    template <typename T>
    bool readLog(uint16_t indexFromOldest, T& entry) {
      static_assert(ISKAKINO_IS_TRIVIALLY_COPYABLE(T),
        "IskakINO_Storage::readLog() hanya mendukung tipe trivially-copyable.");
      if (_logMetaAddr < 0 || _logMaxEntries == 0) return false;

      LogMeta meta;
      if (!load(_logMetaAddr, meta)) return false;
      if (indexFromOldest >= meta.count) return false;

      uint16_t startIdx = (meta.count < _logMaxEntries) ? 0 : meta.writeIndex;
      uint16_t actualIdx = (startIdx + indexFromOldest) % _logMaxEntries;
      int slotAddr = _logDataBaseAddr + actualIdx * (int)(sizeof(DataWrapper) + sizeof(T));
      return load(slotAddr, entry);
    }

    uint16_t logCount() {
      if (_logMetaAddr < 0) return 0;
      LogMeta meta;
      if (!load(_logMetaAddr, meta)) return 0;
      return meta.count;
    }

    void clearLog() {
      if (_logMetaAddr < 0) return;
      LogMeta meta; meta.writeIndex = 0; meta.count = 0;
      save(_logMetaAddr, meta);
    }
};

extern IskakINO_Storage IskakStorage;
#endif
