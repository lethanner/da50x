#include <LTUI.h>

LTUI::LTUI() {}

void LTUI::renderScale(uint8_t value) {
    
}

void LTUI::click() {
    clickCount++;
    this->setCursor(0, 8);
    this->print(clickCount);
}

void LTUI::hold() {
    holdCount++;
    this->setCursor(50, 8);
    this->print(holdCount);
}