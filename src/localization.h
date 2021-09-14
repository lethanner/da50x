#include <Arduino.h> // из-за PROGMEM, чтобы intellisense не ругался

const char i0[] PROGMEM = "Entry 0";
const char i1[] PROGMEM = "Entry 1";
const char i2[] PROGMEM = "Entry 2";

const char a0[] PROGMEM = "Да";
const char a1[] PROGMEM = "Нет";
const char a2[] PROGMEM = "Отмена";

const char b0[] PROGMEM = "Пунктик 1";
const char b1[] PROGMEM = "Большой теееекст";
const char b2[] PROGMEM = "2";

const char *const names[] PROGMEM = {i0, i1, i2};
const char *const namea[] PROGMEM = {a0, a1, a2};
const char *const nameb[] PROGMEM = {b0, b1, b2};