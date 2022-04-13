#ifndef _ui_h
#define _ui_h

#define USE_MICRO_WIRE
#include <GyverOLED.h>
#include "config.h"
#include "defines.h"
#include "localization.h"
#include "spiregister.h"
#include "bluetooth.h"
#include "ds1803.h"

extern GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> screen;

// extern byte menuVisibleSelId, menuEntryRendererStartId,
extern byte menuChooseId;
extern void (*_handler)(byte);
// extern const char *const *_entries;
// extern byte _entryCount;
// extern bool _handlerAutoCall;
// extern int *_menuBooleans;

extern byte currentScreen;
extern byte pendingSourceId;
extern bool redrawMainScreen, redrawStatusBar;

// extern bool rot_dir;
// extern uint8_t ctrl_state;
// extern uint32_t hold_timer;

extern char heatsink_temp;
extern byte statusbar_show_icons;

extern byte device_mode;
//extern uint32_t act_timeout;

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

void setCursorCenter(const char *text);
//void _drawTitle(const char *title, byte x);
void printPGMLine(uint16_t ptr);
void renderMenuEntries();
void clearMainArea();
void activateDisplay();
void dimmDisplay();
void menuFlip(bool dir);

void createMenu(const char *const *entries, byte entryCount, void (*handler)(byte), const char *title = NULL, byte tt_x = 0, bool handlerAutoCall = false, int *menuBooleans = NULL);
void drawBar(byte val, byte max, byte startX, byte startY);
void drawBTLogo(bool large = false);
void terminate(byte err_code);
//void denySelection();

void main_tick();
void statusbar_tick();
void ctrl_update();

inline void callMenuHandler()
{
    _handler(menuChooseId);
}

inline void ctrl_initialize()
{
    // пины A0, A1, A2 на вход
    DDRC &= ~0b00000111;
    // иниц. прерывания для ручки управления
    PCMSK1 |= 0b00000101;
    PCICR |= (1 << 1);
}

#endif