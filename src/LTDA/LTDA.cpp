#include "LTDA.h"

byte volMaster;
byte curInput = 0;
bool statusRefresh;
bool ampEnabled;
int8_t hsTemp, _hsTemp;
uint16_t inputVoltage, outLevel;

MicroDS18B20 heatsink(A3);
uint32_t hsRefreshTimer;
extern volatile unsigned long timer0_millis;

/* Микшер */
// установка значения громкости мастер-канала в пределах 0-255
void setMasterVolume(byte val)
{
    Wire.beginTransmission(DIGIPOT_MASTER_I2C);
    Wire.write(0b10101111); // команда на запись обеих потенциометров
    Wire.write(val);

    statusRefresh = true;
    if (Wire.endTransmission() != 0)
        terminate(1);
}

// установка значения громкости мастер-канала в пределах 0-100%
void setMasterVolumeClassic(byte vol)
{
    volMaster = vol;
    vol = map(vol, 0, 100, 0, 255);
    setMasterVolume(vol);
}

void changeVolume(bool dir)
{
    uint8_t newVol;
    if (!dir && volMaster > 0)
        newVol = volMaster - 1;
    if (dir && volMaster < 100)
        newVol = volMaster + 1;

    setMasterVolumeClassic(newVol);
}

// управление питанием усилителя
void setAmplifier(bool state)
{
    // Таков порядок подачи сигналов на микросхему усилителя.
    // Это обеспечит отсутствие щелчка при включении/выключении.
    // Даташит: https://www.st.com/resource/en/datasheet/tda7297.pdf

    extWrite(state ? EXT_AMP_STANDBY : EXT_AMP_MUTE, state);
    delay(150);
    extWrite(state ? EXT_AMP_MUTE : EXT_AMP_STANDBY, state);

    ampEnabled = state;
    statusRefresh = true;
}

// проверка, доступен ли запрошенный источник
bool checkInputAvailability(byte src_id)
{
    switch (src_id)
    {
    case SRC_USB:
        if (!((PIND >> 2) & 0x01))
            return false;
        break;
    }

    return true;
}

// переключение источника аудио
void changeAudioInput(byte src_id)
{
    // выключение предыдущего источника
    switch (curInput)
    {
    case SRC_USB:
        extWrite(EXT_USB_ENABLE, false);
        break;
    case SRC_BT:
        bt_disable();
        break;
    }

    // включение запрошенного источника
    switch (src_id)
    {
    case SRC_USB:
        extWrite(EXT_USB_ENABLE, true);
        break;
    case SRC_BT:
        bt_restart();
        break;
    }

    curInput = src_id;
    statusRefresh = true;
}

void deviceStatusRefresh()
{
    /* Обновление датчика температуры */
    if (timer0_millis - hsRefreshTimer > TEMP_REFRESH_INTERVAL_MS) {
        hsTemp = heatsink.getTemp();
        if (hsTemp != _hsTemp)
        {
            _hsTemp = hsTemp;
            statusRefresh = true;
        }
        hsRefreshTimer = timer0_millis;
        heatsink.requestTemp();
    }

    /* Обновление данных с АЦП, если преобразование окончено */
    if (bit_is_clear(ADCSRA, ADSC))
    {
        uint16_t adcData = ADC;
        if (ADMUX & 0x01) // канал A7 (аудио)
        {
            ADMUX = 0b11100110; // переключаемся на A6
            outLevel = adcData;
        }
        else // канал А6 (напряжение)
        {
            ADMUX = 0b11100111; // переключаемся на A7
            inputVoltage = adcData * 18;
        }
        bitSet(ADCSRA, ADSC);
    }

    if (curInput == SRC_BT)
        bt_update();
}