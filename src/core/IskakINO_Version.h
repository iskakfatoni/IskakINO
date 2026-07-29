/*
 * src/core/IskakINO_Version.h
 * Satu sumber versi untuk SELURUH library IskakINO gabungan.
 *
 * Sebelum penggabungan, tiap modul (IskakINO_ArduFast, IskakINO_Storage,
 * IskakINO_WifiPortal, dst.) punya nomor versi & tag rilis sendiri-sendiri
 * di repo masing-masing. Setelah digabung jadi satu library, hanya ADA SATU
 * versi: versi IskakINO itu sendiri, sinkron dengan satu library.properties
 * dan satu tag Git (tanpa prefix 'v', mis. "1.0.0").
 *
 * Makro versi lama tiap modul (ARDUFAST_VERSION, dll.) TIDAK dihapus —
 * dibekukan sebagai konstanta historis yang menandai "kode modul ini
 * berasal dari rilis standalone versi berapa", supaya pola lama di kode
 * konsumen (#if ARDUFAST_VERSION >= 10100) tetap valid. Tapi makro itu
 * TIDAK lagi naik mengikuti rilis IskakINO — yang naik cuma ISKAKINO_VERSION
 * di bawah ini.
 */

#ifndef ISKAKINO_VERSION_H
#define ISKAKINO_VERSION_H

// Versi awal library gabungan. Mulai dari 1.0.0 sebagai produk baru
// (bukan melanjutkan nomor versi salah satu modul lama) karena strukturnya
// memang berbeda: satu library.properties, satu release cycle untuk semua
// modul sekaligus. Naikkan angka ini saat rilis, ikuti Semantic Versioning:
//   MAJOR : perubahan API yang tidak backward-compatible di modul mana pun
//   MINOR : fitur/modul baru ditambahkan, tetap backward-compatible
//   PATCH : bug fix saja, tidak ada perubahan API
#define ISKAKINO_VERSION_MAJOR 1
#define ISKAKINO_VERSION_MINOR 0
#define ISKAKINO_VERSION_PATCH 0
#define ISKAKINO_VERSION ((ISKAKINO_VERSION_MAJOR * 10000UL) + \
                           (ISKAKINO_VERSION_MINOR * 100UL) + \
                           (ISKAKINO_VERSION_PATCH))

#endif
