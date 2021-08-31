#define MAIN_SCREEN 0
#define ACTION_SCREEN 1
#define MENU_SCREEN 2
#define SUBMENU_SCREEN 3

//#define STATUSBAR_ALLOWED disp_id < MENU_SCREEN

#define USE_MICRO_WIRE
#include <GyverOLED.h>
class LTUI: public GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> {
    public:
        LTUI();
        unsigned char disp_id = 0;
        void renderScale(uint8_t value);
        void renderMenu(const char* title);
        void renderQuestionScreen(const char* title, const char* text, const char* f_entry, const char* s_entry);
        // Действия ручки управления
        void click();
        void hold();
        void turn(bool dir);
    private:
        //void *menuSelectAction;
        uint16_t clickCount, holdCount;
};