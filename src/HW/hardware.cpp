#include "hardware.h"

byte currentMasterVolume;
int8_t balance = 0;
byte currentInput = 0;
byte statusRefresh;
byte undervoltage = 0;
bool ampEnabled;
int8_t hsTemp, _hsTemp;
uint16_t inputVoltageADC;
int deviceSettings = 0b00001001; // defaults так сказать

MicroDS18B20 heatsink(A3);
uint32_t hsRefreshTimer;
extern volatile unsigned long timer0_millis;

bool dacOnlyEnabledEarly;
uint32_t srcCheckTimer;
byte srcStateCache;

/* 
 * А может ну его нафиг, этот режим точной подстройки громкости?
 * Он вообще пригодится когда-нибудь?
 * Может, пора бы уже убрать эти хвосты бесполезного кода под него?
 * 
 * UPD 25.05.2023 - убрал.
*/

// установка значения громкости мастер-канала в пределах 0-100%
/* 
 * В кой-то веки добавил я этот стереобаланс.
 * Правда вот громкость канала, в сторону которого будет подкручиваться баланс,
 * никоим образом не изменяется. Разве что уменьшается громкость противоположного.
 * По-хорошему к подкручиваемому каналу должно добавляться до +6 дБ, как это
 * делает Stereo Balance DSP в моём любимом foobar2000, но пока это затруднительно.
 * 
 * А всё потому, что внутренние значения громкости в устройстве измеряются в попугаях,
 * а не в децибелах.
 * Калибровочную таблицу бы сделать.
*/
void setMasterVolume(byte vol)
{
    byte levelL, levelR;
    byte val = (bitRead(deviceSettings, DAC_ONLY_MODE)) ? 0 : map(vol, 0, 100, 0, 255);
    if (balance == 0)
        levelL = levelR = val;
    else
    {
        levelL = (balance > 0) ? val : map(balance, -50, 0, 0, val);
        levelR = (balance < 0) ? val : map(balance, 0, 50, val, 0);  
        // Возможна путаница левого и правого канала, т.к. при реализации
        // своей схемы я был вынужден свапнуть их в некоторых участках. 
    }
    
    Wire.beginTransmission(DIGIPOT_MASTER_I2C);
    Wire.write(0b10101001); // команда на отдельную запись потенциометров, начиная с нулевого
    Wire.write(levelL);
    Wire.write(levelR);
    if (Wire.endTransmission() != 0)
        terminate(1);
        
    currentMasterVolume = vol;
    statusRefresh = 1;
}

void changeVolume(bool dir, bool quick)
{
    int8_t newVol;
    if (!dir)
    {
        newVol = (quick) ? (currentMasterVolume - 5) : (currentMasterVolume - 1);
        if (newVol < 0)
            newVol = 0;
    }
    else
    {
        newVol = (quick) ? (currentMasterVolume + 5) : (currentMasterVolume + 1);
        if (newVol > 100)
            newVol = 100;
    }
    
    setMasterVolume((uint8_t)newVol);
}

void setStereoBalance(int8_t val)
{
    if (val < -50)
        balance = -50;
    else if (val > 50)
        balance = 50;
    else
        balance = val;
    
    setMasterVolume(currentMasterVolume);
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
    switch (currentInput)
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

    currentInput = src_id;
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
    if (bit_is_clear(ADCSRA, ADSC))
    {
        inputVoltageADC = ADC;
        bitSet(ADCSRA, ADSC);
    }

    /* Обновление данных с Bluetooth, если это необходимо */
    if (currentInput == SRC_BT)
        bt_update();

    /* Проверка состояний источников для автопереключения */
    // источникОВ, ага. Тут в автосвитч умеет только USB. Так же, как и checkInputAvailability проверяет только USB.
    // А флешки и карты памяти вряд ли вообще нужны в этом девайсе. Там, может, AUX появится, и вот он уже нужен.
    if (timer0_millis - srcCheckTimer > 500)
    {
        bool usb_state = PINB & 0x01;
        if (usb_state != bitRead(srcStateCache, 0))
        {
            statusRefresh = 1;
            if (usb_state && bitRead(deviceSettings, ALLOW_AUTOSWITCH))
                changeAudioInput(SRC_USB);
            else if (!usb_state && currentInput == SRC_USB)
                changeAudioInput(SRC_NULL);
            bitWrite(srcStateCache, 0, usb_state);
        }
        srcCheckTimer = timer0_millis;
    }

    /* Флаг о заниженном напряжении питания + автопереход в DAC-only mode при его отсутствии */
    // чёт какой-то некрасивый код получается
    if (inputVoltageADC > INSUFF_VOLTAGE_ADC)
    {
        if (undervoltage == 2)
        {
            undervoltage = 0;
            setDACOnlyMode(dacOnlyEnabledEarly);
        }
        if (inputVoltageADC < UNDERVOLTAGE_ADC)
        {
            if (undervoltage != 1)
            {
                undervoltage = 1;
                if (statusRefresh != 3)
                    statusRefresh = 1;
            }
        }
        else if (undervoltage != 0)
        {
            undervoltage = 0;
            statusRefresh = 1;
        }
    }
    else if (undervoltage != 2)
    {
        dacOnlyEnabledEarly = bitRead(deviceSettings, DAC_ONLY_MODE);
        setDACOnlyMode();
        undervoltage = 2;
    }
}

void setMonitoring(bool state)
{
    extWrite(EXT_MON_ENABLE, state);
    bitWrite(deviceSettings, ENABLE_MONITORING, state);
}

void setDACOnlyMode(bool state) {
    if (state == bitRead(deviceSettings, DAC_ONLY_MODE) || (undervoltage == 2 && !state))
        return;

    bitWrite(deviceSettings, DAC_ONLY_MODE, state);
    if (!state)
    {
        if (currentInput != SRC_NULL)
            setAmplifier(true);
        setMasterVolume(INIT_VOLUME);
    }
    else
    {
        setAmplifier(false);
        setMasterVolume(0);
    }
    statusRefresh = 3;
}

uint16_t readDeviceVcc()
{
    uint16_t vcc = ((uint32_t)inputVoltageADC * READ_VCC_CALIBRATION) / READ_VCC_DIVIDER;
    return (inputVoltageADC > VOLTAGE_ADC_CUTOFF) ? vcc : 0;
}