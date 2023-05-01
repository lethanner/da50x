#include "UI.h"

GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> screen(SSD1306_DISP_I2C);

// переменные для работы меню со скроллингом
byte menuVisibleSelId, menuEntryRendererStartId, menuChooseId;
void (*_handler)(byte);
const char *const *_entries;
byte _entryCount;
bool _handlerAutoCall;
int *_menuBooleans;
int _menuBoolsLast;

byte activeSbIcons;
byte screenId = SCREEN_MAIN;
byte actionId;

uint16_t actionRefreshRate;
uint32_t actionRefreshTimer;

bool displayActive;
uint32_t dimmTimer;

uint32_t blinkTimer;

// спасибо чуваку с форума arduino.ru за реализацию strlen() для юникода
// http://arduino.ru/forum/programmirovanie/pomogite-razobratsya-chto-za-glyuk-co-stringlength#comment-313068
// void setCursorCenter(const char *text)
// {
//     byte i, l = 0;
//     while (text[i++] != '\0')
//     {
//         l++;
//         if (text[i] < 0)
//             i++;
//     }
//     screen.setCursor(64 - ((l * 6) / 2), 0);
// }

void ui_printPGMLine(uint16_t ptr)
{
    char linebuffer[42];
    uint8_t charid = 0;
    do
    {
        linebuffer[charid] = (char)(pgm_read_byte(ptr++));
    } while (linebuffer[charid++] != '\0');
    screen.print(linebuffer);
}

void renderMenuEntries()
{
    const byte endLine = ((_entryCount < 7) ? _entryCount + 1 : 7) + menuEntryRendererStartId;

    uint8_t currentLine = 1;
    for (byte line = menuEntryRendererStartId; line < endLine; line++)
    {
        uint16_t ptr = pgm_read_word(&(_entries[line]));

        // отображение плюсиков и минусиков возле boolean-пунктов меню
        byte flagId = 0;
        bool isBoolEntry = false;
        if ((char)(pgm_read_byte(ptr)) == '?')
        {
            ptr++;
            flagId = pgm_read_byte(ptr++) - 65;
            isBoolEntry = true;
        }

        screen.setCursor(0, currentLine);
        screen.invertText(line == menuChooseId);
        ui_printPGMLine(ptr);
        screen.invertText(0);

        if (isBoolEntry)
        {
            screen.setCursor(120, currentLine);
            if ((*_menuBooleans >> flagId) & 0x01)
                screen.print('+');
            else
                screen.print('-');
        }
        currentLine++;
    }
    
    if (_menuBooleans != NULL)
        _menuBoolsLast = *_menuBooleans;
}

void clearMainArea()
{
    screen.clear(0, 8, 127, 63);
}

void drawBar(byte val, byte max, byte startX, byte startY)
{
    //screen.rect(startX, startY, startX + max + 1, startY + 5, OLED_STROKE);
    if (val > 0)
        screen.rect(startX, startY, startX + val, startY + 5, OLED_FILL);
    if (val < max)
        screen.clear(startX + val, startY, startX + max, startY + 5);
}

void drawBTLogo(bool large)
{
    if (large)
    {
        screen.line(52, 20, 75, 43);
        screen.line(74, 44, 65, 53);
        screen.line(64, 54, 64, 9);
        screen.line(65, 10, 74, 19);
        screen.line(75, 20, 52, 43);
    }
    else
    {
        screen.line(55, 20, 72, 37);
        screen.line(71, 38, 65, 44);
        screen.line(64, 45, 64, 12);
        screen.line(65, 13, 71, 19);
        screen.line(72, 20, 55, 37);
    }
}

/**
 * Вызов меню на экране устройства.
 * 
 * @param entries Указатель на PROGMEM-массив с текстами для строк меню.
 * @param entryCount Количество строк меню.
 * @param handler Функция, принимающая аргумент типа byte, которая будет вызвана при
 *      выборе пункта меню пользователем. В аргумент передаётся номер выбранного пункта.
 * @param title Заголовок меню под F-макро (FlashStringHelper).
 * @param tt_x Смещение заголовка от левого края дисплея в пикселях (обычно используется
 *      для его размещения посередине, а вообще это какой-то костыль).
 * @param handlerAutoCall Автоматически вызывать handler при каждом перемещении по пунктам
 *      меню.
 * @param menuBooleans Указатель на двухбайтовую переменную, хранящую биты состояний для
 *      пунктов меню типа "включено-выключено".
*/
void createMenu(const char *const *entries, byte entryCount, void (*handler)(byte), const __FlashStringHelper *title, byte tt_x, bool handlerAutoCall, int *menuBooleans)
{
    menuChooseId = menuEntryRendererStartId = menuVisibleSelId = 0;
    _handler = handler;
    _handlerAutoCall = handlerAutoCall;
    _entries = entries; // вот эту строчку я однажды забыл и удивлялся, почему МК ресетается при попытке открыть меню
    _menuBooleans = menuBooleans;
    _entryCount = entryCount - 1;

    if (title != NULL)
        drawTitle(title, tt_x);
        
    clearMainArea();
    renderMenuEntries();
    screenId = SCREEN_MENU;
}

void menuRotate(bool dir)
{
    if (dir)
    { // вправо
        if (menuVisibleSelId < 6)
        {
            if (menuVisibleSelId < _entryCount)
                menuVisibleSelId++;
            else
                return;
        }
        else
        {
            if (menuEntryRendererStartId + 6 < _entryCount)
            {
                menuEntryRendererStartId++;
                clearMainArea();
            }
            else
                return;
        }
    }
    else
    { // влево
        if (menuVisibleSelId > 0)
            menuVisibleSelId--;
        else
        {
            if (menuEntryRendererStartId > 0)
            {
                menuEntryRendererStartId--;
                clearMainArea();
            }
            else
                return;
        }
    }

    menuChooseId = menuEntryRendererStartId + menuVisibleSelId;
    renderMenuEntries();

    if (_handlerAutoCall)
        callMenuHandler();
}

void setStatusbarIcon(byte id, bool state)
{
    if (id > 0)
    {
        screen.clear(30, 0, 70, 6);
        bitWrite(activeSbIcons, id-1, state);
    }

    byte dispStartX = 64;
    for (byte i = 0; i < 5; i++)
    {
        if ((activeSbIcons >> i) & 0x01)
        {
            screen.drawBitmap(dispStartX, 0, statusbar_icons[i], 5, 8);
            dispStartX -= 7;
        }
    }
}

void callMenuHandler()
{
    _handler(menuChooseId);
    if (_menuBooleans != NULL && *_menuBooleans != _menuBoolsLast)
        renderMenuEntries();
}

void reactivateDisplay()
{
    dimmTimer = timer0_millis;
    if (displayActive)
        return;

    //screen.setPower(true);
    screen.setContrast(127);
    displayActive = true;
}

void dimmDisplay()
{
    if (displayActive && timer0_millis - dimmTimer > DISP_AUTO_DIMM_TIMEOUT_MS)
    {
        screen.setContrast(10);
        displayActive = false;
    }
}

/**
 * Управление сигнальным светодиодом передней панели.
 * 
 * @param state Состояние светодиода, true/false
 * @param onInterval Интервал (в мс), в течение которого светодиод будет включен
 * @param blinkTimes Количество вспышек светодиода (0 - бесконечно)
 * @param offInterval Интервал (в мс), в течение которого светодиод будет выключен (при мигании)
 * @param repeatInterval Интервал (в мс), через который вспышки будут повторяться (0 - не повторять)
 * @param _override Приоритет мигания светодиода для заданного сценария
 *      (0 - без приоритета, 1 - игнорировать последующие вызовы функции без
 *      override, 2 - приостановить всю программу при выполнении сценария)
*/
// TODO! TODO!!!
//void setIndicator(bool state, uint16_t onInterval, uint8_t blinkTimes, uint16_t offInterval, uint16_t repeatInterval, uint8_t _override)
void setIndicator(bool state)
{
    extWrite(EXT_INDICATOR, state);
}

void initializeMenuAction(const __FlashStringHelper *title, byte tt_x, byte actId, uint16_t refreshRate_ms)
{
    drawTitle(title, tt_x);
    actionRefreshRate = refreshRate_ms;
    //actionRefreshTimer = timer0_millis;
    actionId = actId;
    screenId = SCREEN_MENU_ACT;
    clearMainArea();
}

void drawTitle(const __FlashStringHelper *title, uint8_t offset)
{
    screen.rect(0, 0, 127, 7, OLED_FILL);
    screen.setCursor(offset, 0);
    screen.invertText(1);
    screen.print(title);
    screen.invertText(0);
}