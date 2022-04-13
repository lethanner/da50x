#ifndef _config_h
#define _config_h

/* Пользовательские настройки */
//#define RUSSIAN
#define CTRL_HOLD_TIMEOUT_MS 500
#define ACT_AUTOCLOSE_TIMEOUT_MS 3000
#define BT_AUTO_PAIR_MODE_TIMEOUT_MS 10000
#define DISP_AUTO_DIMM_TIMEOUT_MS 15000
#define ENC_DEBOUNCE_DELAY_MS 50
#define INIT_VOL_HNDR 25

/* Аппаратные настройки */
// адреса модулей
#define DIGIPOT_MASTER_I2C 0b0101000 // все пины адреса на минус
#define DIGIPOT_SUB_I2C 0b0101100    // A2 на плюс, остальное на минус
#define SSD1306_DISP_I2C 0x3C
// таймауты и попытки перезапуска модуля
#define CHIP_REPLY_TIMEOUT_MS 3000
#define REQUEST_RETRIES_COUNT 3

// настройки подогнаны под аппаратные требования, НЕОБДУМАННОЕ ИЗМЕНЕНИЕ ЗАПРЕЩЕНО!
#define I2C_SPEED_HZ 400000L

#endif