#include "spiregister.h"
#include "defines.h"

void setAmplifier(bool state) {
    extWrite(state ? EXT_AMP_STANDBY : EXT_AMP_MUTE, state);
    delay(150);
    extWrite(state ? EXT_AMP_MUTE : EXT_AMP_STANDBY, state);
}