/*
 * Исходный код DA50X, основной файл
 * Полное переписывание альфа-версии.
 *
 * Работа начата: 27.08.2021
 */
#include <Arduino.h>
#include "HW/hardware.h"
#include "UART/remote.h"
#include "UI/UI.h"

// Инициализация устройства
void setup()
{
  /* инициализация сдвигового регистра */
  DDRB |= 0b00111000;
  srReset();

  /* инициализация удаленного управления */
  remote_init();

  /* инициализация дисплея и отображение логотипа */
  if (!disp_initialize())
  {
    while (1)
    {
      // Выбросить и никогда не использовать.
      // Немедленно заменить одной строкой после того, как будет доделана вся функция setIndicator.
      setIndicator(true);
      delay(250);
      setIndicator(false);
      delay(250);
      setIndicator(true);
      delay(250);
      setIndicator(false);
      delay(750);
    }
  }
  screen.drawBitmap(0, 0, logo_128x64, 128, 64);

  /* инициализация датчика температуры */
  heatsink.setResolution(10);
  // сразу запросить данные от датчика, чтобы к концу инициализации аппарата они уже лежали на столе
  heatsink.requestTemp();

  delay(500);
  setIndicator(true);

  /* инициализация АЦП для измерений */
  // АЦП вкл, ручной режим, прерывания откл, скорость преобразования минимальная (F_CPU / 128, 9.6 кГц)
  ADCSRA = 0b10000111;
  // стандартный reference, стандартный байтовый порядок, канал A6
  ADMUX = 0b01000110;
  // ну тут опять заранее запрашиваем данные, как и с датчиком температуры...
  bitSet(ADCSRA, ADSC);

  /* инициализация модуля bluetooth */
  bt_uart_initialize();
  bt_restart();
  bt_disable();

  /* инициализация микшерного блока */
  setMasterVolume(INIT_VOLUME);

  /* инициализация порта и прерывания для энкодера */
  DDRC &= ~0b00000111;
  PCMSK1 |= 0b00000101;
  PCICR |= (1 << 1);

  /* включение усилителя */
  // не надо, этот вопрос решает функция changeAudioInput ниже

  /* настройка порта обнаружения подключения USB + его автовключение */
  DDRB &= ~0x01;
  if (checkInputAvailability(SRC_USB))
    changeAudioInput(SRC_USB);
  // PCMSK0 = 0x01;
  // PCICR |= 0x01;

  /* включение индикатора мониторинга */
  setMonitoring(true);

  /* отрисовка главного экрана */
  // уж лучше сделать это через statusRefresh
  statusRefresh = 3;
  setIndicator(false);
}

// Главный цикл программы
void loop()
{
  hardware_tick();
  ui_tick();
  remote_tick();

  if (statusRefresh > 0)
  {
    if (statusRefresh == 1) // онли статусбар
      ui_refresh(false);
    else if (statusRefresh == 2) // апдейт существующего содержимого на экране
      ui_refresh();
    else if (statusRefresh == 3) // фулл перерисовка
      ui_redraw(true);

    shareDeviceRegisters(statusRefresh);
  }
  statusRefresh = 0;
}