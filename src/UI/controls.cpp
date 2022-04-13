#include "UI.h"

// переменные для ручки управления
bool rot_dir;
uint8_t ctrl_state;
uint32_t hold_timer;

uint32_t act_timeout;
bool redrawMainScreen = true, redrawStatusBar = true;

char heatsink_temp = 0;
byte volume_master = INIT_VOL_HNDR;
byte statusbar_show_icons;
byte device_mode;
byte currentSourceId = 255, pendingSourceId = 0; // с 255 костыль, но пока что некуда деваться. Ах да, пишу в час ночи.

// прерывание PORT CHANGE для ручки управления
ISR(PCINT1_vect)
{
    // вращение энкодера, проще простого
    if (!(PINC & 0x01))
    {
        rot_dir = !((PINC >> 1) & 0x01);
        ctrl_state = CTRL_ROTATING;
    }
    // обработка кнопки вместе с удержанием, уже посложнее
    bool btn_state = !((PINC >> 2) & 0x01);
    if (btn_state)
    {
        hold_timer = timer0_millis;
        ctrl_state = CTRL_HOLDING;
    }
    else if (ctrl_state == CTRL_HOLDING && !btn_state)
        ctrl_state = CTRL_CLICK;
}

void setSource(byte id)
{
    switch (id)
    {
    case SRC_USB:
        if (!((PIND >> 2) & 0x01))
            return;
        break;
    case SRC_BT:
        break;
    }

    pendingSourceId = id;
    redrawMainScreen = redrawStatusBar = true;
}

void ctrl_update()
{
    if (ctrl_state > 0)
    {
        // действие при нажатии на ручку управления
        if (ctrl_state == CTRL_CLICK)
        {
            switch (currentScreen)
            {
            case SCREEN_MAIN:
                if (currentSourceId == SRC_BT)
                {
                    if (bt_conn_count > 1)
                        bt_sendAT("MA", false);
                    else
                        if (!bt_universal_bool)
                            bt_enterPairingMode();
                }
                break;
            case SCREEN_MENU:
                callMenuHandler();
                break;
            }
        }

        // действие при вращении энкодера
        else if (ctrl_state == CTRL_ROTATING)
        {
            switch (currentScreen)
            {
            case SCREEN_MAIN:
                clearMainArea();
                screen.setCursor(MASTER_VOLUME_OFFSET, 3);
                screen.print(F(MASTER_VOLUME));
                currentScreen = SCREEN_ACTION;
                //break;
            case SCREEN_ACTION:
                if (!rot_dir && volume_master > 0)
                    volume_master--;
                if (rot_dir && volume_master < 100)
                    volume_master++;

                setVolume(volume_master);
                drawBar(volume_master, 100, 14, 44);
                act_timeout = timer0_millis;
                break;
            case SCREEN_MENU:
                menuFlip(rot_dir);
                break;
            }
        }

        if (ctrl_state != CTRL_HOLDING)
            ctrl_state = 0;
        // действие при удержании кнопки
        else if (timer0_millis - hold_timer > CTRL_HOLD_TIMEOUT_MS)
        {
            createMenu(menu_sources, 3, setSource, SOURCE, SOURCE_OFFSET);
            ctrl_state = 0;
        }
    }
}

void statusbar_tick()
{
    if (currentScreen > SCREEN_ACTION)
        return;

    // Кэширование предыдущих значений содержимого статусной строки.
    // Прощай ОЗУ...
    static char _heatsink_temp_last;
    static byte _volume_last;
    //static byte _statusbar_show_icons_last;
    //static byte _device_mode_last;
    //static byte _source_id_last;

    if (redrawStatusBar)
    {
        screen.clear(0, 0, 127, 7);
        screen.setCursor(0, 0);
        printPGMLine(pgm_read_word(&(sb_sources[currentSourceId])));

        screen.setCursor(91, 0);
        screen.print(F("%"));
        screen.setCursor(115, 0);
        screen.print(F("'C"));
    }

    if ((heatsink_temp != _heatsink_temp_last) || redrawStatusBar)
    {
        screen.setCursor(103, 0);
        if (heatsink_temp < 10)
            screen.print(' ');
        screen.print((heatsink_temp > 99) ? 99 : heatsink_temp);
        _heatsink_temp_last = heatsink_temp;
    }

    if ((_volume_last != volume_master) || redrawStatusBar)
    {
        screen.setCursor(73, 0);
        if (volume_master < 100)
            screen.print(' ');
        if (volume_master < 10)
            screen.print(' ');
        screen.print(volume_master);
        _volume_last = volume_master;
    }

    redrawStatusBar = false;
}

void main_tick()
{
    if (currentScreen == SCREEN_ACTION && timer0_millis - act_timeout > ACT_AUTOCLOSE_TIMEOUT_MS)
        redrawMainScreen = true;

    if (redrawMainScreen)
    {
        clearMainArea();
        if (currentSourceId != pendingSourceId)
        {
            if (pendingSourceId == SRC_BT)
            {
                drawBTLogo(true);
                screen.setCursor(BLUETOOTH_OFFSET, 7);
                screen.print(F(BLUETOOTH));
            }
            else
            {
                screen.setCursor(4, 2);
                screen.print(F("wait"));
            }
        }
        else
        {
            if (currentSourceId == SRC_BT)
            {
                drawBTLogo();
                if (bt_conn_count == 0)
                {
                    if (!bt_universal_bool)
                    {
                        screen.setCursor(BT_SEARCH_OFFSET, 6);
                        screen.print(F(BT_SEARCH));
                        screen.setCursor(BT_CLICK_TO_PAIR_OFFSET, 7);
                        screen.print(F(BT_CLICK_TO_PAIR));
                    }
                    else
                    {
                        screen.setCursor(BT_PAIRING_OFFSET, 6);
                        screen.print(F(BT_PAIRING));
                    }
                }
                else
                {
                    screen.setCursor(0, 7);
                    if (bt_conn_count == 1)
                        screen.print(F(BT_CONNECTING));
                    else
                        screen.print(bt_playback_state ? F(">") : F("-"));
                    screen.setCursor(109, 7);
                    screen.print(bt_conn_count - 1);
                    screen.print(F("/2"));
                }
            }
            else
            {
                screen.setCursor(4, 2);
                screen.print(F("active"));
            }
        }
        currentScreen = SCREEN_MAIN;
        redrawMainScreen = false;
    }

    if (currentSourceId != pendingSourceId)
    {
        // отключить предыдущий источник
        switch (currentSourceId)
        {
        case SRC_USB:
            extWrite(EXT_USB_ENABLE, false);
            break;
        case SRC_BT:
            if (!bt_disable())
                terminate(95);
            break;
        }

        switch (pendingSourceId)
        {
        case SRC_NULL:
            break;
        case SRC_BT:
            if (!bt_activate())
                terminate(97);
            break;
        case SRC_USB:
            extWrite(EXT_USB_ENABLE, true);
            break;
        }

        currentSourceId = pendingSourceId;
        redrawMainScreen = redrawStatusBar = true;
    }

    if (currentScreen > SCREEN_MAIN)
        return;
    // TODO: обновляемые в реалтайме значения в основной области
    if (bt_data_refresh)
    {
        redrawMainScreen = true;
        bt_data_refresh = false;
    }
}