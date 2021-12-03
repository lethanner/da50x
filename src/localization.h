#include <Arduino.h> // из-за PROGMEM, чтобы intellisense не ругался

const char i0[] PROGMEM = "Баланс";
const char i1[] PROGMEM = "Огр. мощности";
const char i2[] PROGMEM = "?AСинхр. регулировка";
const char i3[] PROGMEM = "?BАвтопереключение";
const char i4[] PROGMEM = "ФНЧ";
const char i5[] PROGMEM = "Яркость дисп.";
const char i6[] PROGMEM = "Режим индикатора";
const char i7[] PROGMEM = "?CЭнергосбережение";
const char i8[] PROGMEM = "Сброс";

const char *const names[] PROGMEM = {i0, i1, i2, i3, i4, i5, i6, i7, i8};