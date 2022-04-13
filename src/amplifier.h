#include "spiregister.h"
#include "defines.h"

// Да, таков порядок подачи сигналов на микросхему усилителя.
// Это обеспечит отсутствие щелчка при включении/выключении.
// Даташит: https://www.st.com/resource/en/datasheet/tda7297.pdf

void setAmplifier(bool state) {
    extWrite(state ? EXT_AMP_STANDBY : EXT_AMP_MUTE, state);
    delay(150);
    extWrite(state ? EXT_AMP_MUTE : EXT_AMP_STANDBY, state);
}