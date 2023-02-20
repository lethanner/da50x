#include "bluetooth.h"

byte bt_conn_count;
bool bt_playback_state;
bool bt_pairing_mode = true;

// служебные переменные
byte rx_buffer_pos;
byte rx_buff_c_pos;
bool rx_data_ready;
bool rx_packet_start;
bool rx_comm_ok;

char bt_rx_buffer[48];
unsigned long autopair_timer;
extern volatile unsigned long timer0_millis;
extern byte statusRefresh;

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
         * Я потратил целый день для того, чтобы всё-таки пришлось делать этот костыль.
         * Я даже не знаю, кого винить. Меня-г..нокодера, который захотел
         * максимально упростить код для эмуляции асинхронного УАРТа,
         * или же разработчиков блютуз-чипа BK8000L, которые не предусмотрели
         * конкретный байт, обозначающий начало передачи?
         *
         * Как же меня взбесил постоянный приём каких-то мусорных байтов в начало буфера, если модуль
         * возвращает ответ сразу же после передачи на него команды, а он делает это всегда.
         * Короче, это решение подразумевает использование символов от A до Z из таблицы ASCII в качестве
         * стартового байта, а остальной мусор - идёт в мусор. Ну хотя бы работает.
         */
        if ((rx_packet_start || rx_buffer_pos == 0) && (rx_char < 65 || rx_char > 90))
            return;
        // переполнять буфер не будем
        if (rx_buffer_pos > 47)
            return;

        rx_packet_start = false;
        // если приём команды закончился - закрываем конец буфера нуль-терминатором и вешаем флаг
        if (rx_char == '\n' && bt_rx_buffer[rx_buffer_pos - 1] == '\r')
        {
            bt_rx_buffer[rx_buffer_pos - 1] = '\0';
            rx_data_ready = rx_packet_start = true;
            return;
        }

        bt_rx_buffer[rx_buffer_pos] = rx_char;
        rx_buffer_pos++;
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

void bt_update()
{
    rx_comm_ok = false; // ну такое...
    if (!rx_data_ready)
    {
        if (!bt_pairing_mode && timer0_millis - autopair_timer > BT_AUTOCONNECT_TIMEOUT_MS)
            bt_gotoPairingMode();
        return;
    }

    // обработка команд
    if (strcmp(bt_rx_buffer, "ON") == 0 || strcmp(bt_rx_buffer, "OK") == 0)
        rx_comm_ok = true;
    else if (strcmp(bt_rx_buffer, "II") == 0) { // подключение протокола
        bt_conn_count++;
        bt_pairing_mode = true; // костыль... чтобы не срабатывал таймер окончания режима автопоиска
    }
    else if (strcmp(bt_rx_buffer, "IA") == 0 && bt_conn_count > 0) // отключение протокола
    {
        bt_conn_count--;
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
    else if (strcmp(bt_rx_buffer, "MB") == 0) // начало воспроизведения
        bt_playback_state = true;
    else if (strcmp(bt_rx_buffer, "MA") == 0) // остановка воспроизведения
        bt_playback_state = false;

    /*
     * Первое, что пришло в башку при размышлении о том, как принимать
     * следующую команду, если предыдущая ещё не обработана.
     * Этот кусок кода сдвигает остаток буфера к началу (если он есть),
     * стирая уже обработанные данные.
     * 
     * Интересно, как называется такой буфер?
     */
    //Serial.print("B: ");
    //Serial.println(bt_rx_buffer);
    byte _p_length = strlen(bt_rx_buffer) + 1;
    if (_p_length < rx_buffer_pos) {
        memmove(bt_rx_buffer, bt_rx_buffer + _p_length, 48 - _p_length);
        rx_buffer_pos -= _p_length;
    }
    else {
        rx_buffer_pos = 0;
        rx_data_ready = false;
    }

    //Serial.print("A: ");
    //Serial.println(bt_rx_buffer);
    statusRefresh = 2;
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

bool _bt_okWait()
{
    uint32_t resp_timeout = timer0_millis;
    while (timer0_millis - resp_timeout < CHIP_REPLY_TIMEOUT_MS)
    {
        bt_update();
        if (rx_comm_ok)
            break;
    }
    return rx_comm_ok;
}

void bt_sendAT(const char *cmd, bool check)
{
    _bt_printLine("AT+");
    _bt_printLine(cmd);
    _bt_writeChar('\r');
    _bt_writeChar('\n');

    if (check && !_bt_okWait())
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
    rx_comm_ok = bt_pairing_mode = bt_playback_state = false;
    autopair_timer = timer0_millis;
    if (!_bt_okWait())
        terminate(2);
}

void bt_gotoPairingMode()
{
    bt_pairing_mode = true;
    statusRefresh = 2;
    bt_sendAT("CA");
}

void bt_startSPP()
{
    _bt_printLine("APT+SPP8888");
    if (!_bt_okWait())
        terminate(10);
}