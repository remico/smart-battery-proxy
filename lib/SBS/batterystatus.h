#pragma once

namespace sbs {

enum BatteryStatusFlags {
    // errors
    // statuses
    FULLY_DISCHARGED    = 0x0010,
    FULLY_CHARGED       = 0x0020,
    DISCHARGING         = 0x0040,
    INITIALIZED         = 0x0080,
    // alarms
    REMAINING_TIME_ALARM        = 0x0100,
    REMAINING_CAPACITY_ALARM    = 0x0200,
    TERMINATE_DISCHARGE_ALARM   = 0x0800,
    OVER_TEMP_ALARM             = 0x1000,
    TERMINATE_CHARGE_ALARM      = 0x4000,
    OVER_CHARGED_ALARM          = 0x8000
};

inline BatteryStatusFlags operator|(BatteryStatusFlags a, BatteryStatusFlags b)
{
    return static_cast<BatteryStatusFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline BatteryStatusFlags operator&(BatteryStatusFlags a, BatteryStatusFlags b)
{
    return static_cast<BatteryStatusFlags>(static_cast<int>(a) & static_cast<int>(b));
}

inline BatteryStatusFlags operator~(BatteryStatusFlags a)
{
    return static_cast<BatteryStatusFlags>(~static_cast<int>(a));
}

// inline BatteryStatusFlags& operator|=(BatteryStatusFlags &a, BatteryStatusFlags b)
// {
//     return a = a | b;
// }

// inline BatteryStatusFlags& operator&=(BatteryStatusFlags &a, BatteryStatusFlags b)
// {
//     return a = a & b;
// }

} // namespace sbs
