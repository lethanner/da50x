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
    this->setCursor(0, 1);
    for (byte line = 0; line < (menuEntryCount + 1); line++) {
        char linebuffer[42];
        uint16_t ptr = pgm_read_word(&(menuEntries[line]));
        uint8_t charid = 0;

        do {
          linebuffer[charid] = (char)(pgm_read_byte(ptr++));
        } while (linebuffer[charid++] != NULL);

        this->invertText(line == menuChooseId);
        this->println(linebuffer);
        this->invertText(0);
    }
}

void UI::createMenu(const char *title, const char* const *entries, void (*handler)(byte)) {
    menuChooseId = 0;
    menuEntryCount = sizeof(entries);
    menuHandler = handler;
    menuEntries = entries;

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

void UI::rotate(bool dir) { // true - вправо
    if (dir && menuChooseId < menuEntryCount)
        menuChooseId++;
    else if (!dir && menuChooseId > 0)
        menuChooseId--;

    this->renderMenuEntries();
}