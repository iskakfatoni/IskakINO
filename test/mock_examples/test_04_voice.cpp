#include "ArduinoExtra.h"
#include "../../examples/04_SmartVoice_PlayTrack/04_SmartVoice_PlayTrack.ino"
int main() {
    VOICE_SERIAL._autoReplyCmd = 0x3F;
    VOICE_SERIAL._autoReplyParam = 0x0003; // SD siap
    setup();
    for (int i = 0; i < 10; i++) loop();
    printf("OK: 04_SmartVoice_PlayTrack jalan tanpa crash\n");
    return 0;
}
