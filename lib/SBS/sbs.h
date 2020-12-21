#pragma once

#include "Arduino.h"
#include "batterystatus.h"
#include "pins.h"
#include "BitBang_SMBus.h"

#define BATTERY_ADDRESS_DEFAULT 0x0B
#define COMMAND_SET_SIZE 39

namespace sbs {

typedef uint8_t CommandCode;

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

    uint8_t readByte(uint8_t command) const;
    uint16_t readWord(uint8_t command) const;
    void readBlock(uint8_t command, uint8_t *buf, uint8_t size) const;
    void writeWord(uint8_t command, uint16_t value) const;
    void writeBlock(uint8_t command, uint8_t *buf, uint8_t size) const;

    static SBSCommand command(const uint8_t idx);
    static SBSCommand command(const String &code);

    uint8_t address() const { return m_smbusAddress; }
    uint32_t clkSpeed() const { return m_clkSpeed; }

private:
    static const SBSCommand *m_commands;
    uint8_t m_smbusAddress;
    uint32_t m_clkSpeed;
    mutable BBI2C m_bbi2c = {0};
};

} // namespace sbs
