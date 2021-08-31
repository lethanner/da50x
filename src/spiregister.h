/*
 * Файл для работы со сдвиговым регистром в DA50X.
 * Микросхема 74HC595, интерфейс - аппаратно ускоренный SPI.
 * 
 * За некоторую информацию спасибо сайту narodstream:
 * https://narodstream.ru/avr-urok-25-spi-podklyuchaem-sdvigovyj-registr-74hc595/
*/

#include <Arduino.h>
#include <SPI.h>
#include <avr/io.h>
// сигналы на выходе регистра
#define AMP_STANDBY 0
#define AMP_MUTE 1
#define DAC_ENABLE 2
#define BT_RESET 3
#define SIG_SHUTDOWN 4
#define SIG_RESET 5
#define ACT_LED 6

uint8_t _shiftreg_buffer = 0x00;

void _spiSendBuffer() {
    SPDR = _shiftreg_buffer;
    while (!(SPSR & (1 << SPIF)));
    // дёрнуть Latch на регистре
    digitalWrite(12, HIGH);
    digitalWrite(12, LOW);
}

// инициализация шины SPI и обнуление регистра
inline void spiRegisterInit() {
    digitalWrite(12, LOW);
    pinMode(12, OUTPUT); // Latch
    pinMode(11, OUTPUT); // Data
    pinMode(13, OUTPUT); // Clock

    // режим Master, частота шины сокращена до 1 МГц во избежание потерь
    SPCR = ((1 << SPE) | (1 << MSTR) | (1 << SPR0));

    // сразу обнулить сдвиговый регистр
    _spiSendBuffer();
}

// задать состояние вывода внешнего регистра
void shiftWrite(uint8_t pin, bool state) {
    bitWrite(_shiftreg_buffer, pin, state);
    _spiSendBuffer();
}