# Project Rules & Preferences — IskakINO

## Git & Workflow Policy
- **Larangan Auto Push:** JANGAN PERNAH menjalankan perintah `git push` ke remote repository kecuali pengguna secara eksplisit menginstruksikannya (misal: perintah "push github", "push ke repo").
- **Alasan:** GitHub Actions CI membutuhkan waktu yang cukup lama untuk menjalankan seluruh build matrix (`arduino:avr:uno`, `esp8266:esp8266:nodemcuv2`, `esp32:esp32:esp32`), sehingga push hanya dilakukan saat pengguna benar-benar menginginkannya.
