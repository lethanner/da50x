#include "bluetooth.h"

char bt_rx_buffer[16];
byte rx_buffer_pos = 0, rx_buff_c_pos = 0;
uint32_t buffer_drop_timeout;
uint32_t autopair_timer;
extern volatile unsigned long timer0_millis;

bool _bt_data_ready = false;
bool _bt_command_ok = false;

bool bt_universal_bool = true;
bool bt_playback_state;
bool bt_data_refresh;
byte bt_conn_count;

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
        // ах итерация с 32-битной переменной на 8-бит процессоре, ещё и в моменты, когда нужно побыстрее обработать полученный байт
        buffer_drop_timeout = timer0_millis;

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
        if (rx_buffer_pos == 0 && (rx_char < 65 || rx_char > 90))
            return;
        // переполнять буфер не будем
        if (rx_buffer_pos > 15)
            return;
        //Serial.print(rx_buffer_pos);
        //Serial.write(rx_char);
        // если приём команды закончился - закрываем конец буфера нуль-терминатором и вешаем флаг
        if (rx_char == '\n')
        {
            bt_rx_buffer[rx_buffer_pos - 1] = '\0';
            _bt_data_ready = true;
            //Serial.println(bt_rx_buffer);
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

void bt_tick()
{
    _bt_command_ok = false; // хз какой по счёту костыль
    if (_bt_data_ready)
    {
        if (strcmp(bt_rx_buffer, "ON") == 0 || strcmp(bt_rx_buffer, "OK") == 0)
            _bt_command_ok = true;
        else if (strcmp(bt_rx_buffer, "II") == 0)
        {
            bt_conn_count++;
            bt_universal_bool = true;
        }
        else if (strcmp(bt_rx_buffer, "IA") == 0)
        {
            bt_conn_count--;
            // Да, модуль BK8000L так устроен, что при подключении всех протоколов пошлёт II три раза,
            // а при полном отключении пошлёт IA только два раза. Видимо не хочет показаться осликом.
            if (bt_conn_count == 1)
                bt_conn_count = 0;
        }
        else if (strcmp(bt_rx_buffer, "MB") == 0)
            bt_playback_state = true;
        else if (strcmp(bt_rx_buffer, "MA") == 0)
            bt_playback_state = false;

        rx_buffer_pos = 0;
        _bt_data_ready = false;
        bt_data_refresh = true;
    }

    // сброс буфера приёма данных при отсутствии новых байтов в течение 10 мс
    if (rx_buffer_pos > 0 && timer0_millis - buffer_drop_timeout > 10 && !_bt_data_ready)
        rx_buffer_pos = 0;

    if (!bt_universal_bool && timer0_millis - autopair_timer > BT_AUTO_PAIR_MODE_TIMEOUT_MS)
    {
        bt_enterPairingMode();
        bt_data_refresh = true;
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
    /*
     * Почему не while (!_bt_command_ok)?
     * Потому что цикл какого-то чёрта не останавливается, если флаг ставится в true
     * из прерывания.
    */
    uint32_t resp_timeout = timer0_millis;
    while (timer0_millis - resp_timeout < CHIP_REPLY_TIMEOUT_MS)
    {
        bt_tick();
        if (_bt_command_ok)
            break;
    }
    return _bt_command_ok;
}

// костыль номер 4294967296
bool bt_enterPairingMode()
{
    bt_universal_bool = true;
    return bt_sendAT("CA");
}

bool bt_sendAT(const char *cmd, bool check)
{
    _bt_printLine("AT+");
    _bt_printLine(cmd);
    _bt_writeChar('\r');
    _bt_writeChar('\n');

    if (check)
        return _bt_okWait();
    else
        return true;
}

bool bt_disable()
{
    if (!bt_sendAT("CP"))
        return false;
    delay(100);
    bt_universal_bool = true;
    extWrite(EXT_BT_ENABLE, false);
    return true;
}

bool bt_activate()
{
    extWrite(EXT_BT_ENABLE, false);
    delay(100);
    extWrite(EXT_BT_ENABLE, true);

    bt_conn_count = 0;
    bt_data_refresh = bt_playback_state = bt_universal_bool = false;
    autopair_timer = timer0_millis;

    return _bt_okWait();
}