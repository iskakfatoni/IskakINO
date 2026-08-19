# 📈 Modul: IskakINO_Filter

Kumpulan algoritma pemrosesan sinyal digital dan kalibrasi sensor analog/jarak berkecepatan tinggi, hemat memori (*zero dynamic allocation*), dan dirancang khusus untuk mikrokontroler.

---

## 🛠️ Komponen Modul

| Class | Jenis Algoritma | Penggunaan Utama |
|---|---|---|
| **`IskakINO_Kalman1D`** | 1D Kalman Filter | Sensor berisik/bising kontinu: Ultrasonik HC-SR04, ToF VL53L0X, Load Cell / Jembatan Wheatstone, Sensor Arus. |
| **`IskakINO_MedianFilter<N>`** | Moving Median Filter | Mengeliminasi data *spike* lonjakan liar ekstrem yang tiba-tiba meleset jauh dari tren. |
| **`IskakINO_EMAFilter`** | Exponential Moving Average | Penghalusan sinyal analog ringan super cepat (potensiometer, sensor cahaya LDR, suhu LM35). |
| **`IskakINO_LinearCalibrator`** | Multi-Point Linear Calibration | Mengonversi tegangan/ADC mentah ke satuan fisik terkalibrasi (pH meter, TDS air, termistor NTC). |

---

## 💻 Contoh Penggunaan Singkat

### 1. 1D Kalman Filter (Menghaluskan Data Sensor Berisik)
```cpp
#include <IskakINO.h>

// Parameter: process noise (Q = 0.125), measurement noise (R = 4.0)
IskakINO_Kalman1D kalman(0.125f, 4.0f);

void setup() {
    Serial.begin(115200);
}

void loop() {
    float rawSensor = analogRead(A0);
    float cleanValue = kalman.update(rawSensor);

    Serial.print("Raw:"); Serial.print(rawSensor);
    Serial.print(" Filtered:"); Serial.println(cleanValue);
    delay(50);
}
```

### 2. Moving Median Filter (Menghilangkan Spike/Outlier)
```cpp
#include <IskakINO.h>

// Filter median dengan ukuran window 5 sampel (N harus ganjil)
IskakINO_MedianFilter<5> medianFilter;

void loop() {
    float rawDistance = readUltrasonicCM();
    float stableDistance = medianFilter.update(rawDistance);
    
    Serial.println(stableDistance);
}
```

### 3. Kalibrasi Multi-Titik Linear (Konversi Nilai ADC ke Satuan Fisik)
```cpp
#include <IskakINO.h>

IskakINO_LinearCalibrator phSensor;

void setup() {
    // Daftarkan titik acuan: addPoint(raw_adc, ph_aktual)
    phSensor.addPoint(145.0f, 4.01f); // Titik buffer asam pH 4
    phSensor.addPoint(220.0f, 7.00f); // Titik buffer netral pH 7
    phSensor.addPoint(310.0f, 10.01f); // Titik buffer basa pH 10
}

void loop() {
    float rawADC = analogRead(A0);
    float actualPH = phSensor.calibrate(rawADC);

    Serial.print("pH Terkalibrasi: ");
    Serial.println(actualPH, 2);
    delay(500);
}
```

---

## 📖 Referensi API Publik

### `IskakINO_Kalman1D`
* `float update(float measurement)`: Memperbarui perhitungan matriks kovariansi dan mengembalikan estimasi sinyal bersih.
* `void setParameters(float q, float r)`: Menyesuaikan parameter $Q$ (*Process Noise*) dan $R$ (*Measurement Noise*).
* `void reset(float initialValue = 0.0f)`: Me-reset estimasi ke nilai awal.
* `float getEstimate() const`: Mengambil estimasi nilai saat ini.

### `IskakINO_MedianFilter<N>`
* `float update(float val)`: Memasukkan data baru ke *circular buffer*, mengurutkan elemen di stack, dan mengembalikan nilai median (tengah).
* `void reset()`: Membersihkan buffer sampel.

### `IskakINO_EMAFilter`
* `float update(float val)`: Menghitung rata-rata bergerak eksponensial.
* `void setAlpha(float alpha)`: Mengatur rasio bobot data baru (0.0f = sangat lambat/halus, 1.0f = instan).
* `float get() const`: Mengambil nilai terfilter saat ini.

### `IskakINO_LinearCalibrator`
* `bool addPoint(float rawInput, float calibratedOutput)`: Menambahkan pasangan titik kalibrasi (maksimal 8 titik per kalibrator).
* `float calibrate(float raw) const`: Menghitung nilai terkalibrasi via interpolasi / ekstrapolasi linear antar segmen.
* `void clear()`: Menghapus seluruh titik kalibrasi yang tersimpan.
