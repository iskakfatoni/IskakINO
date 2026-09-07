/*
 * src/core/IskakINO_Result.h
 * Status detail hasil operasi, dipakai bersama oleh seluruh modul IskakINO.
 *
 * SUMBER: digeneralisasi dari `enum class IskakStorageResult` di
 * IskakINO_Storage v1.1.0. Semua fungsi publik modul (save()/load(),
 * saveParams()/loadParams() di WifiPortal, dll.) tetap mengembalikan bool
 * untuk kompatibilitas mundur; gunakan lastError() masing-masing modul
 * untuk detail penyebabnya lewat enum ini.
 *
 * Kode WRITE_FAILED s.d. VERSION_MISMATCH persis sama urutan & nilainya
 * dengan IskakStorageResult v1.1.0 supaya modul Storage bisa migrasi tanpa
 * mengubah makna nilai yang mungkin sudah tersimpan/dibandingkan user.
 * Kode baru (TIMEOUT, INVALID_ARG, NOT_CONNECTED, ALREADY_EXISTS) ditambah
 * di akhir untuk kebutuhan WifiPortal/FastNTP/SmartVoice saat modul-modul
 * itu bermigrasi ke enum bersama ini.
 */

#ifndef ISKAKINO_RESULT_H
#define ISKAKINO_RESULT_H

#include <Arduino.h>

enum class IskakINO_Result : uint8_t {
  OK = 0,
  NOT_FOUND,
  CRC_MISMATCH,
  OUT_OF_BOUNDS,
  WRITE_FAILED,
  VERSION_MISMATCH,
  TIMEOUT,          // baru: dipakai FastNTP/WifiPortal (batas waktu operasi tercapai)
  INVALID_ARG,       // baru: parameter/argumen yang diberikan tidak valid
  NOT_CONNECTED,     // baru: dipakai WifiPortal (belum ada koneksi WiFi)
  ALREADY_EXISTS     // baru: dipakai Storage.reserve()/exists()
};

// Alias untuk kompatibilitas mundur dengan kode IskakINO_Storage v1.0.x/1.1.0
// yang sudah beredar (mis. contoh sketsa lama yang memakai nama lama).
// Nilai numeriknya identik dengan IskakINO_Result di atas untuk kode yang sama.
typedef IskakINO_Result IskakStorageResult;

// Representasi string singkat untuk logging/debug (bahasa Indonesia teknis,
// konsisten dengan gaya pesan log modul-modul lain).
inline const char* IskakINO_ResultToString(IskakINO_Result r) {
  switch (r) {
    case IskakINO_Result::OK:               return "OK";
    case IskakINO_Result::NOT_FOUND:        return "NOT_FOUND";
    case IskakINO_Result::CRC_MISMATCH:     return "CRC_MISMATCH";
    case IskakINO_Result::OUT_OF_BOUNDS:    return "OUT_OF_BOUNDS";
    case IskakINO_Result::WRITE_FAILED:     return "WRITE_FAILED";
    case IskakINO_Result::VERSION_MISMATCH: return "VERSION_MISMATCH";
    case IskakINO_Result::TIMEOUT:          return "TIMEOUT";
    case IskakINO_Result::INVALID_ARG:      return "INVALID_ARG";
    case IskakINO_Result::NOT_CONNECTED:    return "NOT_CONNECTED";
    case IskakINO_Result::ALREADY_EXISTS:   return "ALREADY_EXISTS";
    default:                                return "UNKNOWN";
  }
}

#endif
