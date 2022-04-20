/* 
 * Исходный код DA50X, основной файл
 * Полное переписывание альфа-версии.
 * 
 * Работа начата: 27.08.2021
*/
#include <Arduino.h>
#include "LTDA/LTDA.h"
#include "UI/UI.h"

uint32_t debugTimer;
extern volatile unsigned long timer0_millis;
// void terminate(byte err_code)
// {
//   cli();
//   srReset();
//   screen.setCursor(0, 0);
//   screen.print(F(TT_ERROR));
//   screen.print(err_code);
//   while (1)
//     ;
// }

// Инициализация устройства
void setup()
{
  /* инициализация сдвигового регистра */
  DDRB |= 0b00111000;
  srReset();

  /* инициализация дисплея и отображение логотипа */
  disp_initialize();
  screen.drawBitmap(0, 0, logo_128x64, 128, 64);

  Serial.begin(115200);

  /* инициализация датчика температуры */
  heatsink.setResolution(10);
  // сразу запросить данные от датчика, чтобы к концу инициализации аппарата они уже лежали на столе
  heatsink.requestTemp();

  /* инициализация АЦП для измерений */
  // АЦП вкл, ручной режим, прерывания откл, скорость преобразования минимальная (F_CPU / 128, 9.6 кГц)
  ADCSRA = 0b10000111;
  // внутренний reference, стандартный байтовый порядок, канал A6
  ADMUX = 0b11000110;
  // ну тут опять заранее запрашиваем данные, как и с датчиком температуры...
  bitSet(ADCSRA, ADSC);

  /* инициализация модуля bluetooth */
  bt_uart_initialize();
  if (!bt_restart());
    // TODO: terminate
  bt_disable();

  /* инициализация микшерного блока */
  if (!setMasterVolumeClassic(INIT_VOLUME));
    // TODO: terminate

  /* автовыбор доступного источника */
  if (checkInputAvailability(SRC_USB))
    changeAudioInput(SRC_USB);

  /* инициализация порта и прерывания для энкодера */
  DDRC &= ~0b00000111;
  PCMSK1 |= 0b00000101;
  PCICR |= (1 << 1);

  /* включение усилителя */
  setAmplifier(true);

  /* отрисовка главного экрана */
  ui_redraw(true);
}

// Главный цикл программы
void loop()
{
  ui_tick();
  deviceStatusRefresh();
  
  if (timer0_millis - debugTimer > 1500)
  {
    Serial.println();
    if (statusRefresh)
    {
      Serial.println(F("RF"));
    }
    Serial.println(inputVoltage);
    Serial.println(outLevel);
    debugTimer = timer0_millis;
  }

  if (statusRefresh)
  {
    ui_refresh();
    // TODO: remote refresh
    statusRefresh = false;
  }
}