#pragma once

#define BATTERY_PROXY_ENABLE PIN_A2
#define BATTERY_PROXY_SDA SDA
#define BATTERY_PROXY_SCL SCL

// #define FAST_AVR  // SMBus lib doesn't support it now

#ifdef FAST_AVR
#define BATTERY_SDA 0xC0
#define BATTERY_SCL 0xC1
#else
#define BATTERY_SDA PIN_A0
#define BATTERY_SCL PIN_A1
#endif
