#include "RXBuffer.h"

void RXBuffer::write(char ch)
{
    // переполнять буфер не будем
    if (buffer_pos > (RX_BUFFER_LENGTH - 1))
        return;

    // если приём команды закончился - закрываем конец буфера нуль-терминатором и вешаем флаг
    if (ch == '\n' && data[buffer_pos - 1] == '\r')
    {
       data[buffer_pos - 1] = '\0';
       packet_ready = packet_start = true;
       return;
    }
    
    packet_start = false;
    data[buffer_pos] = ch;
    buffer_pos++;
}

void RXBuffer::erasePacket()
{
    if (!packet_ready)
        return;
    /*
     * Первое, что пришло в башку при размышлении о том, как принимать
     * следующую команду, если предыдущая ещё не обработана.
     * Этот кусок кода сдвигает остаток буфера к началу (если он есть),
     * стирая уже обработанные данные.
     * 
     * Интересно, как называется такой буфер?
     */

    // наверно можно было бы сделать кольцевой буфер, но обрабатывать команду,
    // когда один её кусок окажется в его конце, а другой - в начале, как-то...
    // короче, в другой раз. наверное.
    byte _p_length = strlen(data) + 1;
    if (_p_length < buffer_pos) {
        memmove(data, data + _p_length, RX_BUFFER_LENGTH - _p_length);
        buffer_pos -= _p_length;
    }
    else {
        buffer_pos = 0;
        packet_ready = false;
        packet_start = true;
    }
}

void RXBuffer::flush()
{
    packet_start = true;
    packet_ready = false;
    buffer_pos = 0;
    memset(data, 0, RX_BUFFER_LENGTH);
}