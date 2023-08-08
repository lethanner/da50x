// Формула расчёта смещения для вставки текста посередине:
// x = 64 - ((n * 6) / 2), где n - число знаков

#ifndef _localization_h
#define _localization_h

#include <Arduino.h> // из-за PROGMEM, чтобы intellisense не ругался
#include "config.h"

// Русский язык
#ifdef RUSSIAN

#define TT_ERROR "Произошла ошибка.\r\nКод: "

#define MASTER_VOLUME_OFFSET 37
#define MASTER_VOLUME "Громкость"
#define SUB_VOLUME "Громкость НЧ"
#define SUB_VOLUME_OFFSET

const char fsrc0[] PROGMEM = "Без входа";
const char fsrc1[] PROGMEM = "USB ЦАП";
//const char fsrc3[] PROGMEM = "FM-радио";
//const char fsrc4[] PROGMEM = "Карта памяти";
//const char fsrc5[] PROGMEM = "USB-накопитель";

#define SOURCE "Источник"

#define WAIT "Подождите..."
#define WAIT_OFFSET 28

#define BT_SEARCH "Поиск..."
#define BT_CLICK_TO_PAIR "Нажм. для сопряжения"
#define BT_PAIRING "Ожидание..."
#define BT_CONNECTING "Соединение..."
#define BT_SEARCH_OFFSET 40
#define BT_CLICK_TO_PAIR_OFFSET 4
#define BT_PAIRING_OFFSET 31

#define NOTHING "Ничего нет."
#define NOTHING_OFFSET 31
#define AUDIO_ONLY "Только аудио"
#define AUDIO_ONLY_OFFSET 28

#define DEVICE_MENU "Устройство"
#define DEVICE_MENU_OFFSET 34
#define USB_TITLE "USB ЦАП"
#define USB_TITLE_OFFSET 43

const char srcmenu0[] PROGMEM = "Источник...";
const char srcmenu1[] PROGMEM = "Параметры...";
const char srcmenu2[] PROGMEM = "Выключение...";

const char btmenu0[] PROGMEM = "Отключиться";
const char btmenu1[] PROGMEM = "Сброс сопряжений";
const char btmenu2[] PROGMEM = "Перезапуск";
const char btmenu0_1[] PROGMEM = "Подкл. к последн.";

#define SETTINGS "Параметры"
#define SETTINGS_OFFSET 37
const char settings_m0[] PROGMEM = "?AАвтопереключение";
const char settings_m1[] PROGMEM = "?BМониторинг";
const char settings_m2[] PROGMEM = "Настр. ФНЧ";
const char settings_m3[] PROGMEM = "?CТолько ЦАП";
const char settings_m5[] PROGMEM = "?DБыстрая регулировка";
const char settings_m7[] PROGMEM = "Статистика";

#define BALANCE "Баланс"
#define BALANCE_OFFSET 46

#define DAC_ONLY_MARKER "внешн."
#define DAC_ONLY_MARKER_OFFSET 91
#define DAC_ONLY_VOLUME_REJECT "Недоступно"
#define DAC_ONLY_VOLUME_REJECT_2 "в этом режиме"
#define DAC_ONLY_VOLUME_REJECT_OFFSET 34
#define DAC_ONLY_VOLUME_REJECT_2_OFFSET 25

#define STATISTICS "Статистика"
#define STATISTICS_OFFSET 34
#define STATS_BDATE "ПО:"
#define STATS_BDATE_OFFSET 24
#define STATS_UPTIME "Время работы:"
#define STATS_UPTIME_OFFSET 84
#define STATS_VOLTAGE "Питание:"
#define STATS_VOLTAGE_OFFSET 54
//#define ALL_F____D_UP "Напряжение линии\r\n5В недостаточно\r\nдля работы DA50X.\r\n\nТребуется замена\r\nисточника питания"

#else // Английский язык

#define TT_ERROR "An error occured.\r\nIssue # "

#define MASTER_VOLUME_OFFSET 25
#define MASTER_VOLUME "Master volume"
#define SUB_VOLUME "Subwoofer volume"
#define SUB_VOLUME_OFFSET

#define SOURCE "Source"
#define SOURCE_OFFSET 46
const char fsrc0[] PROGMEM = "No input";
const char fsrc1[] PROGMEM = "USB DAC";
//const char fsrc3[] PROGMEM = "FM radio";
//const char fsrc4[] PROGMEM = "TF card";
//const char fsrc5[] PROGMEM = "USB flash";

#define WAIT "Please wait"
#define WAIT_OFFSET 31

#define BT_SEARCH "Searching..."
#define BT_CLICK_TO_PAIR "Click to pair"
#define BT_PAIRING "Waiting..."
#define BT_CONNECTING "Connecting..."
#define BT_SEARCH_OFFSET 28
#define BT_CLICK_TO_PAIR_OFFSET 25
#define BT_PAIRING_OFFSET 34

#define NOTHING "Just nothing."
#define NOTHING_OFFSET 25
#define AUDIO_ONLY "Audio only"
#define AUDIO_ONLY_OFFSET 31

#define DEVICE_MENU "Device menu"
#define DEVICE_MENU_OFFSET 31
#define USB_TITLE "USB DAC"
#define USB_TITLE_OFFSET 43
const char srcmenu0[] PROGMEM = "Source...";
const char srcmenu1[] PROGMEM = "Settings...";
const char srcmenu2[] PROGMEM = "Shutdown...";

const char btmenu0[] PROGMEM = "Disconnect";
const char btmenu1[] PROGMEM = "Clear pairs";
const char btmenu2[] PROGMEM = "Reset";
const char btmenu0_1[] PROGMEM = "Reconnect to last";

#define SETTINGS "Settings"
#define SETTINGS_OFFSET 40
const char settings_m0[] PROGMEM = "?AAuto switch";
const char settings_m1[] PROGMEM = "?BMonitoring";
const char settings_m2[] PROGMEM = "LPF cutoff";
const char settings_m6[] PROGMEM = "Stereo balance";
const char settings_m3[] PROGMEM = "?CDAC-only mode";
const char settings_m5[] PROGMEM = "?DQuick volume";
const char settings_m7[] PROGMEM = "Statistics";

#define BALANCE "Balance"
#define BALANCE_OFFSET 43

#endif

const char settings_m4[] PROGMEM = "Debug";
const char fsrc2[] PROGMEM = "Bluetooth";
#define BLUETOOTH_R "Bluetooth(R)"
#define BLUETOOTH_OFFSET 28
#define BT_TITLE "Bluetooth"
#define BT_TITLE_OFFSET 37

#define DAC_ONLY_MARKER "DAC-only"
#define DAC_ONLY_MARKER_OFFSET 79
#define DAC_ONLY_VOLUME_REJECT "Not available"
#define DAC_ONLY_VOLUME_REJECT_2 "in this mode."
#define DAC_ONLY_VOLUME_REJECT_OFFSET 25
#define DAC_ONLY_VOLUME_REJECT_2_OFFSET 25

#define STATISTICS "Statistics"
#define STATISTICS_OFFSET 34
#define STATS_BDATE "FW:"
#define STATS_BDATE_OFFSET 24
#define STATS_UPTIME "Uptime:"
#define STATS_UPTIME_OFFSET 48
#define STATS_VOLTAGE "Voltage:"
#define STATS_VOLTAGE_OFFSET 54
//#define ALL_F____D_UP "Insufficient 5V\r\nrail voltage has\r\nbeen detected.\r\nDA50X functions\r\ncan not be used.\r\n\nREPLACE POWER SUPPLY"

// Микросдшка и юсб флешка вообще нужны на этом аппарате? Ладно радио, но вот, боюсь, музычка через DFPlayer будет полным извращением.
const char src0[] PROGMEM = "null";
const char src1[] PROGMEM = "USB";
const char src2[] PROGMEM = "BT";
//const char src3[] PROGMEM = "FM";
//const char src4[] PROGMEM = "TF";
//const char src5[] PROGMEM = "UDISK";

const char splitter[] PROGMEM = "--";

const char *const sb_sources[] PROGMEM = {
    src0, src1, src2//, src3, src4, src5
};

const char *const menu_sources[] PROGMEM = {
    fsrc0, fsrc1, fsrc2//, fsrc3, fsrc4, fsrc5
};

const char *const bt_menu_entr[] PROGMEM = {
    srcmenu0, srcmenu1, srcmenu2, splitter, btmenu0, btmenu1, btmenu2
};
const char *const bt_menu_entr_rec[] PROGMEM = {
    srcmenu0, srcmenu1, srcmenu2, splitter, btmenu0_1, btmenu1, btmenu2
};

const char *const null_src_menu[] PROGMEM = {
    srcmenu0, srcmenu1, srcmenu2
};

const char *const settings_menu[] PROGMEM = {
    settings_m0, settings_m1, settings_m2, settings_m6, settings_m3, settings_m5, settings_m7, settings_m4
};

#endif