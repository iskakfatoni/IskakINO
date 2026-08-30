/*
 * 10_BasicIOShield_Overview.ino
 * Modul: IskakINO_BasicIOShield & IskakINO_ArduFast
 * (Khusus Mikrokontroler Arduino AVR: Uno, Nano, Mega, Duemilanove, dsb)
 *
 * Fitur yang didemonstrasikan:
 *   1. Kontrol Output LED (Merah, Hijau, Biru, Kuning)
 *   2. Pembacaan Digital Input (Button 1 & Button 2)
 *   3. Pembacaan Analog Input Potensiometer (Pin A1)
 *   4. Peragaan Angka 7-Segment Multiplexing NON-BLOCKING (setDisplay & update)
 *   5. Output Analog Presisi via I2C DAC AD5612 10-Bit (0 - 1023)
 *   6. Penjadwalan & Logging Telemetri Cepat via IskakINO_ArduFast
 */

#include <IskakINO.h>

#if !defined(ISKAKINO_PLATFORM_AVR) && !defined(__AVR__)
  #error "Sketsa ini dirancang khusus untuk board Arduino AVR (Uno, Nano, Mega, dll)."
#endif

// Inisialisasi objek driver ArduFast dan Basic I/O Shield
IskakINO_ArduFast      fast;
IskakINO_BasicIOShield shield;

void setup() {
    fast.begin(115200);
    fast.log(F("========================================"));
    fast.log(F("  IskakINO - Basic I/O Shield Showcase  "));
    fast.log(F("========================================"));

    // Inisialisasi seluruh pin I/O dan antarmuka I2C Wire
    shield.begin();

    // Atur angka awal display 7-segment
    shield.setDisplay(0);

    fast.log(F("[Ready] Putar potensiometer atau tekan tombol!"));
}

void loop() {
    // ------------------------------------------------------------------------
    // 1. Refresh 7-Segment Multiplexer (WAJIB dipanggil di setiap putaran loop)
    // ------------------------------------------------------------------------
    shield.update();

    // ------------------------------------------------------------------------
    // 2. Kontrol LED berdasarkan Tombol (Push Button)
    // ------------------------------------------------------------------------
    // Jika Button 1 ditekan -> Nyalakan LED Merah & Biru
    if (shield.Button1State() == HIGH) {
        shield.RedOn();
        shield.BlueOn();
    } else {
        shield.RedOff();
        shield.BlueOff();
    }

    // Jika Button 2 ditekan -> Nyalakan LED Hijau & Kuning
    if (shield.Button2State() == HIGH) {
        shield.GreenOn();
        shield.YellowOn();
    } else {
        shield.GreenOff();
        shield.YellowOff();
    }

    // ------------------------------------------------------------------------
    // 3. Baca Potensiometer, Kirim ke DAC, & Perbarui Display (Tiap 100 ms)
    // ------------------------------------------------------------------------
    if (fast.every(100, 0)) {
        // Baca nilai analog potensiometer (0 - 1023)
        uint16_t potVal = shield.ReadPotentiometer();

        // Kirim nilai analog ke IC DAC AD5612 (10-bit, 0 - 1023)
        shield.WriteDAC(potVal);

        // Konversi nilai ADC (0-1023) ke rentang 2-digit (0 - 99) untuk 7-segment
        uint8_t displayVal = map(potVal, 0, 1023, 0, 99);
        shield.setDisplay(displayVal);

        // Cetak telemetri terformat menggunakan ArduFast
        fast.logf(F("Pot: %u | 7-Seg: %u | Btn1: %d | Btn2: %d"),
                  potVal, displayVal, shield.Button1State(), shield.Button2State());
    }
}
