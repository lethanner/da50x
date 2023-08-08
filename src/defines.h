#ifndef _defines_h
#define _defines_h

// текстовое обозначение состояний обработчика ручки управления
#define CTRL_CLICK 1
#define CTRL_HOLDING 2
#define CTRL_ROTATING 3

// ну что тут говорить? тут нечего говорить.
#define SCREEN_MAIN 0
#define SCREEN_ACTION 1
#define SCREEN_MENU 2
#define SCREEN_MENU_ACT 3

#define ACTION_DEBUG 0
#define ACTION_BALANCE 1
#define ACTION_STATS 2

// выводы сдвигового регистра
#define EXT_BT_ENABLE 7
#define EXT_USB_ENABLE 6
#define EXT_INDICATOR 5
#define EXT_MON_ENABLE 2
#define EXT_AMP_STANDBY 0
#define EXT_AMP_MUTE 1

// источники под id'ами
#define SRC_NULL 0
#define SRC_USB 1
#define SRC_BT 2
#define SRC_FM 3
#define SRC_TF 4
#define SRC_UDISK 5

#endif