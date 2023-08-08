#include "UI.h"

// переменные для ручки управления
bool rot_dir;
uint8_t ctrl_state;
uint32_t hold_timer;

uint32_t actTimer;
uint32_t lastVolumeChangeTimer;
uint8_t temperatureBlink = 0;

// прерывание PORT CHANGE для ручки управления
ISR(PCINT1_vect)
{
    // вращение энкодера, проще простого
    if (!(PINC & 0x01) && ctrl_state == 0)
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

    reactivateDisplay();
}

void _hSource(byte id)
{
    if (!checkInputAvailability(id))
        return;

    if (id == currentInput)
    {
        ui_redraw(true);
        return;
    }

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
}

void _hSettings(byte id)
{
    switch (id)
    {
    case 0: // autoswitch
        bitToggle(deviceSettings, ALLOW_AUTOSWITCH);
        break;
    case 1: // monitoring
        setMonitoring(!(bitRead(deviceSettings, ENABLE_MONITORING)));
        break;
    case 2: // LPF
        // TODO...
        break;
    case 3: // balance
        initializeMenuAction(F(BALANCE), BALANCE_OFFSET, ACTION_BALANCE);
        setStereoBalance(balance); // весьма костыльно, это лишь ради срабатывания отрисовки сразу при открытии меню
        break;
    case 4: // DAC-only
        setDACOnlyMode(!(bitRead(deviceSettings, DAC_ONLY_MODE)));
        break;
    case 5: // quick volume
        bitWrite(deviceSettings, ALLOW_QUICK_VOLUME, !bitRead(deviceSettings, ALLOW_QUICK_VOLUME));
        break;
    case 6: // stats
        initializeMenuAction(F(STATISTICS), STATISTICS_OFFSET, ACTION_STATS, 500);
        screen.setCursor(0, 1); screen.print(F(STATS_BDATE)); screen.print(F(" " __DATE__ " " __TIME__));
        screen.setCursor(0, 2); screen.print(F(STATS_UPTIME));
        screen.setCursor(0, 3); screen.print(F(STATS_VOLTAGE));
        break;
    case 7: // Debug
        initializeMenuAction(F("Debug"), 49, ACTION_DEBUG, 500);
        screen.setCursor(0, 1);
        screen.print(F("VOLTAGE ADC:"));
        break;
    }
}

bool _hThreeEntries(byte id)
{
    if (id > 3)
        return false;

    switch (id)
    {
    case 0: // source
        createMenu(menu_sources, 3, _hSource, F(SOURCE), SOURCE_OFFSET);
        break;
    case 1: // settings
        createMenu(settings_menu, 8, _hSettings, F(SETTINGS), SETTINGS_OFFSET, false, &deviceSettings);
        break;
    case 2: // shutdown
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
    case 4:
        if (bt_conn_count > 0)
            bt_sendAT("CD"); // disconnect
        else
            bt_sendAT("CC"); // reconnect
        break;
    case 5:
        // TODO: clear pairs
        break;
    case 6:
        bt_restart();
        break;
    }

    ui_redraw(true);
}

void ui_redraw(bool hard)
{
    // хард рэдроу.
    if (screenId > SCREEN_ACTION || hard)
    {
        screen.clear(0, 0, 127, 7);
        screen.setCursor(0, 0);

        ui_printPGMLine(pgm_read_word(&(sb_sources[currentInput])));
        
        screen.setCursor(91, 0);
        screen.print(F("%"));
        screen.setCursor(115, 0);
        screen.print(F("'C"));

        setStatusbarIcon();
        reactivateDisplay();
    }

    /* Отрисовка статичной информации дисплея */
    clearMainArea();
    switch (currentInput)
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
            switch (currentInput)
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
            if (bitRead(deviceSettings, DAC_ONLY_MODE))
            {
                screen.setCursor(DAC_ONLY_VOLUME_REJECT_OFFSET, 3);
                screen.print(F(DAC_ONLY_VOLUME_REJECT));
                screen.setCursor(DAC_ONLY_VOLUME_REJECT_2_OFFSET, 4);
                screen.print(F(DAC_ONLY_VOLUME_REJECT_2));
            }
            else {
                screen.setCursor(MASTER_VOLUME_OFFSET, 3);
                screen.print(F(MASTER_VOLUME));
            }
            screenId = SCREEN_ACTION;
            // break;
        case SCREEN_ACTION:
            actTimer = timer0_millis;
            if (bitRead(deviceSettings, DAC_ONLY_MODE))
                break;
            changeVolume(rot_dir, (bitRead(deviceSettings, ALLOW_QUICK_VOLUME) && (timer0_millis - lastVolumeChangeTimer < QUICK_VOLUME_ACTIVATE_MS)));
            drawBar(currentMasterVolume, 100, 14, 44);
            lastVolumeChangeTimer = timer0_millis;
            break;
        case SCREEN_MENU:
            menuRotate(rot_dir);
            break;
        case SCREEN_MENU_ACT:
            switch (actionId)
            {
                case ACTION_BALANCE:
                    setStereoBalance(rot_dir ? balance + 1 : balance - 1);
                    break;
            }
            break;
        }
    }

    if (ctrl_state != CTRL_HOLDING)
        ctrl_state = 0;
    else if (timer0_millis - hold_timer > CTRL_HOLD_TIMEOUT_MS)
    {
        switch (screenId)
        {
        case SCREEN_MAIN:
            switch (currentInput)
            {
            case SRC_NULL:
                createMenu(null_src_menu, 3, _hTEOnly, F(DEVICE_MENU), DEVICE_MENU_OFFSET);
                break;
            case SRC_USB:
                createMenu(null_src_menu, 3, _hTEOnly, F(USB_TITLE), USB_TITLE_OFFSET);
                break;
            case SRC_BT:
                createMenu((bt_conn_count > 0) ? bt_menu_entr : bt_menu_entr_rec, 7, _hBluetooth, F(BT_TITLE), BT_TITLE_OFFSET);
                break;
            }
            break;
        case SCREEN_MENU:
            ui_redraw(true);
            break;
        case SCREEN_MENU_ACT:
            ui_redraw(true);
            break;
        }
        ctrl_state = 0;
    }
    /* конец обработчика ручки управления */

    if (screenId == SCREEN_ACTION && timer0_millis - actTimer > ACT_AUTOCLOSE_TIMEOUT_MS)
        ui_redraw();

    /* Моргание индикатором и значением температуры, если она выше предела варнинга */
    // TODO: можно и не моргать, если перегретый усилок отключен
    if (hsTemp > TEMP_MAX_WARNING)
    {
        if (timer0_millis - blinkTimer > 1000) {
            if (temperatureBlink < 2)
                temperatureBlink = 2;
            else
                temperatureBlink = 1;

            setIndicator((bool)(temperatureBlink - 1));
            ui_refresh(false);
            reactivateDisplay();
            blinkTimer = timer0_millis;
        }
    }
    else if (temperatureBlink > 0)
    {
        temperatureBlink = 0;
        setIndicator(false);
        ui_refresh(false);
    }

    if (screenId == SCREEN_MENU_ACT && actionRefreshRate > 0)
    {
        if (timer0_millis - actionRefreshTimer > actionRefreshRate)
        {
            switch (actionId)
            {
                case ACTION_DEBUG:
                    screen.setCursor(78, 1);
                    screen.print(inputVoltageADC);
                    
                    reactivateDisplay();
                    break;
                case ACTION_STATS:
                    uint16_t voltage = readDeviceVcc();
                    screen.setCursor(STATS_UPTIME_OFFSET, 2);
                    printTimeValue((timer0_millis / 1000 / 60 / 60) % 24); screen.print(':');
                    printTimeValue((timer0_millis / 1000 / 60) % 60); screen.print(':');
                    printTimeValue((timer0_millis / 1000) % 60);

                    screen.setCursor(STATS_VOLTAGE_OFFSET, 3);
                    screen.print(voltage / 1000); screen.print('.');
                    screen.print(voltage % 1000); screen.print(F("V   "));
                    break;
            }
            actionRefreshTimer = timer0_millis;
        }
    }

    dimmDisplay();
}

void ui_refresh(bool fullRefresh)
{
    /* Для перерисовки SCREEN_MENU_ACT по обновлению данных на хардвейре */
    if (screenId == SCREEN_MENU_ACT)
    {
        switch (actionId)
        {
            case ACTION_BALANCE:
                byte x = (balance > 9 || balance < 0) ? 58 : 61; // если 2 или 1 знак в значении, то выравниваем соответствующим образом
                x = (balance < -9) ? 55 : x; // а это для 3-х знаков, когда появляется минус вместе с двумя знаками
                screen.setCursor(55, 3);
                screen.print(F("   ")); // ааааа я вынужден это делать, чтобы остатков символов не было на экране
                screen.setCursor(x, 3);
                screen.print(balance);
                drawBar(balance, 50, 64, 44, true);
        }
        return;
    }
    else if (screenId == SCREEN_MENU)
        return;

    /* Для SCREEN_ACTION и SCREEN_MAIN */

    /* Обновление статусной строки */
    setStatusbarIcon(1, ampEnabled, (bool)undervoltage);
    setStatusbarIcon(2, checkInputAvailability(SRC_USB));

    if (!bitRead(deviceSettings, DAC_ONLY_MODE)) {
        // значение громкости
        screen.setCursor(73, 0);
        if (currentMasterVolume < 100)
            screen.print(' ');
        if (currentMasterVolume < 10)
            screen.print(' ');
        screen.print(currentMasterVolume);

        // значение температуры
        screen.setCursor(103, 0);
        if (temperatureBlink == 1)
            screen.print("  ");
        else
        {
            if (hsTemp < 10 && hsTemp > -1)
                screen.print(' ');
            screen.print(hsTemp > 99 ? 99 : hsTemp);
        }
    }
    else
    {
        screen.setCursor(DAC_ONLY_MARKER_OFFSET, 0);
        screen.print(F(DAC_ONLY_MARKER));
    }
    
    /* Обновление основной части экрана, если надо */
    // пхах, коммент при прошлом коммите забыл.
    // здесь была грубейшая ошибка, которую я заметил только в самом коде, а не при использовании
    // аппарата. Это условие выполнялось с точностью, да наоборот.
    if (!fullRefresh || screenId > SCREEN_MAIN)
        return;

    switch (currentInput)
    {
    /* Информация о Bluetooth-соединении */
    case SRC_BT:
        screen.clear(0, 51, 127, 63); // опять стирать всю инфу с дисплея и рисовать заново... а ведь всего лишь обновилось состояние воспроизведения...
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