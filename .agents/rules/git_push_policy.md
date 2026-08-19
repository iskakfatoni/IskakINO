# Aturan Git Push Khusus Proyek IskakINO

- **JANGAN LAKUKAN `git push`** secara otomatis atau tanpa perintah eksplisit dari pengguna.
- Eksekusi `git push` hanya boleh dilakukan jika pengguna secara langsung memberikan instruksi seperti "push", "push ke github", "push github", dll.
- **Alasan:** GitHub Actions CI membutuhkan waktu yang cukup lama untuk mengompilasi seluruh board matrix (AVR, ESP8266, ESP32), sehingga push harus diminimalkan dan hanya dilakukan atas permintaan eksplisit.
