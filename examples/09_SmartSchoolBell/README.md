# IskakINO — Smart School Bell (Bel Sekolah Otomatis Multi-Profile)

[![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20ESP8266-green.svg)](#)
[![Library](https://img.shields.io/badge/Library-IskakINO%20v1.0.0-blue.svg)](#)
[![License](https://img.shields.io/badge/License-MIT-lightgrey.svg)](#)

Contoh proyek komprehensif sistem **Bel Sekolah Otomatis Berbasis Suara MP3, Multi-Profile, dan Web Portal** yang memanfaatkan integrasi penuh **6 modul library IskakINO** (`ArduFast`, `Storage`, `LCD`, `SmartVoice`, `WifiPortal`, `FastNTP`).

---

## 📌 Fitur Utama

1. **Waktu Akurat Otomatis via NTP**: Sinkronisasi waktu SNTP via internet (WIB/WITA/WIT) dengan fallback waktu saat offline dari `IskakStorage`.
2. **Sistem Multi-Profile Fleksibel**:
   * `Otomatis`: Menyesuaikan hari (Senin-Kamis = Reguler, Jumat = Khusus Jumat, Minggu = Libur).
   * `Reguler`: Jadwal kegiatan belajar mengajar harian normal.
   * `Khusus Jumat`: Jadwal dengan jam pulang lebih awal (sebelum ibadah Sholat Jumat).
   * `Ujian / Asesmen`: Jadwal khusus sesi ujian (UTS / UAS).
   * `Ramadhan`: Jadwal khusus bulan puasa (durasi jam dipersingkat).
   * `Libur`: Bel otomatis non-aktif.
3. **Pengumuman Suara MP3 (DFPlayer Mini)**: Memutar rekaman suara jam pelajaran, nada lonceng, lagu kebangsaan, atau audio pengumuman.
4. **Relay Amplifier Cerdas (*Anti-Pop / Zero-Humming*)**: Relay menyalakan daya amplifier 2 detik sebelum audio berbunyi dan mematikannya 1 detik setelah audio selesai.
5. **Dashboard Web Portal (Responsive UI)**: Pengaturan jadwal, ganti mode profil, uji coba audio, dan trigger bel manual dari smartphone/laptop.
6. **Dukungan Microsoft Excel (.xlsx)**: Upload dan download template jadwal sekolah langsung via browser.
7. **Tampilan LCD 16x2 I2C**: Menampilkan mode profil aktif, jam real-time, dan informasi jadwal bel terdekat berikutnya.
8. **Tombol Fisik Manual**: Pemicu bel manual/darurat instan dengan debounce cepat `FastPin`.

---

## 🔌 Skema Pengkabelan (Wiring Diagram ESP32)

### Tabel Pinout

| Komponen | Pin Modul | Pin ESP32 | Catatan Penting |
| :--- | :--- | :--- | :--- |
| **LCD 16x2 / 20x4 I2C** | `VCC` | **VIN (5V)** | Sumber daya 5V untuk kontras jelas |
| | `GND` | **GND** | Ground bersama (*Common GND*) |
| | `SDA` | **GPIO 21** | I2C Data Hardware |
| | `SCL` | **GPIO 22** | I2C Clock Hardware |
| **DFPlayer Mini MP3** | `VCC` | **VIN (5V)** | Sumber daya DFPlayer (5V) |
| | `GND` | **GND** | Ground bersama |
| | `RX` | **GPIO 17 (TX2)** | **Wajib seri Resistor 1kΩ** (peredam noise) |
| | `TX` | **GPIO 16 (RX2)** | Serial RX hardware ESP32 |
| | `BUSY` | **GPIO 4** | Status pemutaran audio |
| | `DAC_R / L` | **AUX IN (+)** | Ke terminal Audio Positif Amplifier |
| | `GND (Audio)` | **AUX IN (-)** | Ke terminal Ground Audio Amplifier |
| **Modul Relay 5V** | `VCC` | **VIN (5V)** | Daya koil relay |
| | `GND` | **GND** | Ground bersama |
| | `IN` | **GPIO 18** | Kontrol sakelar (Active HIGH) |
| | `COM & NO` | **Power Ampli** | Diseri ke kabel daya AC Amplifier |
| **Tombol Manual** | Pin 1 | **GPIO 19** | Menggunakan internal `INPUT_PULLUP` |
| | Pin 2 | **GND** | Tekan ke GND saat tombol aktif |

```text
                     +-----------------------------+
                     |         ESP32 DEVKIT        |
                     |                             |
  [LCD 16x2 I2C]     |                             |     [DFPlayer Mini]
  +------------+     |                             |     +-------------+
  |        VCC |<----+ VIN (5V)                    |     | VCC         |<----+ VIN (5V)
  |        GND |<----+ GND                         |     | GND         |<----+ GND
  |        SDA |<----+ GPIO 21                     |     | RX          |<----+ [Resistor 1kΩ] <-- GPIO 17 (TX2)
  |        SCL |<----+ GPIO 22                     |     | TX          +----> GPIO 16 (RX2)
  +------------+     |                             |     | BUSY        +----> GPIO 4
                     |                             |     | DAC_L / R   +---\
  [Relay 5V Module]  |                             |     | GND (Audio) +----\  (Ke AUX IN Amplifier)
  +------------+     |                             |     +-------------+     |
  |        VCC |<----+ VIN (5V)                    |                         |
  |        GND |<----+ GND                         |                         v
  |         IN |<----+ GPIO 18                     |                  +---------------+
  +------------+     |                             |                  | AMPLIFIER TOA |
       | (COM & NO)  |                             |                  |  (Kabel Daya  |
       \-------------+-----------------------------+                  |   dilewatkan  |
                     |                                                |   ke Relay)   |
  [Tombol Manual]    |                                                +---------------+
  +------------+     |
  |      Pin 1 |<----+ GPIO 19 (INPUT_PULLUP)
  |      Pin 2 |<----+ GND
  +------------+     +-----------------------------+
---

## 📅 Contoh Tabel Jadwal Default Sekolah

Berikut adalah referensi susunan jadwal default yang umum diterapkan di sekolah-sekolah di Indonesia:

### 1. Profil Reguler (Senin s/d Kamis & Sabtu)
| Jam | Kegiatan / Keterangan | Track MP3 | File Audio | Hari Aktif |
| :--- | :--- | :---: | :--- | :--- |
| **06:45** | Persiapan Masuk / Lagu Wajib | `05` | `0005.mp3` | Senin s/d Kamis, Sabtu |
| **07:00** | Bel Masuk Pagi & Doa Bersama | `01` | `0001.mp3` | Senin s/d Kamis, Sabtu |
| **07:45** | Ganti Jam Pelajaran ke-2 | `01` | `0001.mp3` | Senin s/d Kamis, Sabtu |
| **08:30** | Ganti Jam Pelajaran ke-3 | `01` | `0001.mp3` | Senin s/d Kamis, Sabtu |
| **09:15** | Ganti Jam Pelajaran ke-4 | `01` | `0001.mp3` | Senin s/d Kamis, Sabtu |
| **10:00** | **Waktu Istirahat Pertama (1)** | `02` | `0002.mp3` | Senin s/d Kamis, Sabtu |
| **10:30** | Masuk Kelas Setelah Istirahat 1 | `03` | `0003.mp3` | Senin s/d Kamis, Sabtu |
| **11:15** | Ganti Jam Pelajaran ke-6 | `01` | `0001.mp3` | Senin s/d Kamis, Sabtu |
| **12:00** | **Istirahat ke-2 & Sholat Dhuhur** | `02` | `0002.mp3` | Senin s/d Kamis, Sabtu |
| **12:45** | Masuk Kelas Setelah Istirahat 2 | `03` | `0003.mp3` | Senin s/d Kamis, Sabtu |
| **13:30** | Ganti Jam Pelajaran ke-8 | `01` | `0001.mp3` | Senin s/d Kamis, Sabtu |
| **14:15** | **Bel Waktu Pulang Sekolah** | `04` | `0004.mp3` | Senin s/d Kamis, Sabtu |

### 2. Profil Khusus Jumat (Pulang Lebih Awal)
| Jam | Kegiatan / Keterangan | Track MP3 | File Audio | Hari Aktif |
| :--- | :--- | :---: | :--- | :--- |
| **06:45** | Senam Pagi / Literasi Jumat | `05` | `0005.mp3` | Hanya Hari Jumat |
| **07:00** | Bel Masuk Jam Pelajaran ke-1 | `01` | `0001.mp3` | Hanya Hari Jumat |
| **07:40** | Ganti Jam Pelajaran ke-2 | `01` | `0001.mp3` | Hanya Hari Jumat |
| **08:20** | Ganti Jam Pelajaran ke-3 | `01` | `0001.mp3` | Hanya Hari Jumat |
| **09:00** | **Waktu Istirahat Jumat** | `02` | `0002.mp3` | Hanya Hari Jumat |
| **09:30** | Masuk Jam Pelajaran ke-4 | `03` | `0003.mp3` | Hanya Hari Jumat |
| **10:10** | Ganti Jam Pelajaran ke-5 | `01` | `0001.mp3` | Hanya Hari Jumat |
| **11:00** | **Pulang & Persiapan Sholat Jumat** | `04` | `0004.mp3` | Hanya Hari Jumat |

### 3. Profil Ujian / Asesmen (UTS / UAS)
| Jam | Kegiatan / Keterangan | Track MP3 | File Audio | Hari Aktif |
| :--- | :--- | :---: | :--- | :--- |
| **07:15** | Bel Masuk Ruang Ujian | `01` | `0001.mp3` | Hari Pelaksanaan Ujian |
| **07:30** | **Pengerjaan Ujian Sesi 1 Dimulai** | `06` | `0006.mp3` | Hari Pelaksanaan Ujian |
| **09:30** | Ujian Sesi 1 Selesai & Istirahat | `02` | `0002.mp3` | Hari Pelaksanaan Ujian |
| **10:00** | **Pengerjaan Ujian Sesi 2 Dimulai** | `06` | `0006.mp3` | Hari Pelaksanaan Ujian |
| **12:00** | Ujian Sesi 2 Selesai & Pulang | `04` | `0004.mp3` | Hari Pelaksanaan Ujian |

### 4. Profil Bulan Ramadhan (Durasi Khusus)
| Jam | Kegiatan / Keterangan | Track MP3 | File Audio | Hari Aktif |
| :--- | :--- | :---: | :--- | :--- |
| **07:30** | Bel Masuk Tadarus & Jam ke-1 | `01` | `0001.mp3` | Senin s/d Jumat |
| **08:05** | Ganti Jam Pelajaran ke-2 | `01` | `0001.mp3` | Senin s/d Jumat |
| **08:40** | Ganti Jam Pelajaran ke-3 | `01` | `0001.mp3` | Senin s/d Jumat |
| **09:15** | Ganti Jam Pelajaran ke-4 | `01` | `0001.mp3` | Senin s/d Jumat |
| **09:50** | **Istirahat & Sholat Dhuha** | `02` | `0002.mp3` | Senin s/d Jumat |
| **10:20** | Masuk Jam Pelajaran ke-5 | `03` | `0003.mp3` | Senin s/d Jumat |
| **10:55** | Ganti Jam Pelajaran ke-6 | `01` | `0001.mp3` | Senin s/d Jumat |
| **11:30** | Sholat Dhuhur Berjamaah | `02` | `0002.mp3` | Senin s/d Jumat |
| **12:15** | **Bel Pulang Sekolah (Ramadhan)** | `04` | `0004.mp3` | Senin s/d Jumat |

---

## 📁 Struktur File Audio pada MicroSD Card

1. Format kartu MicroSD ke **FAT32**.
2. Buat folder bernama **`mp3`** di root direktori MicroSD.
3. Beri nama file audio dengan format **4 digit angka**:

```text
MicroSD Root/
└── mp3/
    ├── 0001.mp3   # Nada Bel Masuk / Ganti Jam Pelajaran
    ├── 0002.mp3   # Nada Bel Waktu Istirahat
    ├── 0003.mp3   # Nada Bel Masuk Kelas Setelah Istirahat
    ├── 0004.mp3   # Nada Bel Waktu Pulang Sekolah
    ├── 0005.mp3   # Lagu Kebangsaan / Senam Pagi
    └── 0006.mp3   # Pengumuman Ujian Dimulai
```

---

## 🚀 Panduan Penggunaan & Web Portal

1. **Koneksi WiFi & Portal Konfigurasi**:
   * Saat pertama kali dinyalakan, ESP32 memancarkan Access Point bernama **`IskakINO-SchoolBell`**.
   * Buka browser di smartphone/laptop ke alamat `http://192.168.4.1` untuk memasukkan SSID & Password WiFi sekolah.
2. **Dashboard Kontrol Bel**:
   * Setelah tersambung ke WiFi, buka alamat IP ESP32 pada browser di path: `http://<IP-ESP32>/bell` (atau `http://192.168.4.1/bell` saat mode AP).
   * Pilih profil jadwal, tambah/edit jam pelajaran, atau bunyikan bel darurat dari jarak jauh.

---

## 📁 Struktur File di Folder Example

```text
examples/09_SmartSchoolBell/
├── 09_SmartSchoolBell.ino               # Sketch Arduino Utama (ESP32)
├── BellWebPage.h                        # Tampilan Web UI (HTML/CSS/JS di PROGMEM Flash)
├── README.md                            # Dokumentasi, Pinout & Panduan
└── Template_Jadwal_Bel_Sekolah.xlsx     # Template Excel Resmi (.xlsx)
```

---

## 📄 Lisensi

Bagian dari ekosistem [IskakINO Framework](https://github.com/iskakfatoni/IskakINO) — Lisensi MIT.
Dikembangkan oleh **Iskak Fatoni** — Nisnas Computer, SMKN 1 Jetis Mojokerto.
