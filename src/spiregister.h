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

#define SOFTSPI_DT_HIGH PORTB |= _BV(PB3)
#define SOFTSPI_DT_LOW PORTB &= ~_BV(PB3)
#define SOFTSPI_LT_HIGH PORTB |= _BV(PB4)
#define SOFTSPI_LT_LOW PORTB &= ~_BV(PB4)
#define SOFTSPI_CK_HIGH PORTB |= _BV(PB5)
#define SOFTSPI_CK_LOW PORTB &= ~_BV(PB5)

void shifterReset();
void extWrite(uint8_t pin, bool state);

#endif