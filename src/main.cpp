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
#include "ds1803.h"
#include "UI.h"
#include "bitmaps.h"
#include "spiregister.h"
#include "bluetooth.h"
#include "amplifier.h"

extern volatile unsigned long timer0_millis;
// глобальные переменные для ручки управления
bool enc_s2;
uint8_t ctrl_state;
uint32_t hold_timer;
// классы
UI ui;

void testHandler(byte selection) {
  
}
char vol_temp = INIT_VOL_HNDR;

void crash(byte code) {
  shifterReset();
  cli();
  ui.setCursor(0, 0);
  ui.print("Error code: ");
  ui.print(code);
  ui.print("\r\nStartup failure.");
  while (1);
}

void setup() {
  // в первую же очередь инициализировать сдвиговый регистр, пока он не включил выводы (по схеме).
  // пины 11, 12, 13 на выход
  DDRB |= 0b00111000;
  shifterReset();
  
  ui.init();
  Wire.setClock(I2C_SPEED_HZ);
  ui.clear();
  ui.drawBitmap(0, 0, logo_128x64, 128, 64);
  Serial.begin(115200);
  Serial.println("DA50X");
  // инициализация порта и прерывания ручки управления
  // пины A0, A1, A2 на вход
  DDRC &= ~0b00000111;
  PCMSK1 |= 0b00000101;
  PCICR |= (1 << 1);

  delay(500);
  bool state = bt_activate();
  if (!state)
    crash(97);
    
  if (!setVolume(vol_temp))
    crash(95);

  setAmplifier(true);
  ui.clear();
}

void loop() {
  // обработка сигналов с ручки управления
  if (ctrl_state > 0) {
    if (ctrl_state == CTRL_CLICK)
      ui.click();
    else if (ctrl_state == CTRL_ROTATING) {
      
      if (enc_s2 && vol_temp < 100)
        vol_temp++;
      else if (vol_temp > 0)
        vol_temp--;

      if (!setVolume(vol_temp))
        crash(10);

      ui.renderScale(vol_temp);
      
      //ui.rotate(enc_s2);
    }
    if (ctrl_state != CTRL_HOLDING)
      ctrl_state = 0;
    else if (timer0_millis - hold_timer > CTRL_HOLD_TIMEOUT_MS) {
      ui.hold();
      ctrl_state = 0;
    }
  }
}

// прерывание PORT CHANGE для ручки управления
ISR(PCINT1_vect) {
  // вращение энкодера, проще простого
  if(!(PINC & 0x01)) {
    enc_s2 = !((PINC >> 1) & 0x01);
    ctrl_state = CTRL_ROTATING;
  }

  // обработка кнопки вместе с удержанием, уже посложнее
  bool btn_state = !((PINC >> 2) & 0x01);
  if (btn_state) {
    hold_timer = timer0_millis;
    ctrl_state = CTRL_HOLDING;
  }
  else if (ctrl_state == CTRL_HOLDING && !btn_state) {
    ctrl_state = CTRL_CLICK;
  }
}