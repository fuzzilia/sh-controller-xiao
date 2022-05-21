#ifndef SH_CONTROLLER_ANALOG_STICK_H
#define SH_CONTROLLER_ANALOG_STICK_H

#include "src/lib/SHValue.h"

void InitPinsForStick();
void RefreshStickValue();
float StickValue(int, TwoDimension dimension);

#endif // SH_CONTROLLER_ANALOG_STICK_H
