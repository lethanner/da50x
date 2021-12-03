#include <Arduino.h>
#include "GyverTimers.h"
#include "config.h"
#include "defines.h"
#include "spiregister.h"

#define BT_TX_HIGH PORTD |= _BV(PD4)
#define BT_TX_LOW PORTD &= ~_BV(PD4)

char bt_rx_buffer[16];
int rx_char;
byte rx_buffer_pos = 0, rx_buff_c_pos = 0;
// bool replyReady = false;
extern volatile unsigned long timer0_millis;

void _bt_writeChar(char c)
{
    uint8_t oldSREG = SREG;
    cli();
    BT_TX_LOW;
    for (byte i = 0; i < 8; i++)
    {
        delayMicroseconds(102);
        if ((c >> i) & 0x01)
            BT_TX_HIGH;
        else
            BT_TX_LOW;
    }
    delayMicroseconds(101);
    BT_TX_HIGH;
    SREG = oldSREG;
    delayMicroseconds(101);
}

void _bt_printLine(const char s[])
{
    byte pos = 0;
    while (s[pos] != '\0')
    {
        _bt_writeChar(s[pos]);
        pos++;
    }
}

bool _bt_signatureCheck(const char* signature) {
    unsigned long timeout = timer0_millis;
    bool flag = false;
    while (timer0_millis - timeout < CHIP_REPLY_TIMEOUT_MS)
    {
        if (strcmp(bt_rx_buffer, signature) == 0)
        {
            flag = true;
            break;
        }
    }
    rx_buffer_pos = 0;
    return flag;
}

bool bt_sendAT(const char* cmd) {
    _bt_printLine("AT+");
    _bt_printLine(cmd);
    _bt_writeChar('\r');
    _bt_writeChar('\n');
    
    return _bt_signatureCheck("OK");
}

bool bt_activate()
{
    DDRB &= ~_BV(PB0);
    PORTB |= _BV(PB0);

    DDRD |= _BV(PD4);
    BT_TX_HIGH;         // установить высокий уровень на TX
    PCMSK2 |= _BV(PD5); // активировать пин D5 на прерывании PCINT2
    PCICR |= (1 << 2);  // активировать PCINT2

    Timer1.setPeriod(SOFTUART_BIT_RX_US);
    Timer1.stop();
    Timer1.enableISR();

    extWrite(EXT_BT_ENABLE, false);
    delay(100);
    extWrite(EXT_BT_ENABLE, true);

    if (!_bt_signatureCheck("ON")) return false;
    //Serial.println("INIT");
    //return bt_sendAT("CP");
    return true;
}

ISR(TIMER1_A)
{
    if ((PIND >> 5) & 0x01)
        rx_char |= (1 << rx_buff_c_pos);
    else
        rx_char &= ~(1 << rx_buff_c_pos);

    rx_buff_c_pos++;
    if (rx_buff_c_pos > 8)
    {
        Timer1.stop();
        PCMSK2 |= _BV(PD5);

        // проверка наличия стоп-бита
        if (!((rx_char >> 8) & 0x01))
            return;

        rx_char &= 0xFF;
        if (rx_buffer_pos == 0 && (rx_char < 65 || rx_char > 90))
            return;

        //Serial.print(rx_buffer_pos);
        //Serial.write(rx_char);
        if (rx_char == '\n')
        {
            bt_rx_buffer[rx_buffer_pos - 1] = '\0';
            //Serial.println(bt_rx_buffer);
            return;
            //replyReady = true;
        }
        bt_rx_buffer[rx_buffer_pos] = rx_char;
        rx_buffer_pos++;
    }
}

ISR(PCINT2_vect)
{
    if (!((PIND >> 5) & 0x01))
    {
        PCMSK2 &= ~_BV(PD5);
        rx_buff_c_pos = 0;
        if (rx_buffer_pos > 14)
            return;
        delayMicroseconds(8);
        Timer1.restart();
    }
}