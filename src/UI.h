#define MAIN_SCREEN 0
#define ACTION_SCREEN 1
#define MENU_SCREEN 2
#define SUBMENU_SCREEN 3

//#define STATUSBAR_ALLOWED disp_id < MENU_SCREEN

#define USE_MICRO_WIRE
#include <GyverOLED.h>
class UI: public GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> {
    public:
        UI();
        byte disp_id = 0;
        void renderScale(uint8_t value);
        void createMenu(const char *title, const char* const *entries, void (*handler)(byte));
        //void renderQuestionScreen(const char* title, const char* text, const char* f_entry, const char* s_entry);
        // Действия ручки управления
        void click();
        void hold();
        void rotate(bool dir);
    private:
        byte strlen_unicode(const char *text);
        void renderMenuEntries();
        void (*menuHandler)(byte);
        const char* const *menuEntries;
        byte menuChooseId;
        byte menuEntryCount;
};