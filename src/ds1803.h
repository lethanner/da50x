#include "microWire.h"
#include "defines.h"
#include "config.h"

#define HNDR_TO_BYTE false
#define BYTE_TO_HNDR true

byte _digipot_cache[2];

/* За основу нагло взят исходник оригинальной ардуиновской функции map().
 * Доработан под собственные нужды, в частности, 8-битный int вместо 32-битного,
 * нафиг шерудить ненужные 32-бит числа на 8-бит процессоре?
 * 
 * Так-то компилятор должен это оптимизировать, но я на всякий случай...
 * 02.12.2021, 23:46
*/
byte volConvert(byte input, bool direction)
{
    if (direction)
        return input * 255 / 100;
    else
        return input * 100 / 255;       
    //return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

bool writeValues()
{
    Wire.beginTransmission(DIGIPOT_MASTER_I2C);
    Wire.write(0b10101001); // команда на запись обеих потенциометров
    Wire.write(_digipot_cache[0]);
    Wire.write(_digipot_cache[1]);

    return (Wire.endTransmission() == 0);
}

bool setVolume(byte vol)
{
    vol = volConvert(vol, HNDR_TO_BYTE);
    _digipot_cache[0] = vol, _digipot_cache[1] = vol;

    return writeValues();
}