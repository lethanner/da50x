/************************************************************************
* Light & powerful Dallas 1-Wire DS18B20 thermometer arduino library	*
* Designed specifically for	AlexGyver by Egor 'Nich1con' Zakharov		*
* V2.0 from 23.02.2020													*
* V2.1 from 10.04.2020													*
* V2.2 from 26.04.2020													*
************************************************************************/

#pragma once
#include <Arduino.h>

/*
	Легкая и удобная в обращении arduino - библиотека для работы с 1-Wire термометрами DS18B20
	- Возможность работы с несколькими датчиками на одном пине при использовании 8-битного уникального адреса
	- Возможность работы с одним датчиком на пине без использования адресации
	- Настраиваемое разрешение преобразования
	- Проверка подлинности данных "на лету", с использованием CRC8 от Dallas [#define DS_CHECK_CRC]
	- Рассчет CRC8 (~6 мкс) или чтение из таблицы (<1 мкс + 256 байт flash) [#define DS_CRC_USE_TABLE]
	- Рассчет температуры как в целых числах, так и с плавающей точкой [#define DS_TEMP_TYPE float / int]
	- Чтение уникального адреса подключенного термометра в указанный массив типа byte
*/

/*
Время исполнения функций для работы с датчиком (частота ядра - 16 МГц):
 _________________________________________________________________________________
| Датчик без адресации (один на линии) | Датчик с адресацией (несколько на линии) |
|______________________________________|__________________________________________|
| .setResolution(...) ~ 3591.125 мкс   | .setResolution(...) ~ 8276.0625 мкс 	  |
| .requestTemp() ~ 1839.9375 мкс	   | .requestTemp() ~ 6522.1875 мкс 		  |
|______________________________________|__________________________________________|
|							С использованием CRC8								  |
|_________________________________________________________________________________|
| .readAddress(...) ~ 6467.3125 мкс	   | float .getTemp() ~ 12250.25 мкс	 	  | 
| float .getTemp() ~ 7620.25 мкс	   | int .getTemp() ~ 12250.2500 мкс 		  |	
| int .getTemp() ~ 7574.0625 мкс	   | 									      |
|______________________________________|__________________________________________|
|							Без использования CRC8								  |
|_________________________________________________________________________________|
| .readAddress(...) ~ 6394.3125 мкс    | float .getTemp() ~ 7809.1250 мкс	 	  |
| float .getTemp() ~ 3132.9375 мкс	   | int .getTemp() ~ 7809.1250 мкс	 		  |
| int .getTemp() ~ 3132.9375 мкс	   | 								  		  |
|______________________________________|__________________________________________|
*/

/*************************************************** Настройки ***************************************************/
#define DS18B20_DA50X
#define DS_TEMP_TYPE int	  	  // float/int - тип данных для рассчета температуры float / int <- (экономит flash, выполняется быстрее)
#define DS_CHECK_CRC true		  // true/false - проверка контрольной суммы принятых данных <- надежнее, но тратит немного больше flash
#define DS_CRC_USE_TABLE false   // true/false - использовать готовую таблицу контрольной суммы <- значительно быстрее, +256 байт flash

/************************************************* Класс **************************************************/
class MicroDS18B20 {
public:
	MicroDS18B20();
	MicroDS18B20(uint8_t pin);              			// Создать обьект термометра без адресации
	MicroDS18B20(uint8_t pin, uint8_t *address); 		// Создать обьект термометра с адресацией

	void setResolution(uint8_t resolution);       		// Установить разрешение термометра 9-12 бит
	void readAddress(uint8_t *addressArray);        	// Прочитать уникальный адрес термометра в массив
	void setAddress(uint8_t *address);					// Установить адрес
	void setPin(uint8_t pin); 							// Установить пин

	void requestTemp(void);								// Запросить новое преобразование температуры
	DS_TEMP_TYPE getTemp(void);							// Прочитать значение температуры

private: 
	void addressRoutine(void);							// Процедура адресации / пропуска адресации
	uint8_t crc_update(uint8_t crc, uint8_t data);		// Обновление значения CRC8
	
	uint8_t _ds_pin;									// Пин термометра
	bool _ds_address_defined = false;					// Флаг - определен ли адрес?
	uint8_t *_ds_address;								// Указатель на адрес термометра 
};