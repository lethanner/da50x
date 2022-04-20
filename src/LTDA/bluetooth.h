#ifndef _bluetooth_h
#define _bluetooth_h

#include <Arduino.h>
#include "GyverTimers.h"
#include "config.h"
#include "defines.h"
#include "spiregister.h"

#define BT_TX_HIGH PORTD |= _BV(PD4)
#define BT_TX_LOW PORTD &= ~_BV(PD4)

// extern int rx_char;
// extern char bt_rx_buffer[16];
// extern byte rx_buffer_pos, rx_buff_c_pos;
extern bool bt_pairing_mode;
extern bool bt_playback_state;
extern byte bt_conn_count;

bool bt_sendAT(const char *cmd, bool check = true);
bool bt_restart();
bool bt_disable();
bool bt_gotoPairingMode();
bool bt_startSPP();
bool bt_update();

inline void bt_uart_initialize()
{
    DDRB &= ~_BV(PB0);
    PORTB |= _BV(PB0);

    DDRD |= _BV(PD4);
    BT_TX_HIGH;         // установить высокий уровень на TX
    PCMSK2 |= _BV(PD5); // активировать пин D5 на прерывании PCINT2
    PCICR |= (1 << 2);  // активировать PCINT2

    Timer1.setPeriod(104);
    Timer1.stop();
    Timer1.enableISR();
}

#endif