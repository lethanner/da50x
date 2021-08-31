/* 
 * Исходный код DA50X, основной файл
 * Полное переписывание альфа-версии.
 * 
 * Работа начата: 27.08.2021
*/

#include <Arduino.h>
#include "freememory.h"
#include "LTUI.h"
#include "bitmaps.h"
//#include <spiregister.h>

bool ctrl_update, hold_flag;
uint8_t ctrl_cache;
uint32_t hold_timer;
LTUI ui;

void setup() {
  Serial.begin(115200);
  Serial.println(memoryFree());
  ui.init();
  ui.clear();
  Serial.println(memoryFree());
  ui.drawBitmap(0, 0, logo_128x64, 128, 64);

  Serial.println(memoryFree());
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  PCMSK1 |= 0b00000101;
  PCICR |= (1 << 1);
  Serial.println(memoryFree());
  delay(1000);
  ui.clear();
}

void loop() {
  if (ctrl_update) {
    // кнопка
    static bool button_last_state;
    bool btn_state = !((ctrl_cache >> 2) & 0x01);
    if (btn_state != button_last_state) {
      if (btn_state) {
        hold_timer = millis();
        hold_flag = true;
      } else if (hold_flag && !btn_state) {
        ui.click();
        hold_flag = false;
      }
      button_last_state = btn_state;
    } else { // энкодер
      static uint8_t count;
      uint8_t enc_state = ctrl_cache & 0x03;
      if (enc_state == 0b11) count++;
      else if (enc_state == 0b01) count--;
      count = constrain(count, 0, 100);
      ui.renderScale(count);
    }
    ctrl_update = false;
  }

  // обработка удержания кнопки
  if (hold_flag && millis() - hold_timer > 500) {
    ui.hold();
    hold_flag = false;
  }
}

ISR(PCINT1_vect) {
  ctrl_cache = PINC;
  ctrl_update = true;
}