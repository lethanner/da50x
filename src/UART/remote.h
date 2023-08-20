// НАКОНЕЦ-ТО!!! Удалённое управление с ПК
#ifndef _remote_h
#define _remote_h

#define REMOTE_TX_BUFFER 14

#include <Arduino.h>
#include "RXBuffer.h"
#include "HW/hardware.h"
#include "config.h"

void remote_init();
void processRemoteCommand(const char *cmd);
void remote_tick();
void remoteBroadcast(const char *data = NULL);
void shareDeviceRegisters(byte level);

void printDebug(const char *str);

#endif