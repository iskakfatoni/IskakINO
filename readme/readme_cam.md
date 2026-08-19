# 📷 Modul: IskakINO_Cam (Khusus ESP32)

Driver modul kamera ESP32 (**ESP32-CAM AI-Thinker**, M5Stack Cam, WROVER-KIT, ESP-EYE) dengan manajemen buffer cerdas, deteksi otomatis PSRAM, kontrol lampu Flash LED onboard, serta penyesuaian sensor OV2640 yang mudah.

---

## 🛠️ Fitur Utama

1. **Preset Model Board Instan:**
   * Tidak perlu mendefinisikan 16 baris pin GPIO kamera secara manual. Cukup pilih model: `CAM_MODEL_AI_THINKER` (default), `CAM_MODEL_M5STACK_PSRAM`, `CAM_MODEL_WROVER_KIT`, atau `CAM_MODEL_ESP_EYE`.
2. **Auto-Deteksi PSRAM & Frame Buffer:**
   * Otomatis mengalokasikan buffer ke PSRAM jika tersedia untuk mendukung resolusi tinggi (UXGA 1600x1200 / HD 1280x720 / SVGA 800x600) atau ke DRAM internal (VGA 640x480 / QVGA 320x240).
3. **Kontrol Lampu Flash LED Onboard:**
   * Mengontrol lampu blitz LED (GPIO 4) dengan fungsi `flashOn()`, `flashOff()`, dan `flashPulse(durationMs)` non-blocking.
4. **Penyesuaian Sensor OV2640:**
   * Fungsi helper instan untuk kecerahan (`setBrightness`), kontras (`setContrast`), saturasi (`setSaturation`), rotasi (`setVFlip`), dan cermin horizontal (`setHMirror`).
5. **Aman Lintas Platform (Zero Error):**
   * Otomatis menjadi *no-op* aman jika `#include <IskakINO.h>` dikompilasi di board non-ESP32 (Arduino AVR atau ESP8266).

---

## 🔌 Diagram Pinout ESP32-CAM AI-Thinker

```
                        +-------------------+
                        |   [Antenna/PCB]   |
                  GND --| 1               16|-- GPIO 12 (Flash / HS2_DATA2)
                  VCC --| 2 (5V/3.3V)     15|-- GPIO 13 (HS2_DATA3)
              GPIO 14 --| 3 (HS2_CLK)     14|-- GPIO 15 (HS2_CMD)
              GPIO 15 --| 4 (HS2_CMD)     13|-- GPIO 14 (HS2_CLK)
              GPIO 13 --| 5 (HS2_DATA3)   12|-- GPIO 2  (HS2_DATA0 / Flash)
              GPIO 12 --| 6 (HS2_DATA2)   11|-- GPIO 4  (Flashlight LED)
              GPIO 4  --| 7 (Flash LED)   10|-- GND
                  GND --| 8                9|-- 5V
                        +-------------------+
```

---

## 💻 Contoh Penggunaan Singkat

### 1. Inisialisasi & Pengambilan Foto (Snapshot)
```cpp
#include <IskakINO.h>

// Inisialisasi objek kamera (Lampu Flash pada GPIO 4)
IskakINO_Cam cam(4);

void setup() {
    Serial.begin(115200);

    // Inisialisasi modul kamera ESP32-CAM AI-Thinker resolusi VGA
    if (!cam.begin(CAM_MODEL_AI_THINKER, FRAMESIZE_VGA, 12)) {
        Serial.println("[Error] Gagal menginisialisasi kamera!");
        return;
    }

    Serial.println("[Ready] Kamera siap digunakan.");

    // Nyalakan flash sejenak dan ambil foto
    cam.flashPulse(100);
    camera_fb_t* fb = cam.capture();

    if (fb) {
        Serial.print("Foto berhasil diambil! Ukuran: ");
        Serial.print(fb->len);
        Serial.println(" bytes");

        // WAJIB: Kembalikan buffer frame setelah selesai digunakan
        cam.release(fb);
    }
}

void loop() {
    cam.update(); // Update timer flash non-blocking
}
```

### 2. Pola Framework Kernel
```cpp
#include <IskakINO.h>

IskakINO_Cam cam;
IskakINO_CamModule camMod(cam, CAM_MODEL_AI_THINKER, FRAMESIZE_QVGA);

void setup() {
    IskakINO.registerModule(&camMod);
    IskakINO.begin(); // Otomatis menginisialisasi kamera
}

void loop() {
    IskakINO.update();
}
```

---

## 📖 Referensi API Publik

### Inisialisasi & Informasi
* `bool begin(IskakCamModel model = CAM_MODEL_AI_THINKER, framesize_t frameSize = FRAMESIZE_VGA, uint8_t jpegQuality = 12, uint8_t fbCount = 1)`: Inisialisasi hardware kamera.
* `bool isInitialized() const`: Memeriksa status inisialisasi kamera.
* `bool hasPSRAM() const`: Memeriksa ketersediaan modul PSRAM eksternal.

### Pengambilan Gambar & Manajemen Buffer
* `camera_fb_t* capture()`: Mengambil 1 frame gambar JPEG. Mengembalikan pointer ke `camera_fb_t`.
* `void release(camera_fb_t* fb)`: Membebaskan buffer memori frame (wajib dipanggil setelah data foto selesai diproses).

### Kontrol Lampu Flash LED Onboard
* `void setFlashPin(int8_t pin)`: Mengatur pin GPIO lampu flash (default GPIO 4).
* `void flashOn()`: Menyalakan lampu flash secara terus menerus.
* `void flashOff()`: Mematikan lampu flash.
* `void flashPulse(uint16_t durationMs)`: Menyalakan lampu flash selama $X$ ms lalu mati otomatis tanpa blocking.
* `void update()`: Memproses timer pulse lampu flash (wajib di `loop()`).

### Penyesuaian Sensor Gambar
* `bool setBrightness(int level)`: Mengatur kecerahan (-2 s/d 2).
* `bool setContrast(int level)`: Mengatur kontras (-2 s/d 2).
* `bool setSaturation(int level)`: Mengatur saturasi warna (-2 s/d 2).
* `bool setVFlip(bool flip)`: Membalik orientasi gambar vertikal (atas-bawah).
* `bool setHMirror(bool mirror)`: Membalik orientasi cermin horizontal (kiri-kanan).
* `bool setResolution(framesize_t size)`: Mengubah resolusi kamera secara dinamis.
* `sensor_t* getSensor()`: Mengambil raw pointer driver sensor `sensor_t*` ESP-IDF.
