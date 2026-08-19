# Project Rules & Preferences — IskakINO

## Git & Workflow Policy
- **Larangan Auto Push:** JANGAN PERNAH menjalankan perintah `git push` ke remote repository kecuali pengguna secara eksplisit menginstruksikannya (misal: perintah "push github", "push ke repo").
- **Alasan:** GitHub Actions CI membutuhkan waktu yang cukup lama untuk menjalankan seluruh build matrix (`arduino:avr:uno`, `esp8266:esp8266:nodemcuv2`, `esp32:esp32:esp32`), sehingga push hanya dilakukan saat pengguna benar-benar menginginkannya.
- **Cek Status CI Sebelum Push (Wajib Batal jika Sibuk):** Saat pengguna menginstruksikan `git push`, pastikan untuk memeriksa status GitHub Actions terlebih dahulu (via `gh run list --limit 1`). Jika masih ada workflow yang sedang berjalan (*in_progress* / *queued*), **BATALKAN perintah push seketika** dan beri tahu pengguna bahwa Actions sedang sibuk/tidak *free*, sehingga push baru akan dilakukan jika diinstruksikan lagi saat CI sudah *free/idle*.
- **Larangan Compile Test Lokal:** JANGAN menjalankan pengujian kompilasi (`arduino-cli compile` atau sejenisnya) di environment lokal karena prosesnya memakan waktu dan resource yang besar. Serahkan pengujian kompilasi sepenuhnya ke **GitHub Actions CI** (`.github/workflows/ci.yml`) saat commit di-push ke repository.
