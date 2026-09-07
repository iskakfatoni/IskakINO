/*
 * 10_BasicIOShield_Overview.ino
 * Modul: IskakINO_BasicIOShield & IskakINO_ArduFast
 * (Khusus Mikrokontroler Arduino AVR: Uno, Nano, Mega, Duemilanove, dsb)
 *
 * Fitur yang didemonstrasikan (SEMUA I/O Modul EMS Basic I/O Shield):
 *   1. Peragaan Angka 7-Segment Multiplexing NON-BLOCKING (setDisplay & update)
 *   2. Pembacaan Analog Input Potensiometer (Pin A0)
 *   3. Output Analog Presisi via I2C DAC AD5612 10-Bit (0 - 1023)
 *   4. Pembacaan Digital Input (Button 1 di Pin D2 & Button 2 di Pin D4)
 *   5. Kontrol Output 4x LED (Merah D9, Kuning D8, Biru D7, Hijau D6)
 *   6. Kontrol Audio Buzzer Onboard (Pin D3) untuk Feedback & Alarm
 *   7. Deteksi Rintangan / Jarak Proximity via Infrared (TX: D5, RX: A1)
 *   8. Penjadwalan & Logging Telemetri Cepat via IskakINO_ArduFast
 *
 * CATATAN PENTING ARSITEKTUR HARDWARE (EMS Basic I/O Shield):
 * Pada board ini, Pin D6, D7, D8, D9 terhubung paralel antara LED (Hijau,
 * Biru, Kuning, Merah) dan Segmen 7-segment (Segmen A, B, C, D).
 *
 * Sketsa ini mendemonstrasikan seluruh periferal secara harmonis:
 * - MODE STANDAR (7-Segment, DAC, Potensiometer, Buzzer, & Sensor IR):
 *   Saat tidak ada tombol ditekan, 7-Segment menampilkan nilai pembacaan
 *   Potensiometer (0-99) secara jernih dan stabil, serta mengalirkan
 *   sinyal analog ke IC DAC AD5612. Sensor IR mendeteksi halangan/tangan;
 *   jika ada objek mendekat, buzzer membunyikan peringatan!
 * - MODE INTERAKTIF LED & TOMBOL (Button Press):
 *   Saat tombol ditekan, 7-Segment dinonaktifkan sementara (clearDisplay)
 *   sehingga ke-4 LED dapat diuji secara mandiri tanpa tabrakan sinyal:
 *     * Button 1 ditekan   : Uji LED Merah & Biru (RedOn & BlueOn) + Audio Beep
 *     * Button 2 ditekan   : Uji LED Hijau & Kuning (GreenOn & YellowOn) + Audio Beep
 *     * Kedua tombol ditekan: Nyalakan ke-4 LED sekaligus
 *   Saat tombol dilepas, 7-Segment otomatis aktif kembali menampilkan nilai!
 */

#include <IskakINO.h>

#if !defined(ISKAKINO_PLATFORM_AVR) && !defined(__AVR__)
  #error "Sketsa ini dirancang khusus untuk board Arduino AVR (Uno, Nano, Mega, dll)."
#endif

// Inisialisasi objek driver ArduFast dan Basic I/O Shield
IskakINO_ArduFast      fast;
IskakINO_BasicIOShield shield;

// Variabel penyimpan nilai display terakhir & status tombol sebelumnya
static uint8_t lastDisplayVal = 0;
static bool    lastBtn1State  = false;
static bool    lastBtn2State  = false;

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - Basic I/O Shield Complete  "));
    fast.log(F("========================================"));

    // Inisialisasi seluruh pin I/O, antarmuka I2C Wire, Buzzer, dan IR
    shield.begin();

    // Nada sambutan awal (Double Beep)
    shield.Beep(40);
    delay(60);
    shield.Beep(40);

    // Atur angka awal display 7-segment
    shield.setDisplay(0);

    fast.log(F("[Ready] Putar potensiometer, dekatkan tangan ke IR, atau tekan tombol!"));
    fast.log(F("[Info]  Dilepas: 7-Segment + DAC + IR | Ditekan: Pengujian 4 LED + Buzzer"));
}

void loop() {
    // ------------------------------------------------------------------------
    // 1. Baca Status Kedua Push Button (Aktif HIGH)
    // ------------------------------------------------------------------------
    bool btn1 = shield.Button1State();
    bool btn2 = shield.Button2State();

    // Feedback audio instan saat tombol baru saja ditekan (Rising Edge)
    if ((btn1 && !lastBtn1State) || (btn2 && !lastBtn2State)) {
        shield.BuzzerOn();
    } else if (!btn1 && !btn2 && (lastBtn1State || lastBtn2State)) {
        shield.BuzzerOff();
    }
    lastBtn1State = btn1;
    lastBtn2State = btn2;

    // ------------------------------------------------------------------------
    // 2. Manajemen Mode: Uji LED vs Peragaan 7-Segment
    // ------------------------------------------------------------------------
    if (btn1 || btn2) {
        // Jika salah satu tombol ditekan:
        // Matikan transistor 7-segment agar pin D6..D9 bebas untuk kendali LED
        if (shield.isDisplayEnabled()) {
            shield.clearDisplay();
        }

        // Kontrol interaktif 4 LED:
        // Button 1 -> LED Merah (D9) & Biru (D7)
        if (btn1) {
            shield.RedOn();
            shield.BlueOn();
        } else {
            shield.RedOff();
            shield.BlueOff();
        }

        // Button 2 -> LED Hijau (D6) & Kuning (D8)
        if (btn2) {
            shield.GreenOn();
            shield.YellowOn();
        } else {
            shield.GreenOff();
            shield.YellowOff();
        }
    } else {
        // Jika tombol dilepas:
        // Kembalikan pin D6..D9 ke multiplexer 7-segment
        if (!shield.isDisplayEnabled()) {
            shield.RedOff();
            shield.GreenOff();
            shield.BlueOff();
            shield.YellowOff();
            // Aktifkan kembali display 7-segment dengan nilai terakhir
            shield.setDisplay(lastDisplayVal);
        }

        // Jalankan multiplexing refresh 7-segment (Non-blocking)
        shield.update();
    }

    // ------------------------------------------------------------------------
    // 3. Baca Potensiometer, Sensor IR, DAC, & Telemetri (Tiap 50 ms)
    // ------------------------------------------------------------------------
    if (fast.every(50, 0)) {
        // Baca nilai analog potensiometer (Pin A0: 0 - 1023)
        uint16_t potVal = shield.ReadPotentiometer();

        // Kirim nilai analog ke IC DAC AD5612 (10-bit: 0 - 1023)
        shield.WriteDAC(potVal);

        // Periksa Sensor Infrared (Deteksi halangan/tangan di depan shield)
        bool irDetected = shield.IsIRObjectDetected();

        // Jika ada halangan terdeteksi dan tombol tidak ditekan -> Peringatan Buzzer
        if (irDetected && !btn1 && !btn2) {
            shield.BuzzerOn();
        } else if (!btn1 && !btn2) {
            shield.BuzzerOff();
        }

        // Konversi nilai ADC (0-1023) ke rentang 2-digit (0 - 99) untuk 7-segment
        uint8_t displayVal = map(potVal, 0, 1023, 0, 99);
        lastDisplayVal = displayVal;

        // Perbarui angka 7-segment hanya jika display sedang aktif
        if (shield.isDisplayEnabled()) {
            shield.setDisplay(displayVal);
        }

        // Cetak telemetri terformat menggunakan ArduFast tiap 200 ms (slot task 1)
        if (fast.every(200, 1)) {
            fast.logf(F("Pot: %u | DAC: %u | 7-Seg: %u | IR: %s | Mode: %s | B1: %d | B2: %d"),
                      potVal, potVal, displayVal,
                      irDetected ? "OBJECT!" : "CLEAR",
                      (btn1 || btn2) ? "LED TEST" : "7-SEGMENT",
                      btn1, btn2);
        }
    }
}
