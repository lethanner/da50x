/*
 * Файл для работы со сдвиговым регистром в DA50X.
 * Микросхема 74HC595, интерфейс - программно-эмулированный SPI.
 * 
 * За некоторую информацию спасибо сайту narodstream:
 * https://narodstream.ru/avr-urok-25-spi-podklyuchaem-sdvigovyj-registr-74hc595/
*/

#ifndef _spiregister_h
#define _spiregister_h

#include <Arduino.h>
#include <avr/io.h>

// выводы сдвигового регистра
#define EXT_BT_ENABLE 7
#define EXT_USB_ENABLE 6
#define EXT_INDICATOR 5
#define EXT_MON_ENABLE 2
#define EXT_AMP_STANDBY 0
#define EXT_AMP_MUTE 1

#define SOFTSPI_DT_HIGH PORTB |= _BV(PB3)
#define SOFTSPI_DT_LOW PORTB &= ~_BV(PB3)
#define SOFTSPI_LT_HIGH PORTB |= _BV(PB4)
#define SOFTSPI_LT_LOW PORTB &= ~_BV(PB4)
#define SOFTSPI_CK_HIGH PORTB |= _BV(PB5)
#define SOFTSPI_CK_LOW PORTB &= ~_BV(PB5)

void srReset();
void extWrite(uint8_t pin, bool state);

#endif