/* 
 * Исходный код DA50X, основной файл
 * Полное переписывание альфа-версии.
 * 
 * Работа начата: 27.08.2021
*/
#include <Arduino.h>
#include "UI/UI.h"
#include "UI/bitmaps.h"
#include "spiregister.h"
#include "amplifier.h"
#include "microDS18B20.h"

MicroDS18B20 heatsink(A3);

// Инициализация устройства
void setup()
{
  // пины 11, 12, 13 на выход
  DDRB |= 0b00111000;
  // в первую же очередь инициализировать сдвиговый регистр
  shifterReset();
  Serial.begin(115200);
  // иниц. дисплея, а вместе с ним и шины I2C
  disp_initialize();
  screen.drawBitmap(0, 0, logo_128x64, 128, 64);
  delay(500);

  ctrl_initialize();
  heatsink.setResolution(10);
  heatsink.requestTemp();
  // инициализировать модуль Bluetooth
  bt_initialize();
  if (!bt_activate())
    terminate(97);
  bt_disable();
  if (!setVolume(INIT_VOL_HNDR))
    terminate(54);  
  PORTD &= ~_BV(PD2);
  if ((PIND >> 2) & 0x01)
    pendingSourceId = 1;

  setAmplifier(true);
  screen.clear();
}

// Главный цикл программы
void loop()
{
  static uint32_t hsink_refresh_timer = 0;
  if (timer0_millis - hsink_refresh_timer > 1000) {
    heatsink_temp = heatsink.getTemp();
    heatsink.requestTemp();
    hsink_refresh_timer = timer0_millis;
  }

  ctrl_update();
  main_tick();
  statusbar_tick();
  bt_tick();
}