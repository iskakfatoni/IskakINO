/*
 * src/IskakINO_Storage.cpp
 * Implementasi instance global untuk IskakINO_Storage.
 * Digunakan untuk mengalokasikan memori objek IskakStorage agar dapat diakses
 * di seluruh file project tanpa konflik definisi.
 *
 * Library ini header-only untuk sebagian besar implementasinya (lihat .h);
 * file ini hanya menyediakan SATU definisi instance global `IskakStorage`,
 * sesuai deklarasi `extern IskakINO_Storage IskakStorage;` di header.
 *
 * PENTING: jangan deklarasikan ulang `IskakINO_Storage IskakStorage;` di
 * sketch/.ino Anda — itu akan menyebabkan error linker "multiple definition".
 *
 * Created for: iskakfatoni (2026-02-16)
 */

#include "IskakINO_Storage.h"

IskakINO_Storage IskakStorage;
