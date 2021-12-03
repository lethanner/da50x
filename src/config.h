#ifndef _config_h
#define _config_h

// пользовательские настройки
//#define RUSSIAN
#define CTRL_HOLD_TIMEOUT_MS 500
#define CHIP_REPLY_TIMEOUT_MS 3000
#define INIT_VOL_HNDR 25

// адреса модулей
#define DIGIPOT_MASTER_I2C 0b0101000 // все пины адреса на минус
#define DIGIPOT_SUB_I2C 0b0101100 // A2 на плюс, остальное на минус

// настройки подогнаны под аппаратные требования, НЕОБДУМАННОЕ ИЗМЕНЕНИЕ ЗАПРЕЩЕНО!
#define I2C_SPEED_HZ 400000L
#define SOFTUART_BIT_TX_US 104
#define SOFTUART_BIT_RX_US 104

#endif