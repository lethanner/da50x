#ifndef _ui_h
#define _ui_h

#define USE_MICRO_WIRE
#include <GyverOLED.h>
#include "config.h"
#include "bitmaps.h"
#include "localization.h"
#include "HW/hardware.h"

// текстовое обозначение состояний обработчика ручки управления
#define CTRL_CLICK 1
#define CTRL_HOLDING 2
#define CTRL_ROTATING 3

// идентификаторы типов содержимого на экране
#define SCREEN_MAIN 0
#define SCREEN_ACTION 1
#define SCREEN_MENU 2
#define SCREEN_MENU_ACT 3

// идентификаторы экранов действий
#define ACTION_DEBUG 0
#define ACTION_BALANCE 1
#define ACTION_STATS 2

extern GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> screen;

// extern byte menuVisibleSelId, menuEntryRendererStartId,
extern byte menuChooseId;
extern void (*_handler)(byte);
// extern const char *const *_entries;
// extern byte _entryCount;
// extern bool _handlerAutoCall;
// extern int *_menuBooleans;

// extern bool rot_dir;
// extern uint8_t ctrl_state;
// extern uint32_t hold_timer;
extern uint32_t blinkTimer;
extern uint32_t actionRefreshTimer;
extern uint16_t actionRefreshRate;

extern byte screenId, actionId;

extern volatile unsigned long timer0_millis;

inline bool disp_initialize()
{
    screen.init();
    /* 
    * Да-да, именно, наличие дисплея на линии I2C будет проверено уже после попытки инициализации.
    * Если бы я поставил это до инициализации, то мне бы пришлось делать Wire.begin(), а это уже
    * вызывается функцией init() в той библиотеке.
    * Дорабатывать либу не хочу.
    */
    Wire.beginTransmission(SSD1306_DISP_I2C);
    if (Wire.endTransmission() > 0)
        return false;

    Wire.setClock(I2C_SPEED_HZ);
    return true;
}

void ui_tick();
void ui_redraw(bool sb_force = false);
void ui_refresh(bool fullRefresh = true);
void ui_printPGMLine(uint16_t ptr);
void clearMainArea();
void menuRotate(bool dir);

void drawTitle(const __FlashStringHelper *title, uint8_t offset);
void createMenu(const char *const *entries, byte entryCount, void (*handler)(byte), const __FlashStringHelper *title = NULL, byte tt_x = 0, bool handlerAutoCall = false, int *menuBooleans = NULL);
void initializeMenuAction(const __FlashStringHelper *title, byte tt_x, byte actId, uint16_t refreshRate_ms = 0);
void drawBar(int8_t val, byte range, byte startX, byte startY, bool canBeInverted = false);
void drawBTLogo(bool large = false);
void printTimeValue(uint16_t value);

void setStatusbarIcon(byte id = 0, bool state = true, bool alternative = false);

void ctrl_update();

void callMenuHandler();

void reactivateDisplay();
void dimmDisplay();

void setIndicator(bool state);
//void setIndicator(bool state, uint16_t onInterval = 0, uint8_t blinkTimes = 0, uint16_t offInterval = 0, uint16_t repeatInterval = 0, uint8_t _override = 0);

#endif