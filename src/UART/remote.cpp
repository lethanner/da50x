#include "remote.h"

// ВАЖНО разместить этот инклюд в .cpp, а не в .h, иначе при подключении remote.h
// происходит повторное объявление вектора прерывания и в итоге имеем ошибку линкера
#define MU_TX_BUF 0
#define MU_RX_BUF 4
#define MU_PRINT
#include "MicroUART.h"

MicroUART usb;
RXBuffer usb_rx_buffer;

// char str_ok[] = "OK\r\n";
// char str_er[] = "ER\r\n";
char remote_tx_buf[REMOTE_TX_BUFFER];
//bool tx_data_ready = false;
// byte tx_buf_pos = 0;

void remote_init()
{
    DDRD &= ~_BV(PD7); // D7 как вход (под сигнал DTR)
    PORTD |= _BV(PD7); // подтяжка D7 к питанию
    dtr = !bitRead(PIND, 7);

    usb.begin(115200);
    if (dtr) // DTR чек
        usb.println(F(ID));

    memset(remote_tx_buf, 0, REMOTE_TX_BUFFER);
}

void MU_serialEvent()
{
    char ch = usb.read();
    // команды должны начинаться с символа L, иначе они даже в буфер не будут записаны
    if (usb_rx_buffer.packet_start && ch != 'L')
        return;

    usb_rx_buffer.write(ch);
}

void remote_tick()
{
    if (usb_rx_buffer.packet_ready)
    {
        processRemoteCommand(usb_rx_buffer.data);
        usb_rx_buffer.erasePacket();
    }
}

void processRemoteCommand(const char *cmd)
{
    switch (cmd[1])
    {
    // case 'I': // вывести ID устройства
    //     usb.println(F(ID));
    //     break;
    case 'V': // команды управления громкостью
        switch (cmd[2])
        {
        case 'M': // master
            setMasterVolume((cmd[3] - 1));
            break;
        case 'B': // balance
            setStereoBalance((cmd[4] == 0x02) ? (cmd[3] - 1) : -(cmd[3] - 1));
            break;
        }
        break;
    case 'C':
    { // команды редактирования настроек
        bool flag = (cmd[3] == 0x02);
        switch (cmd[2])
        {
        case 'M': // monitoring
            setMonitoring(flag);
            break;
        case 'O': // dac-only
            setDACOnlyMode(flag);
            break;
        case 'S': // auto switch
            bitWrite(deviceSettings, ALLOW_AUTOSWITCH, flag);
            break;
        case 'Q': // quick volume
            bitWrite(deviceSettings, ALLOW_QUICK_VOLUME, flag);
            break;
        }
        break;
    }
    case 'S': // команда смены источника
        changeAudioInput((byte)(cmd[2] - 1));
        break;
    }
}

void shareDeviceRegisters(byte level)
{
    remote_tx_buf[0] = 'L';
    // во что я вляпался...
    // избегаем нулевого байта в буфере, чтобы не тормозить его чтение...
    remote_tx_buf[1] = currentMasterVolume + 1;
    remote_tx_buf[2] = (hsTemp < 1) ? hsTemp - 1 : hsTemp + 1;
    remote_tx_buf[3] = ampEnabled ? 0x02 : 0x01;
    remote_tx_buf[4] = undervoltage + 1;
    remote_tx_buf[5] = (balance < 1) ? balance - 1 : balance + 1;
    remote_tx_buf[6] = '\0';
    if (level > 1)
    {
        remote_tx_buf[6] = (deviceSettings + 1) & 0xFF; // вот когда будут задействованы все 16 бит этой переменной, тогда будем делить её
        remote_tx_buf[7] = ((inputVoltageADC >> 8) & 0xFF) + 1;
        remote_tx_buf[8] = (inputVoltageADC & 0xFF) + 1;
        remote_tx_buf[9] = bt_conn_count + 1;
        remote_tx_buf[10] = bt_playback_state ? 0x02 : 0x01;
        remote_tx_buf[11] = bt_pairing_mode ? 0x02 : 0x01;
        remote_tx_buf[12] = currentInput + 1;
        remote_tx_buf[13] = '\0';
    }
    remoteBroadcast();
}

void remoteBroadcast(const char *data)
{
    if (data != NULL)
        strcpy(remote_tx_buf, data);
    if (dtr)
        usb.println(remote_tx_buf);

    bt_sendSPP(remote_tx_buf);
}

void printDebug(const char *str)
{
    usb.println(str);
}