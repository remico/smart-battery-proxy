#pragma once

#include "sbs.h"

#define BATTERY_ADDRESS 0x0B

void readBattery(sbs::SBS &battery);
void handleUserInput(sbs::SBS &battery);
