#pragma once

#include "sbs.h"
#include "sbsproxy.h"

void readBattery(const sbs::SBS &battery);
void handleUserInput(sbs::SBSProxy *proxy);
void humanizeBatteryStatus(uint16_t batteryStatus);
