#ifndef _ltda_h
#define _ltda_h

#include <Arduino.h>
#include <microWire.h>
#include "config.h"
#include "defines.h"
#include "spiregister.h"
#include "microDS18B20.h"
#include "bluetooth.h"
#include "errorHandler.h"

extern byte volMaster;
extern byte curInput;
extern int8_t hsTemp;
extern bool statusRefresh;
extern bool ampEnabled;
extern uint16_t inputVoltage, outLevel;
extern MicroDS18B20 heatsink;

void deviceStatusRefresh();
void setMasterVolume(byte val);
void setMasterVolumeClassic(byte vol);
void changeVolume(bool dir);
bool checkInputAvailability(byte src_id);
void changeAudioInput(byte src_id);
void setAmplifier(bool state);

#endif