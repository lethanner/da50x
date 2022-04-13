#ifndef _ds1803_h
#define _ds1803_h

#include "microWire.h"
#include "defines.h"
#include "config.h"

#define HNDR_TO_BYTE false
#define BYTE_TO_HNDR true

bool setVolume(byte vol);

#endif