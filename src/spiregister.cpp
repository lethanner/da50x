#include "spiregister.h"

uint8_t _shiftreg_buffer;

void _spiSendBuffer()
{
    SOFTSPI_LT_LOW;
    for (byte i = 0; i < 8; i++)
    {
        if ((_shiftreg_buffer >> i) & 0x01)
            SOFTSPI_DT_HIGH;
        else
            SOFTSPI_DT_LOW;

        SOFTSPI_CK_HIGH;
        SOFTSPI_CK_LOW;
    }
    SOFTSPI_LT_HIGH;
}

// сброс регистра
void shifterReset()
{
    _shiftreg_buffer = 0x00;
    _spiSendBuffer();
}

// задать состояние вывода внешнего регистра
void extWrite(uint8_t pin, bool state)
{
    bitWrite(_shiftreg_buffer, pin, state);
    _spiSendBuffer();
}