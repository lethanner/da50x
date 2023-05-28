#ifndef _ltda_h
#define _ltda_h

#define ALLOW_AUTOSWITCH 0
#define ENABLE_MONITORING 1
#define DAC_ONLY_MODE 2
#define ALLOW_QUICK_VOLUME 3

#include <Arduino.h>
#include <microWire.h>
#include "config.h"
#include "defines.h"
#include "spiregister.h"
#include "microDS18B20.h"
#include "bluetooth.h"
#include "errorHandler.h"

extern byte currentMasterVolume;
extern byte currentInput;
extern int8_t hsTemp;
extern byte statusRefresh;
extern bool ampEnabled;
extern uint16_t inputVoltageADC;
extern MicroDS18B20 heatsink;
extern int deviceSettings;
extern byte undervoltage;
extern int8_t balance;

void hardware_tick();
void setMasterVolume(byte vol);
void changeVolume(bool dir, bool quick = false);
bool checkInputAvailability(byte src_id);
void changeAudioInput(byte src_id);
void setAmplifier(bool state);
void setMonitoring(bool state);
void setDACOnlyMode(bool state = true);
void setStereoBalance(int8_t val);
uint16_t readDeviceVcc();

#endif