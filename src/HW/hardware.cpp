#include "hardware.h"

byte volMaster;
byte curInput = 0;
byte statusRefresh;
bool ampEnabled;
int8_t hsTemp, _hsTemp;
uint16_t inputVoltageADC;
int deviceSettings = 0b00001001; // defaults так сказать

MicroDS18B20 heatsink(A3);
uint32_t hsRefreshTimer;
extern volatile unsigned long timer0_millis;

uint32_t srcCheckTimer;
byte srcStateCache;

/* Микшер */
// установка значения громкости мастер-канала в пределах 0-255
void setMasterVolume(byte val)
{
    Wire.beginTransmission(DIGIPOT_MASTER_I2C);
    Wire.write(0b10101111); // команда на запись обеих потенциометров
    Wire.write(val);

    statusRefresh = 2;
    if (Wire.endTransmission() != 0)
        terminate(1);
}

/* 
 * А может ну его нафиг, этот режим точной подстройки громкости?
 * Он вообще пригодится когда-нибудь?
 * Может, пора бы уже убрать эти хвосты бесполезного кода под него?
*/

// установка значения громкости мастер-канала в пределах 0-100%
void setMasterVolumeClassic(byte vol)
{
    volMaster = vol;
    vol = map(vol, 0, 100, 0, 255);
    setMasterVolume(vol);
}

void changeVolume(bool dir, bool quick)
{
    int8_t newVol;
    if (!dir)
    {
        newVol = (quick) ? (volMaster - 5) : (volMaster - 1);
        if (newVol < 0)
            newVol = 0;
    }
    else
    {
        newVol = (quick) ? (volMaster + 5) : (volMaster + 1);
        if (newVol > 100)
            newVol = 100;
    }
    
    setMasterVolumeClassic((uint8_t)newVol);
}

// управление питанием усилителя
void setAmplifier(bool state)
{
    if (ampEnabled == state || (bitRead(deviceSettings, DAC_ONLY_MODE) && state))
        return;
        
    // Таков порядок подачи сигналов на микросхему усилителя.
    // Это обеспечит отсутствие щелчка при включении/выключении.
    // Даташит: https://www.st.com/resource/en/datasheet/tda7297.pdf

    if (!state)
        extWrite(EXT_MON_ENABLE, false);
    else if (bitRead(deviceSettings, ENABLE_MONITORING))
        extWrite(EXT_MON_ENABLE, true);

    extWrite(state ? EXT_AMP_STANDBY : EXT_AMP_MUTE, state);
    delay(150);
    extWrite(state ? EXT_AMP_MUTE : EXT_AMP_STANDBY, state);

    ampEnabled = state;
    statusRefresh = 1;
}

// проверка, доступен ли запрошенный источник
bool checkInputAvailability(byte src_id)
{
    switch (src_id)
    {
    case SRC_USB:
        if (!(PINB & 0x01)) // D8
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

    // гасим усилок, когда не используем звук. Нафиг его впустую жарить вместе с током от БП/аккумулятора?
    if (src_id == SRC_NULL)
        setAmplifier(false);
    else
        setAmplifier(true);

    curInput = src_id;
    statusRefresh = 3;
}

void hardware_tick()
{
    /* Обновление датчика температуры */
    if (timer0_millis - hsRefreshTimer > TEMP_REFRESH_INTERVAL_MS) {
        hsTemp = heatsink.getTemp();
        if (hsTemp != _hsTemp)
        {
            _hsTemp = hsTemp;
            statusRefresh = 1;
        }
        hsRefreshTimer = timer0_millis;
        heatsink.requestTemp();
    }

    /* Обновление данных с АЦП, если преобразование окончено */
    // if (bit_is_clear(ADCSRA, ADSC))
    // {
    //     uint16_t adcData = ADC;
    //     if (ADMUX & 0x01) // канал A6 (напряжение)
    //     {
    //         outLevel = adcData;
    //         ADMUX = 0b01000111; // переключаемся на A7
    //     }
    //     else // канал А7 (аудио)
    //     {
    //         inputVoltage = adcData * 18;
    //         ADMUX = 0b01000110; // переключаемся на A6
    //     }
    //     bitSet(ADCSRA, ADSC);
    // }
    if (bit_is_clear(ADCSRA, ADSC))
    {
        inputVoltageADC = ADC;
        bitSet(ADCSRA, ADSC);
    }

    /* Обновление данных с Bluetooth, если это необходимо */
    if (curInput == SRC_BT)
        bt_update();

    /* Проверка состояний источников для автопереключения */
    if (timer0_millis - srcCheckTimer > 500)
    {
        bool usb_state = PINB & 0x01;
        if (usb_state != bitRead(srcStateCache, 0))
        {
            if (usb_state && bitRead(deviceSettings, ALLOW_AUTOSWITCH))
                changeAudioInput(SRC_USB);
            else if (!usb_state && curInput == SRC_USB)
                changeAudioInput(SRC_NULL);
            bitWrite(srcStateCache, 0, usb_state);
        }
        srcCheckTimer = timer0_millis;
    }
}

void setMonitoring(bool state)
{
    extWrite(EXT_MON_ENABLE, state);
    bitWrite(deviceSettings, ENABLE_MONITORING, state);
}

void toggleDACOnlyMode() {
    bool oldState = bitRead(deviceSettings, DAC_ONLY_MODE);
    bitWrite(deviceSettings, DAC_ONLY_MODE, !oldState);
    if (oldState)
    {
        if (curInput != SRC_NULL)
            setAmplifier(true);
        setMasterVolumeClassic(INIT_VOLUME);
    }
    else
    {
        setAmplifier(false);
        setMasterVolume(0);
    }
    statusRefresh = 1;
}

uint16_t readDeviceVcc()
{
    uint16_t vcc = ((uint32_t)inputVoltageADC * READ_VCC_CALIBRATION) / READ_VCC_DIVIDER;
    return (inputVoltageADC > VOLTAGE_ADC_CUTOFF) ? vcc : 0;
}