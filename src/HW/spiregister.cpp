#include "spiregister.h"

uint8_t _sr_buffer;

void _spiSendBuffer()
{
    SOFTSPI_LT_LOW;
    for (byte i = 0; i < 8; i++)
    {
        if ((_sr_buffer >> i) & 0x01)
            SOFTSPI_DT_HIGH;
        else
            SOFTSPI_DT_LOW;

        SOFTSPI_CK_HIGH;
        SOFTSPI_CK_LOW;
    }
    SOFTSPI_LT_HIGH;
}

// сброс регистра
void srReset()
{
    _sr_buffer = 0x00;
    _spiSendBuffer();
}

// задать состояние вывода внешнего регистра
void extWrite(uint8_t pin, bool state)
{
    bitWrite(_sr_buffer, pin, state);
    _spiSendBuffer();
}