#ifndef _rxbuffer_h
#define _rxbuffer_h

#include <Arduino.h>
// динамическое выделение памяти в хипе? Не хочу.
#define RX_BUFFER_LENGTH 32

class RXBuffer
{
public:
    RXBuffer()
    {
        packet_start = true;
        packet_ready = false;
        buffer_pos = 0;
        memset(data, 0, RX_BUFFER_LENGTH);
    }

    void write(char ch);
    void erasePacket();
    char data[RX_BUFFER_LENGTH];
    volatile bool packet_ready;
    volatile bool packet_start;

private:
    byte buffer_pos;
};

#endif