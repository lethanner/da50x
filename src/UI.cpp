#include <UI.h>

UI::UI() {}

// спасибо чуваку с форума arduino.ru за реализацию strlen() для юникода
// http://arduino.ru/forum/programmirovanie/pomogite-razobratsya-chto-za-glyuk-co-stringlength#comment-313068
byte UI::strlen_unicode(const char *text) {
    byte i, len = 0;
    while (text[i++] != '\0') {
        len++;
        if (text[i] < 0)
            i++;
    }
    return len;
}

void UI::renderScale(uint8_t value) {
    // начальные координаты: 14, 45, высота столбика: 5
    if (value > 0) // заполненная часть шкалы
        this->rect(14, 45, value + 14, 50, OLED_FILL);
    if (value < 100) // пустая часть
        this->rect(value + 14, 45, 113, 50, OLED_STROKE);
}

void UI::renderMenuEntries() {
    const byte endLine = ((menuEntryCount < 7) ? menuEntryCount + 1 : 7) + menuEntryRendererStartId;

    uint8_t currentLine = 1;
    for (byte line = menuEntryRendererStartId; line < endLine; line++) {
        char linebuffer[42];
        uint16_t ptr = pgm_read_word(&(menuEntries[line]));
        uint8_t charid = 0;

        byte flagId = 0;
        bool mark = false;
        if ((char)(pgm_read_byte(ptr)) == '?') {
            ptr++;
            mark = true;
            flagId = pgm_read_byte(ptr++) - 65;
        }

        do {
          linebuffer[charid] = (char)(pgm_read_byte(ptr++));
        } while (linebuffer[charid++] != NULL);

        this->setCursor(0, currentLine);
        this->invertText(line == menuChooseId);
        this->print(linebuffer);
        this->invertText(0);

        if (mark) {
            this->setCursor(120, currentLine);
            if ((*menuBoolsCache >> flagId) & 0x01) {
                this->print('+');
            } else {
                this->print('-');
            }
        }
        currentLine++;
    }
}

void UI::createMenu(const char *title, const char* const *entries, byte entryCount, void (*handler)(byte), int* boolsCache) {
    menuChooseId = 0, menuVisibleSelId = 0, menuEntryRendererStartId = 0;
    menuEntryCount = entryCount - 1;
    menuHandler = handler;
    menuEntries = entries;
    menuBoolsCache = boolsCache;

    this->clear();

    this->rect(0, 0, 127, 7, OLED_FILL);
    this->setCursor(64 - ((this->strlen_unicode(title) * 6) / 2), 0);
    this->invertText(1);
    this->print(title);
    this->invertText(0);

    this->renderMenuEntries();
}

void UI::click() {
    menuHandler(menuChooseId);
}

void UI::hold() {
    
}

void UI::rotate(bool dir) {
    // if (dir) { // вправо
    //     if (menuVisibleSelId < 6) {
    //         if (menuVisibleSelId < menuEntryCount)
    //             menuVisibleSelId++;
    //         else
    //             return;
    //     } else {
    //         if (menuEntryRendererStartId + 6 < menuEntryCount) {
    //             menuEntryRendererStartId++;
    //             this->clearMainArea();
    //         } else
    //             return;
    //     }
    // } else { // влево
    //     if (menuVisibleSelId > 0)
    //         menuVisibleSelId--;
    //     else {
    //         if (menuEntryRendererStartId > 0) {
    //             menuEntryRendererStartId--;
    //             this->clearMainArea();
    //         } else
    //             return;
    //     }
    // }

    // menuChooseId = menuEntryRendererStartId + menuVisibleSelId;
    // this->renderMenuEntries();
}