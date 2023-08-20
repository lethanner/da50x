#ifndef _bluetooth_h
#define _bluetooth_h

#include <Arduino.h>
#include "GyverTimers.h"
#include "config.h"
#include "spiregister.h"
#include "errorHandler.h"
#include "UART/RXBuffer.h"

#define BT_TX_HIGH PORTD |= _BV(PD4)
#define BT_TX_LOW PORTD &= ~_BV(PD4)

extern bool bt_pairing_mode;
extern bool bt_playback_state;
extern bool bt_spp_pending;
extern byte bt_conn_count;

void bt_sendAT(const char *cmd, bool check = true);
void bt_sendSPP(const char* data);
void bt_restart();
void bt_disable();
void bt_gotoPairingMode();
void bt_update();

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