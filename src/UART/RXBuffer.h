#ifndef _rxbuffer_h
#define _rxbuffer_h

#include <Arduino.h>
// динамическое выделение памяти в хипе? Не хочу.
#define RX_BUFFER_LENGTH 32

class RXBuffer
{
public:
    void write(char ch);
    void erasePacket();
    char data[RX_BUFFER_LENGTH];
    volatile bool packet_ready;
    volatile bool packet_start;
    void flush();

    RXBuffer() { flush(); }

private:
    byte buffer_pos;
};

#endif