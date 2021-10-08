#include <Arduino.h> // из-за PROGMEM, чтобы intellisense не ругался

const char i0[] PROGMEM = "здравствуй";
const char i1[] PROGMEM = "небо";
const char i2[] PROGMEM = "в облаках";
const char i3[] PROGMEM = "здравствуй";
const char i4[] PROGMEM = "юность";
const char i5[] PROGMEM = "в сапогах";
const char i6[] PROGMEM = "пропади";
const char i7[] PROGMEM = "моя тоска";
const char i8[] PROGMEM = "вот он я";
const char i9[] PROGMEM = "привет";
const char i10[] PROGMEM = "ВОЙСКА!!!";

const char a0[] PROGMEM = "А";
const char a1[] PROGMEM = "Б";
const char a2[] PROGMEM = "В";
const char a3[] PROGMEM = "Г";
const char a4[] PROGMEM = "Д";
const char a5[] PROGMEM = "Е";
const char a6[] PROGMEM = "Ж";

const char b0[] PROGMEM = "Пунктик 1";
const char b1[] PROGMEM = "Большой теееекст";
const char b2[] PROGMEM = "2";

const char *const names[] PROGMEM = {i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10};
const char *const namea[] PROGMEM = {a0, a1, a2, a3, a4, a5, a6};
const char *const nameb[] PROGMEM = {b0, b1, b2};