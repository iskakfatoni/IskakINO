// Smoke-test fungsional IskakINO_FastNTP — mensimulasikan siklus state
// machine penuh (SEND_REQUEST -> AWAIT_RESPONSE) via mock UDP, termasuk
// sync sukses, timeout, dan Kiss-of-Death (stratum==0).
#include "ArduinoExtra.h"
#include "Udp.h"
#include "../../src/ntp/IskakINO_FastNTP.h"
#include <cassert>
#include <cstdio>

extern unsigned long _mock_millis_value;

// 1 Jan 2024 00:00:00 UTC dalam epoch NTP (detik sejak 1900) untuk uji.
static const uint32_t SECS_1900_TO_2024_01_01 = 3913056000UL;

int syncCount = 0;
void onSyncHandler(uint32_t) { syncCount++; }
int failCount = 0;
uint8_t lastConsecutiveFails = 0;
void onFailHandler(uint8_t n) { failCount++; lastConsecutiveFails = n; }

int main() {
    UDP udp;
    IskakINO_FastNTP ntp(udp, "pool.ntp.org");
    ntp.onSync(onSyncHandler);
    ntp.onSyncFail(onFailHandler);
    ntp.setDebug(true);
    ntp.setSyncInterval(3600000); // 1 jam
    ntp.setRequestTimeout(2000);

    _mock_millis_value = 0;
    ntp.begin(25200, 0); // GMT+7, tanpa DST
    assert(!ntp.isTimeSet());

    // --- Siklus 1: STATE_IDLE -> STATE_SEND_REQUEST (karena _lastSyncMs==0) ---
    ntp.update(); // IDLE->SEND_REQUEST
    ntp.update(); // SEND_REQUEST: kirim paket, pindah ke AWAIT_RESPONSE
    assert(udp._beginPacketCalls == 1);

    // Belum ada balasan -> tetap AWAIT_RESPONSE, tidak time out (baru 100ms)
    _mock_millis_value = 100;
    ntp.update();
    assert(!ntp.isTimeSet());

    // Sekarang "server" membalas dengan waktu valid
    udp.queueNtpResponse(SECS_1900_TO_2024_01_01, 1 /* stratum normal */);
    _mock_millis_value = 150;
    ntp.update();
    assert(ntp.isTimeSet());
    assert(syncCount == 1);
    assert(ntp.getYear() == 2024);
    assert(ntp.getMonth() == 1);
    assert(ntp.getDay() == 1);
    // GMT+7 -> jam lokal harus 07:00:00
    assert(ntp.getHours() == 7);
    assert(ntp.getMinutes() == 0);
    assert(ntp.getSeconds() == 0);
    printf("Waktu lokal: %s %s\n", ntp.getFormattedTime().c_str(), ntp.getFormattedDate().c_str());

    // --- Uji alarm ---
    bool fired = false;
    assert(ntp.isAlarmActive(7, 0, 0, fired) == true);
    assert(fired == true);
    assert(ntp.isAlarmActive(7, 0, 0, fired) == false); // sudah fired, tidak retrigger

    // --- Uji timeout: paksa siklus baru, jangan kirim balasan ---
    ntp.forceUpdate();
    ntp.update(); // SEND_REQUEST -> AWAIT_RESPONSE
    _mock_millis_value += 2500; // lewati requestTimeout (2000ms)
    ntp.update(); // harus terdeteksi timeout -> onFailHandler terpanggil
    assert(failCount == 1);
    assert(lastConsecutiveFails == 1);

    // --- Uji Kiss-of-Death: paksa siklus baru, balas dgn stratum==0 ---
    ntp.forceUpdate();
    ntp.update();
    udp.queueNtpResponse(SECS_1900_TO_2024_01_01, 0 /* Kiss-of-Death */);
    _mock_millis_value += 10;
    ntp.update();
    assert(failCount == 2); // KoD juga memanggil onFailCb

    printf("OK: semua smoke-test fungsional FastNTP lolos\n");
    return 0;
}
