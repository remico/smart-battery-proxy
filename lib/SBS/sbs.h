#pragma once

#include "Arduino.h"
#include "BitBang_SMBus.h"

#define COMMAND_SET_SIZE 38

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

    uint8_t readByte(uint8_t command);
    uint16_t readWord(uint8_t command);
    void readString(char *buf, uint8_t size, uint8_t command);
    void writeWord(uint8_t command, uint16_t value);

    static SBSCommand command(const uint8_t idx);
    static SBSCommand command(const String &code);

private:
    static const SBSCommand *m_commands;
    uint8_t m_smbusAddress;
    BBI2C m_bbi2c = {0};
};

} // namespace sbs
