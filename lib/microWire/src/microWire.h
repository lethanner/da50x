﻿/*
    Лёгкая библиотека со стандартным набором инструментов для работы с аппаратным I2C
    Документация: 
    GitHub: https://github.com/GyverLibs/microWire
    Возможности:
    - Облегчайте свой код простой заменой Wire.h на microWire.h
    - ATmega168/328p (nano,uno,mini), ATmega32u4 (leonardo,micro) , ATmega2560 (mega)
    
    Egor 'Nich1con' Zaharov for AlexGyver, alex@alexgyver.ru
    https://alexgyver.ru/
    MIT License

    Версии:
    v2.1
*/

/*
    ВНИМАНИЕ!!!
    Для экономии места и возможности запроса большого кол-ва байта (например чтение пакетов EEPROM) , буферов на чтение и запись НЕТ!!!
    Функция write сразу отправляет байт на шину , не дожидаясь endTransmission , очереди на отправку нет , отправка занимает некоторое время.
    Функция read напрямую читает байт из шины, специфика работы с шиной требует читать последний байт в особом порядке , читайте сразу ВСЕ запрошенные байты.
    Пока не будет прочитан последний байт , шина будет занята. Для чтения всех байт воспользуйтесь конструкцией " for(unt8_t i = 0; Wire.available() ,i++){ data[i] = Wire.read } ".
*/

#ifndef microWire_h
#define microWire_h
#include <Arduino.h>
#include "pins_arduino.h"

class TwoWire {
public:
    void begin(void);            				// инициализация шины
    void setClock(uint32_t clock);       		// ручная установка частоты шины 31-900 kHz (в герцах)
    void beginTransmission(uint8_t address); 	// открыть соединение (для записи данных)
    uint8_t endTransmission(bool stop);  		// закрыть соединение , произвести stop или restart (по умолчанию - stop)
    uint8_t endTransmission(void);  			// закрыть соединение , произвести stop
    void write(uint8_t data);                	// отправить в шину байт данных , отправка производится сразу , формат - byte "unsigned char"
    void requestFrom(uint8_t address , uint8_t length , bool stop); //открыть соединение и запросить данные от устройства, отпустить или удержать шину
    void requestFrom(uint8_t address , uint8_t length);  			//открыть соединение и запросить данные от устройства, отпустить шину
    uint8_t read(void);                      	// прочитать байт , БУФЕРА НЕТ!!! , читайте сразу все запрошенные байты , stop или restart после чтения последнего байта, настраивается в requestFrom
    uint8_t available(void);                 	// вернет количество оставшихся для чтения байт
private:
    uint8_t _requested_bytes = 0;            	// переменная хранит количество запрошенных и непрочитанных байт
    bool _address_nack = false;					// Флаг для отслеживания ошибки при передаче адреса
    bool _data_nack = false;					// Флаг для отслеживания ошибки при передаче данных
    bool _stop_after_request = true;         	// stop или restart после чтения последнего байта
    void start(void);                        	// сервисная функция с нее начинается любая работа с шиной
    void stop(void);                         	// сервисная функция ей заканчивается работа с шиной
};
extern TwoWire Wire;
#endif