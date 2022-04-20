#include "UI.h"

// переменные для ручки управления
bool rot_dir;
uint8_t ctrl_state;
uint32_t hold_timer;

uint32_t actTimer;

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

void _hSource(byte id)
{
    if (!checkInputAvailability(id))
        return;

    clearMainArea();
    switch (id)
    {
    case SRC_BT:
        drawBTLogo(true);
        screen.setCursor(BLUETOOTH_OFFSET, 7);
        screen.print(F(BLUETOOTH_R));
        break;
    default:
        screen.setCursor(WAIT_OFFSET, 3);
        screen.print(F(WAIT));
        break;
    }

    changeAudioInput(id);
    ui_redraw(true);
}

bool _hThreeEntries(byte id)
{
    if (id > 2)
        return false;

    switch (id)
    {
    case 0:
        createMenu(menu_sources, 3, _hSource, F(SOURCE), SOURCE_OFFSET);
        break;
    case 1:
        // TODO
        break;
    case 2:
        // TODO
        break;
    }

    return true;
}

// а ведь это костыль, потому что функция createMenu отказалась принимать функцию с bool вместо void!
void _hTEOnly(byte id)
{
    _hThreeEntries(id);
}

void _hBluetooth(byte id)
{
    if (_hThreeEntries(id))
        return;

    switch (id)
    {
    case 3:
        if (bt_conn_count > 0) 
            bt_sendAT("CD"); // disconnect
        else
            bt_sendAT("CC"); // reconnect
        break;
    case 4:
        break;
    case 5:
        bt_restart();
        break;
    }

    ui_redraw(true);
}

void ui_redraw(bool sb_force)
{
    // перерисовка статусной строки, если надо
    if (screenId > SCREEN_ACTION || sb_force)
    {
        screen.clear(0, 0, 127, 7);
        screen.setCursor(0, 0);

        ui_printPGMLine(pgm_read_word(&(sb_sources[curInput])));
        screen.setCursor(91, 0);
        screen.print(F("%"));
        screen.setCursor(115, 0);
        screen.print(F("'C"));

        setStatusbarIcon();
    }

    /* Отрисовка статичной информации дисплея */
    clearMainArea();
    switch (curInput)
    {
    case SRC_NULL:
        screen.setCursor(NOTHING_OFFSET, 2);
        screen.print(F(NOTHING));
        break;
    case SRC_USB:
        screen.setCursor(AUDIO_ONLY_OFFSET, 2);
        screen.print(F(AUDIO_ONLY));
        break;
    case SRC_BT:
        drawBTLogo();
        break;
    }

    screenId = SCREEN_MAIN;
    ui_refresh();
}

void ui_tick()
{
    /* обработчик ручки управления */
    if (ctrl_state == CTRL_CLICK)
    {
        switch (screenId)
        {
        case SCREEN_MAIN:
            switch (curInput)
            {
            case SRC_BT:
                if (bt_conn_count > 1)
                    bt_sendAT("MA", false); // AT-команда play/pause
                else if (!bt_pairing_mode)
                    bt_gotoPairingMode();
                break;
            }
            break;
        case SCREEN_MENU:
            callMenuHandler();
            break;
        }
    }
    else if (ctrl_state == CTRL_ROTATING)
    {
        switch (screenId)
        {
        case SCREEN_MAIN:
            clearMainArea();
            screen.setCursor(MASTER_VOLUME_OFFSET, 3);
            screen.print(F(MASTER_VOLUME));
            screenId = SCREEN_ACTION;
            // break;
        case SCREEN_ACTION:
            changeVolume(rot_dir);
            drawBar(volMaster, 100, 14, 44);
            actTimer = timer0_millis;
            break;
        case SCREEN_MENU:
            menuRotate(rot_dir);
        }
    }

    if (ctrl_state != CTRL_HOLDING)
        ctrl_state = 0;
    else if (timer0_millis - hold_timer > CTRL_HOLD_TIMEOUT_MS)
    {
        switch (screenId) {
        case SCREEN_MAIN:
            switch (curInput)
            {
            case SRC_NULL:
                createMenu(null_src_menu, 3, _hTEOnly, F(DEVICE_MENU), DEVICE_MENU_OFFSET);
                break;
            case SRC_USB:
                createMenu(null_src_menu, 3, _hTEOnly, F(USB_TITLE), USB_TITLE_OFFSET);
                break;
            case SRC_BT:
                createMenu((bt_conn_count > 0) ? bt_menu_entr : bt_menu_entr_rec, 6, _hBluetooth, F(BT_TITLE), BT_TITLE_OFFSET);
                break;
            }
            break;
        case SCREEN_MENU:
            ui_redraw(true);
            break;
        }
        ctrl_state = 0;
    }
    /* конец обработчика ручки управления */

    if (screenId == SCREEN_ACTION && timer0_millis - actTimer > ACT_AUTOCLOSE_TIMEOUT_MS)
        ui_redraw();
}

void ui_refresh()
{
    if (screenId > SCREEN_ACTION)
        return;

    /* Обновление статусной строки */
    // ЗНАЧКИ!
    setStatusbarIcon(1, ampEnabled);
    
    // значение громкости
    screen.setCursor(73, 0);
    if (volMaster < 100)
        screen.print(' ');
    if (volMaster < 10)
        screen.print(' ');
    screen.print(volMaster);

    // значение температуры
    screen.setCursor(103, 0);
    if (hsTemp < 10 && hsTemp > -1)
        screen.print(' ');
    screen.print(hsTemp > 99 ? 99 : hsTemp);

    /* Обновление основной части экрана, если надо */
    if (screenId > SCREEN_MAIN)
        return;

    switch (curInput)
    {
    /* Информация о Bluetooth-соединении */
    case SRC_BT:
        screen.clear(0, 47, 127, 63); // опять стирать всю инфу с дисплея и рисовать заново... а ведь всего лишь обновилось состояние воспроизведения...
        if (bt_conn_count > 0)
        {
            screen.setCursor(0, 7);
            screen.print(bt_conn_count == 1 ? F(BT_CONNECTING) : (bt_playback_state ? F(">") : F("-")));
            screen.setCursor(109, 7);
            screen.print(bt_conn_count - 1);
            screen.print(F("/2"));
        }
        else
        {
            if (bt_pairing_mode)
            {
                screen.setCursor(BT_PAIRING_OFFSET, 6);
                screen.print(F(BT_PAIRING));
            }
            else
            {
                screen.setCursor(BT_SEARCH_OFFSET, 6);
                screen.print(F(BT_SEARCH));
                screen.setCursor(BT_CLICK_TO_PAIR_OFFSET, 7);
                screen.print(F(BT_CLICK_TO_PAIR));
            }
        }
        break;
    }
}