#pragma once

#include "Arduino.h"
#include "BitBang_SMBus.h"

#define BATTERY_ADDRESS_DEFAULT 0x0B
#define COMMAND_SET_SIZE 38

namespace sbs {

typedef uint8_t CommandCode;

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

struct SBSCommand {
    CommandCode code;
    bool writeable;
    int size;
    const char *_name;
    const char *_units;

    String name()
    {
        char buf[25] = {0};
        strcpy_P(buf, _name);
        return buf;
    }

    String units()
    {
        char buf[10] = {" "}; // begin with a space
        strcat_P(buf, _units);
        return buf;
    }
};

class SBS {
public:
    SBS(uint8_t address);

    uint8_t readByte(uint8_t command);
    uint16_t readWord(uint8_t command);
    void readBlock(uint8_t command, uint8_t *buf, uint8_t size);
    void writeWord(uint8_t command, uint16_t value);
    void writeBlock(uint8_t command, uint8_t *buf, uint8_t size);

    static SBSCommand command(const uint8_t idx);
    static SBSCommand command(const String &code);

    uint8_t address() const { return m_smbusAddress; }
    uint32_t clkSpeed() const { return m_clkSpeed; }

private:
    static const SBSCommand *m_commands;
    uint8_t m_smbusAddress;
    uint32_t m_clkSpeed;
    BBI2C m_bbi2c = {0};
};

} // namespace sbs
