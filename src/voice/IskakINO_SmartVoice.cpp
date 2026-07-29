/*
 * IskakINO_SmartVoice.cpp
 * Implementasi protokol komunikasi DFPlayer Mini.
 * Optimized for iskakfatoni (2026-02-17)
 */

#include "IskakINO_SmartVoice.h"

IskakINO_SmartVoice::IskakINO_SmartVoice() {}

void IskakINO_SmartVoice::begin(Stream& serial, uint8_t busyPin, uint16_t bootDelayMs) {
    _voiceSerial = &serial;
    _busyPin = busyPin;
    if (_busyPin != 255) {
        pinMode(_busyPin, INPUT);
    }
    // Reset modul saat awal mulai
    sendRaw(0x0C, 0);
    // Modul DFPlayer Mini butuh waktu boot fisik setelah reset/power-on.
    // Ini keterbatasan hardware (bukan blocking API); jalankan begin() sekali di setup().
    delay(bootDelayMs);
    _lastError = IskakINO_Result::OK;
    _logger.log(F("[SmartVoice] begin() selesai."));
}

void IskakINO_SmartVoice::setVolume(uint8_t volume) {
    if (!_voiceSerial) { _lastError = IskakINO_Result::NOT_CONNECTED; return; }
    if (volume > 30) volume = 30;
    _vol = volume;
    sendRaw(0x06, _vol);
    _lastError = IskakINO_Result::OK;
}

void IskakINO_SmartVoice::playTrack(uint16_t track) {
    if (!_voiceSerial) { _lastError = IskakINO_Result::NOT_CONNECTED; return; }
    if (track == 0) { _lastError = IskakINO_Result::INVALID_ARG; return; } // tidak ada file 0000.mp3
    sendRaw(0x03, track);
    _lastError = IskakINO_Result::OK;
}

void IskakINO_SmartVoice::playFromFolder(uint8_t f, uint8_t t) {
    if (!_voiceSerial) { _lastError = IskakINO_Result::NOT_CONNECTED; return; }
    if (f == 0 || f > 99 || t == 0) { _lastError = IskakINO_Result::INVALID_ARG; return; } // folder valid: 01-99, file valid: 001-255
    sendRaw(0x0F, f, t);
    _lastError = IskakINO_Result::OK;
}

void IskakINO_SmartVoice::announce(uint16_t track) {
    if (!_voiceSerial) { _lastError = IskakINO_Result::NOT_CONNECTED; return; }
    if (track == 0) { _lastError = IskakINO_Result::INVALID_ARG; return; }
    sendRaw(0x13, track);
    _lastError = IskakINO_Result::OK;
}

void IskakINO_SmartVoice::pause() {
    if (!_voiceSerial) { _lastError = IskakINO_Result::NOT_CONNECTED; return; }
    sendRaw(0x0E, 0);
    _lastError = IskakINO_Result::OK;
}

void IskakINO_SmartVoice::resume() {
    if (!_voiceSerial) { _lastError = IskakINO_Result::NOT_CONNECTED; return; }
    sendRaw(0x0D, 0);
    _lastError = IskakINO_Result::OK;
}

void IskakINO_SmartVoice::stop() {
    if (!_voiceSerial) { _lastError = IskakINO_Result::NOT_CONNECTED; return; }
    sendRaw(0x16, 0);
    _lastError = IskakINO_Result::OK;
}

bool IskakINO_SmartVoice::isSDCardReady(uint16_t timeoutMs) {
    if (!_voiceSerial) { _lastError = IskakINO_Result::NOT_CONNECTED; return false; }

    // Buang sisa data lama di buffer supaya tidak salah baca respons.
    while (_voiceSerial->available()) {
        _voiceSerial->read();
    }

    // 0x3F = query status device (SD Card / U-Disk online)
    sendRaw(0x3F, 0);

    uint8_t cmd;
    uint16_t param;
    if (!readResponse(cmd, param, timeoutMs)) {
        _lastError = IskakINO_Result::TIMEOUT;
        _logger.log(F("[SmartVoice] isSDCardReady: timeout, tidak ada respons."));
        return false; // timeout, tidak ada respons sama sekali
    }

    if (cmd == 0x40) {
        _lastError = IskakINO_Result::WRITE_FAILED; // modul membalas dengan kode error komunikasi
        _logger.log(F("[SmartVoice] isSDCardReady: modul membalas error (0x40)."));
        return false; // modul membalas dengan kode error
    }

    if (cmd == 0x3F) {
        // Bit 0 dari parameter = 1 berarti SD Card terpasang & siap (spek DFPlayer Mini)
        // _lastError == OK di sini terlepas dari hasil bit0 — query-nya SENDIRI sukses,
        // status "SD belum siap" itu jawaban valid, bukan kegagalan operasi.
        _lastError = IskakINO_Result::OK;
        return (param & 0x01) != 0;
    }

    _lastError = IskakINO_Result::OK;
    return false;
}

bool IskakINO_SmartVoice::isPlaying() {
    if (_busyPin == 255) return false; // belum dikonfigurasi lewat begin()
    return isPlaying(_busyPin);
}

bool IskakINO_SmartVoice::isPlaying(uint8_t busyPin) {
    // Pin BUSY DFPlayer akan bernilai LOW saat memutar suara
    return (digitalRead(busyPin) == LOW);
}

// --- PRIVATE: Protokol Hex DFPlayer ---
void IskakINO_SmartVoice::sendRaw(uint8_t cmd, uint16_t arg) {
    sendRaw(cmd, (uint8_t)(arg >> 8), (uint8_t)(arg & 0xFF));
}

void IskakINO_SmartVoice::sendRaw(uint8_t cmd, uint8_t high, uint8_t low) {
    if (!_voiceSerial) return;

    uint8_t packet[10];
    packet[0] = 0x7E;          // Start Byte
    packet[1] = 0xFF;          // Version
    packet[2] = 0x06;          // Data Length (fixed)
    packet[3] = cmd;           // Command
    packet[4] = 0x00;          // Feedback (0x00 = No, 0x01 = Yes)
    packet[5] = high;          // Parameter High Byte
    packet[6] = low;           // Parameter Low Byte

    // Checksum Calculation
    uint16_t checksum = -(packet[1] + packet[2] + packet[3] + packet[4] + packet[5] + packet[6]);
    packet[7] = (uint8_t)(checksum >> 8);
    packet[8] = (uint8_t)(checksum & 0xFF);
    packet[9] = 0xEF;          // End Byte

    _voiceSerial->write(packet, 10);
}

bool IskakINO_SmartVoice::readResponse(uint8_t &cmdOut, uint16_t &paramOut, uint16_t timeoutMs) {
    if (!_voiceSerial) return false;

    uint8_t frame[10];
    uint8_t idx = 0;
    unsigned long start = millis();

    while ((millis() - start) < timeoutMs) {
        if (_voiceSerial->available()) {
            uint8_t b = _voiceSerial->read();

            if (idx == 0 && b != 0x7E) continue; // sinkronisasi ke Start Byte
            frame[idx++] = b;

            if (idx >= 10) {
                uint16_t recvChecksum = ((uint16_t)frame[7] << 8) | frame[8];
                uint16_t calcChecksum = -(frame[1] + frame[2] + frame[3] + frame[4] + frame[5] + frame[6]);

                if (frame[0] == 0x7E && frame[9] == 0xEF && recvChecksum == calcChecksum) {
                    cmdOut = frame[3];
                    paramOut = ((uint16_t)frame[5] << 8) | frame[6];
                    return true;
                }
                idx = 0; // frame tidak valid (termasuk checksum salah), coba sinkronisasi ulang
            }
        }
    }
    return false; // timeout
}
