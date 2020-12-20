#pragma once

#include "sbs.h"

namespace sbs {

class SBSProxy {
public:
    static SBSProxy *instance();

    SBS &battery() { return m_battery; }

    void answerWord(uint16_t word);
    void answerString(const char *str);

    uint16_t status() const;
    uint16_t capacity() const;

    inline CommandCode command() const { return m_command; }
    inline void setCommand(CommandCode code) { m_command = code; }

    inline uint16_t voltage() const { return m_voltage; }
    inline void setVoltage(uint16_t voltage) { m_voltage = voltage; }

    inline uint16_t cell1() const { return m_cell1; }
    inline void setCell1(uint16_t voltage) { m_cell1 = voltage; }

    inline uint16_t cell2() const { return m_cell2; }
    inline void setCell2(uint16_t voltage) { m_cell2 = voltage; }

    inline uint16_t cell3() const { return m_cell3; }
    inline void setCell3(uint16_t voltage) { m_cell3 = voltage; }

    inline int16_t current() const { return m_current; }
    inline void setCurrent(int16_t current) { m_current = current; }

    inline int16_t currentAverage() const { return m_currentAverage; }
    inline void setCurrentAverage(int16_t current) { m_currentAverage = current; }

    inline uint16_t designCapacity() const { return m_designCapacity; }

    inline uint16_t command_0x00() const { return m_command_0x00; }
    inline void setCommand_0x00(uint16_t value) { m_command_0x00 = value; }
    inline uint16_t command_0x01() const { return m_command_0x01; }
    inline void setCommand_0x01(uint16_t value) { m_command_0x01 = value; }
    inline uint16_t command_0x02() const { return m_command_0x02; }
    inline void setCommand_0x02(uint16_t value) { m_command_0x02 = value; }
    inline uint16_t command_0x03() const { return m_command_0x03 | 0b0000000000000001; }
    inline void setCommand_0x03(uint16_t value) { m_command_0x03 = value & 0b1110001100000000; }
    inline int16_t command_0x04() const { return m_command_0x04; }
    inline void setCommand_0x04(int16_t value) { m_command_0x04 = value; }
    // inline uint16_t command_0x0f() const { return m_command_0x0f; }
    // inline void setCommand_0x0f(uint16_t value) { m_command_0x0f = value; }
    // inline uint16_t command_0x17() const { return m_command_0x17; }
    // inline void setCommand_0x17(uint16_t value) { m_command_0x17 = value; }
    // inline uint16_t command_0x18() const { return m_command_0x18; }
    // inline void setCommand_0x18(uint16_t value) { m_command_0x18 = value; }
    // inline uint16_t command_0x19() const { return m_command_0x19; }
    // inline void setCommand_0x19(uint16_t value) { m_command_0x19 = value; }
    // inline uint16_t command_0x1a() const { return m_command_0x1a; }
    // inline void setCommand_0x1a(uint16_t value) { m_command_0x1a = value; }
    // inline uint16_t command_0x1b() const { return m_command_0x1b; }
    // inline void setCommand_0x1b(uint16_t value) { m_command_0x1b = value; }
    // inline uint16_t command_0x1c() const { return m_command_0x1c; }
    // inline void setCommand_0x1c(uint16_t value) { m_command_0x1c = value; }


private:
    SBSProxy();

    static void onReceive(const int number);
    static void onRequest();

    inline bool alarm() const;

    SBS m_battery;
    CommandCode m_command;

    int16_t m_current;
    int16_t m_currentAverage;

    uint16_t m_voltage;
    uint16_t m_cell1;
    uint16_t m_cell2;
    uint16_t m_cell3;

    const uint16_t m_designCapacity = 4400;

    uint16_t m_command_0x00;
    uint16_t m_command_0x01;
    uint16_t m_command_0x02;
    uint16_t m_command_0x03;
    int16_t m_command_0x04;
    // uint16_t m_command_0x0f;
    // uint16_t m_command_0x17;
    // uint16_t m_command_0x18;
    // uint16_t m_command_0x19;
    // uint16_t m_command_0x1a;
    // uint16_t m_command_0x1b;
    // uint16_t m_command_0x1c;
};

} // namespace sbs
