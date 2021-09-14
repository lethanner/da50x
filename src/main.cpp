/* 
 * Исходный код DA50X, основной файл
 * Полное переписывание альфа-версии.
 * 
 * Работа начата: 27.08.2021
*/

#include <Arduino.h>
#include "config.h"
#include "defines.h"
#include "localization.h"
//#include "freememory.h"
#include "UI.h"
#include "bitmaps.h"
//#include <spiregister.h>

// глобальные переменные для ручки управления
bool enc_s2;
uint8_t ctrl_state;
uint32_t hold_timer;
// классы
UI ui;

void emptyHandler(byte selection) {

}

void testHandler(byte selection) {
  switch (selection) {
    case 0:
      ui.createMenu("Рус. яз.", namea, emptyHandler);
      break;
    case 1:
      ui.createMenu("Test submenu", nameb, emptyHandler);
      break;
  }
}

void setup() {
  ui.init();
  Wire.setClock(I2C_SPEED_HZ);
  ui.clear();
  ui.drawBitmap(0, 0, logo_128x64, 128, 64);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  PCMSK1 |= 0b00000101;
  PCICR |= (1 << 1);
  delay(1000);
  //ui.clear();
  ui.createMenu("Menu", names, testHandler);
}

void loop() {
  // обработка сигналов с ручки управления
  if (ctrl_state > 0) {
    if (ctrl_state == CTRL_CLICK)
      ui.click();
    else if (ctrl_state == CTRL_ROTATING) {
      ui.rotate(enc_s2);
    }
    if (ctrl_state != CTRL_HOLDING)
      ctrl_state = 0;
    else if (millis() - hold_timer > CTRL_HOLD_TIMEOUT_MS) {
      ui.hold();
      ctrl_state = 0;
    }
  }

}

// прерывание PORT CHANGE для ручки управления
ISR(PCINT1_vect) {
  // вращение энкодера, проще простого
  if(!digitalRead(A0)) {
    enc_s2 = !digitalRead(A1);
    ctrl_state = CTRL_ROTATING;
  }

  // обработка кнопки вместе с удержанием, уже посложнее
  bool btn_state = !digitalRead(A2);
  if (btn_state) {
    hold_timer = millis();
    ctrl_state = CTRL_HOLDING;
  }
  else if (ctrl_state == CTRL_HOLDING && !btn_state) {
    ctrl_state = CTRL_CLICK;
  }
}