#include "bluetooth.h"

// переменные состояний модуля
byte bt_conn_count;
bool bt_playback_state;
bool bt_pairing_mode = true;
bool bt_spp_pending;

// переменные и класс для буферизации команд с порта
RXBuffer bt_rx_buffer;
byte rx_buff_c_pos;
// bool rx_comm_ok;

unsigned long autopair_timer, spp_start_delay;
extern volatile unsigned long timer0_millis;
extern byte statusRefresh;

/* Своя реализация программного UART (фикс. 9600 бод) */
ISR(TIMER1_A)
{
    static int rx_char;
    // проверяем состояние бита и пихаем его в мини-буфер
    if ((PIND >> 5) & 0x01)
        rx_char |= (1 << rx_buff_c_pos);
    else
        rx_char &= ~(1 << rx_buff_c_pos);

    rx_buff_c_pos++;
    if (rx_buff_c_pos > 8)
    {
        Timer1.stop();
        PCMSK2 |= _BV(PD5); // возвращаем прерывания для ожидания приёма следующего байта

        // проверка наличия стоп-бита
        if (!((rx_char >> 8) & 0x01))
            return;

        // выделяем только нижний байт
        rx_char &= 0xFF;

        /*
         * Как же меня бесил постоянный приём каких-то мусорных байтов в начало буфера, если модуль
         * возвращает ответ сразу же после передачи на него команды, а он делает это всегда.
         * Это решение подразумевает использование символов от A до Z из таблицы ASCII в качестве
         * стартового байта, а остальной мусор - идёт в мусор. Ну хотя бы работает.
         */
        if (bt_rx_buffer.packet_start && (rx_char < 65 || rx_char > 90))
            return;

        // пихаем в буфер
        bt_rx_buffer.write(rx_char);
    }
}

ISR(PCINT2_vect)
{
    if (!((PIND >> 5) & 0x01))
    {
        PCMSK2 &= ~_BV(PD5); // запрещаем прерывание на пине на время считывания байта, т.к. теперь это дело аппаратного таймера
        rx_buff_c_pos = 0;
        delayMicroseconds(8); // небольшая задержка, чтобы быть чуть поближе к середине "импульса передачи бита"
        Timer1.restart();
    }
}

// софтверная передача байта по UART на скорости примерно 9600 бод.
// примерно.
void _bt_writeChar(char c)
{
    uint8_t oldSREG = SREG;
    cli();
    BT_TX_LOW;
    for (byte i = 0; i < 8; i++)
    {
        // ДИЛЭЙ!!! ДА! ОН!
        // и это тоже можно переделать на аппаратный таймер!
        delayMicroseconds(102);
        if ((c >> i) & 0x01)
            BT_TX_HIGH;
        else
            BT_TX_LOW;
    }
    delayMicroseconds(101);
    BT_TX_HIGH;
    SREG = oldSREG;
    delayMicroseconds(101);
}

void _bt_printLine(const char s[])
{
    byte pos = 0;
    while (s[pos] != '\0')
        _bt_writeChar(s[pos++]);
}
/* Конец реализации программного UART */

bool _bt_checkOK()
{
    bool rx_comm_ok = false;
    uint32_t resp_timeout = timer0_millis;
    while (timer0_millis - resp_timeout < CHIP_REPLY_TIMEOUT_MS)
    {
        /*
         * таки да, переменная packet_ready должна была быть volatile-типа.
         * в кой-то веки хватило ума до этого додуматься... +1 в копилку опыта ардуино-разработок.
         *
         * тип volatile практически обязателен, если переменная
         * будет меняться из прерывания. а я раньше гадал, какого фига,
         * если какая-нибудь булева переменная проверялась в цикле или его условии,
         * этот цикл никак не менял своё поведение при изменении этой переменной
         * из прерывания.
         */
        if (!bt_rx_buffer.packet_ready)
            continue;

        if (strcmp(bt_rx_buffer.data, "OK") == 0 || strcmp(bt_rx_buffer.data, "ON") == 0)
            rx_comm_ok = true;

        bt_rx_buffer.erasePacket();
        break;
    }
    return rx_comm_ok;
}

void bt_update()
{
    if (!bt_rx_buffer.packet_ready)
    {
        if (!bt_pairing_mode && timer0_millis - autopair_timer > BT_AUTOCONNECT_TIMEOUT_MS)
            bt_gotoPairingMode();
        if (bt_spp_pending && bt_conn_count > 1 && timer0_millis - spp_start_delay > 500)
        {
            bt_spp_pending = false;
            // а зачем модулю BK8000L вообще нужен ПИН-КОД на включение SPP?
            _bt_printLine("APT+SPP8888\r\n");
            if (!_bt_checkOK())
                terminate(11);
        }
        return;
    }

    // обработка команд
    if (strcmp(bt_rx_buffer.data, "II") == 0)
    {                                              // подключение протокола
        if (bt_conn_count == 1 && !bt_spp_pending) // подготовка к запуску SPP по подключению к устройству
        {
            spp_start_delay = timer0_millis;
            bt_spp_pending = true;
        }
        bt_conn_count++;
        bt_pairing_mode = true; // костыль... чтобы не срабатывал таймер окончания режима автопоиска
    }
    else if (strcmp(bt_rx_buffer.data, "IA") == 0 && bt_conn_count > 0) // отключение протокола
    {
        bt_conn_count--;
        // да блин, надо! иначе срабатывает фидбэк bt_sendSPP() уже после отключения устройства
        // костыль наверно.
        spp_start_delay = timer0_millis;
        bt_spp_pending = true;
        // Да, модуль BK8000L так устроен, что при подключении всех протоколов пошлёт II три раза,
        // а при полном отключении пошлёт IA только два раза. Видимо, не хочет показаться осликом.
        if (bt_conn_count == 1)
            bt_conn_count = 0;
        /*
         * А кста, нафига там if bt_conn_count > 0?
         * А всё потому, что при попытке подключения бт устройства с отключенным
         * протоколом громкой связи модуль сначала дважды посылает IA, хотя
         * никаких соединений до этого установлено не было. Итого беззнаковая переменная
         * из нуля обращается в 254, и мы ВНЕЗАПНО получаем число 254 на экране
         * вместо счётчика подключенных протоколов. Назову это "феноменом ошибки 254".
         */
    }
    else if (strcmp(bt_rx_buffer.data, "MB") == 0) // начало воспроизведения
        bt_playback_state = true;
    else if (strcmp(bt_rx_buffer.data, "MA") == 0) // остановка воспроизведения
        bt_playback_state = false;

    statusRefresh = 2;

    // а также данных с порта Bluetooth SPP
    if (strncmp(bt_rx_buffer.data, "APR+", 4) == 0)
    {
        statusRefresh = 0;                           // в таком случае принудительный апдейт данных не нужен
        processRemoteCommand(bt_rx_buffer.data + 4); // если будет надо, то statusRefresh обновится здесь
    }

    bt_rx_buffer.erasePacket();
}

void bt_sendAT(const char *cmd, bool check)
{
    _bt_printLine("AT+");
    _bt_printLine(cmd);
    _bt_writeChar('\r');
    _bt_writeChar('\n');

    if (check && !_bt_checkOK())
        terminate(10);
}

void bt_disable()
{
    bt_sendAT("CP");
    delay(100);
    extWrite(EXT_BT_ENABLE, false);
    bt_pairing_mode = true; // пока что костыль, чтобы таймер pairing mode не срабатывал при bt_update(), если модуль отключен.
}

void bt_restart()
{
    extWrite(EXT_BT_ENABLE, false);
    delay(100);
    extWrite(EXT_BT_ENABLE, true);

    bt_conn_count = 0;
    bt_spp_pending = bt_pairing_mode = bt_playback_state = false;
    autopair_timer = timer0_millis;
    if (!_bt_checkOK())
        terminate(2);
}

void bt_gotoPairingMode()
{
    bt_pairing_mode = true;
    statusRefresh = 2;
    bt_sendAT("CA");
}

void bt_sendSPP(const char *data)
{
    if (bt_conn_count < 2 || bt_spp_pending)
        return;

    _bt_printLine("APT+");
    _bt_printLine(data);
    _bt_writeChar('\r');
    _bt_writeChar('\n');

    /*
     * Стадии фикса ПОСТОЯННО возникающей ошибки 12.
     * 1. Замучаться от неё, потому что она вообще почти не позволяет пользоваться блютузом.
     * 2. Взять USB-TTL и врезать его RX'ом параллельно линии передачи данных между МК и BK8000L.
     * 3. Попытаться отловить байты, из-за которых возникает ошибка.
     * 4. Понять, что как бы ты не старался, отловить ошибку под пристальным взглядом через USB-TTL не получается.
     * 5. Забить и поменять ошибку на обычный варнинг.
     * 6. ???
     * 7. PROFIT!
     */
    if (!_bt_checkOK())
        warning(12);
}