// Формула расчёта смещения для вставки текста посередине:
// x = 64 - ((n * 6) / 2), где n - число знаков

#ifndef _localization_h
#define _localization_h

#include <Arduino.h> // из-за PROGMEM, чтобы intellisense не ругался
#include "config.h"

// const char i0[] PROGMEM = "Баланс";
// const char i1[] PROGMEM = "Огр. мощности";
// const char i2[] PROGMEM = "?AСинхр. регулировка";
// const char i3[] PROGMEM = "?BАвтопереключение";
// const char i4[] PROGMEM = "ФНЧ";
// const char i5[] PROGMEM = "Яркость дисп.";
// const char i6[] PROGMEM = "Режим индикатора";
// const char i7[] PROGMEM = "?CЭнергосбережение";
// const char i8[] PROGMEM = "Сброс";

// const char *const names[] PROGMEM = {i0, i1, i2, i3, i4, i5, i6, i7, i8};

// Я пришёл русский язык учить (С) Russia Paver
#ifdef RUSSIAN

#define TT_ERROR "Произошла ошибка.\r\nКод: "

#define MASTER_VOLUME_OFFSET 37
#define MASTER_VOLUME "Громкость"
#define SUB_VOLUME "Громкость НЧ"
#define SUB_VOLUME_OFFSET

const char fsrc0[] PROGMEM = "Без входа";
const char fsrc1[] PROGMEM = "USB ЦАП";
const char fsrc3[] PROGMEM = "FM-радио";
const char fsrc4[] PROGMEM = "Карта памяти";
const char fsrc5[] PROGMEM = "USB-накопитель";

#define SOURCE "Источник"

#define BT_SEARCH "Поиск..."
#define BT_CLICK_TO_PAIR "Нажм. для сопряжения"
#define BT_PAIRING "Ожидание..."
#define BT_CONNECTING "Соединение..."
#define BT_SEARCH_OFFSET 40
#define BT_CLICK_TO_PAIR_OFFSET 4
#define BT_PAIRING_OFFSET 31

#else // Английский язык

#define TT_ERROR "An error occured.\r\nIssue # "

#define MASTER_VOLUME_OFFSET 25
#define MASTER_VOLUME "Master volume"
#define SUB_VOLUME "Subwoofer volume"
#define SUB_VOLUME_OFFSET

const char fsrc0[] PROGMEM = "No input";
const char fsrc1[] PROGMEM = "USB DAC";
const char fsrc3[] PROGMEM = "FM-radio";
const char fsrc4[] PROGMEM = "TF card";
const char fsrc5[] PROGMEM = "USB flash";

#define SOURCE "Source"
#define SOURCE_OFFSET 46

#define BT_SEARCH "Searching..."
#define BT_CLICK_TO_PAIR "Click to pair"
#define BT_PAIRING "Waiting..."
#define BT_CONNECTING "Connecting..."
#define BT_SEARCH_OFFSET 28
#define BT_CLICK_TO_PAIR_OFFSET 25
#define BT_PAIRING_OFFSET 34

#endif

const char src0[] PROGMEM = "null";
const char src1[] PROGMEM = "USB";
const char src2[] PROGMEM = "BT";
const char src3[] PROGMEM = "FM";
const char src4[] PROGMEM = "TF";
const char src5[] PROGMEM = "UDISK";

const char fsrc2[] PROGMEM = "Bluetooth";
#define BLUETOOTH "Bluetooth(R)"
#define BLUETOOTH_OFFSET 28

const char *const sb_sources[] PROGMEM = {
    src0, src1, src2, src3, src4, src5
};

const char *const menu_sources[] PROGMEM = {
    fsrc0, fsrc1, fsrc2
};

#endif