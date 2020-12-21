#include "sbs.h"
#include "Wire.h"

using namespace sbs;

const char command_0x00[] PROGMEM = "ManufacturerAccess";
const char command_0x01[] PROGMEM = "RemainingCapacityAlarm";
const char command_0x02[] PROGMEM = "RemainingTimeAlarm";
const char command_0x03[] PROGMEM = "BatteryMode";
const char command_0x04[] PROGMEM = "AtRate";
const char command_0x05[] PROGMEM = "AtRateTimeToFull";
const char command_0x06[] PROGMEM = "AtRateTimeToEmpty";
const char command_0x07[] PROGMEM = "AtRateOK";
const char command_0x08[] PROGMEM = "Temperature";
const char command_0x09[] PROGMEM = "Voltage";
const char command_0x0a[] PROGMEM = "Current";
const char command_0x0b[] PROGMEM = "AverageCurrent";
const char command_0x0c[] PROGMEM = "MaxError";
const char command_0x0d[] PROGMEM = "RelativeStateOfCharge";
const char command_0x0e[] PROGMEM = "AbsoluteStateOfCharge";
const char command_0x0f[] PROGMEM = "RemainingCapacity";
const char command_0x10[] PROGMEM = "FullChargeCapacity";
const char command_0x11[] PROGMEM = "RunTimeToEmpty";
const char command_0x12[] PROGMEM = "AverageTimeToEmpty";
const char command_0x13[] PROGMEM = "AverageTimeToFull";
const char command_0x14[] PROGMEM = "ChargingCurrent";
const char command_0x15[] PROGMEM = "ChargingVoltage";
const char command_0x16[] PROGMEM = "BatteryStatus";
const char command_0x17[] PROGMEM = "CycleCount";
const char command_0x18[] PROGMEM = "DesignCapacity";
const char command_0x19[] PROGMEM = "DesignVoltage";
const char command_0x1a[] PROGMEM = "SpecificationInfo";
const char command_0x1b[] PROGMEM = "ManufactureDate";
const char command_0x1c[] PROGMEM = "SerialNumber";
const char command_0x20[] PROGMEM = "ManufacturerName";
const char command_0x21[] PROGMEM = "DeviceName";
const char command_0x22[] PROGMEM = "DeviceChemistry";
const char command_0x23[] PROGMEM = "ManufacturerData";
const char command_0x2f[] PROGMEM = "Authenticate";
const char command_0x3c[] PROGMEM = "VoltageCellFour";
const char command_0x3d[] PROGMEM = "VoltageCellThree";
const char command_0x3e[] PROGMEM = "VoltageCellTwo";
const char command_0x3f[] PROGMEM = "VoltageCellOne";
const char command_0x70[] PROGMEM = "ManufacturerInfo";

const char type_minutes[] PROGMEM = "min";
const char type_percent[] PROGMEM = "%";
const char type_string[] PROGMEM = "string";
const char type_unsigned[] PROGMEM = "unsigned";
const char type_flags[] PROGMEM = "flags";
const char type_mAh[] PROGMEM = "mAh";
const char type_mA[] PROGMEM = "mA";
const char type_mV[] PROGMEM = "mV";
const char type_bool[] PROGMEM = "boolean";
const char type_01K[] PROGMEM = "0.1K";
const char type_word[] PROGMEM = "word";
const char type_count[] PROGMEM = "count";
const char type_number[] PROGMEM = "number";

const SBSCommand commandSet[COMMAND_SET_SIZE] PROGMEM = {
    // function code, writable?, size, command name, units
    {0x00, true,  2, command_0x00, type_word},
    {0x01, true,  2, command_0x01, type_mAh},
    {0x02, true,  2, command_0x02, type_minutes},
    {0x03, true,  2, command_0x03, type_flags},
    {0x04, true,  2, command_0x04, type_mA},
    {0x05, false, 2, command_0x05, type_minutes},
    {0x06, false, 2, command_0x06, type_minutes},
    {0x07, false, 2, command_0x07, type_bool},
    {0x08, false, 2, command_0x08, type_01K},
    {0x09, false, 2, command_0x09, type_mV},
    {0x0a, false, 2, command_0x0a, type_mA},
    {0x0b, false, 2, command_0x0b, type_mA},
    {0x0c, false, 1, command_0x0c, type_percent},
    {0x0d, false, 1, command_0x0d, type_percent},
    {0x0e, false, 1, command_0x0e, type_percent},
    {0x0f, true,  2, command_0x0f, type_mAh},
    {0x10, false, 2, command_0x10, type_mAh},
    {0x11, false, 2, command_0x11, type_minutes},
    {0x12, false, 2, command_0x12, type_minutes},
    {0x13, false, 2, command_0x13, type_minutes},
    {0x14, false, 2, command_0x14, type_mA},
    {0x15, false, 2, command_0x15, type_mV},
    {0x16, false, 2, command_0x16, type_flags},
    {0x17, true,  2, command_0x17, type_count},
    {0x18, true,  2, command_0x18, type_mAh},
    {0x19, true,  2, command_0x19, type_mV},
    {0x1a, true,  2, command_0x1a, type_unsigned},
    {0x1b, true,  2, command_0x1b, type_unsigned},
    {0x1c, true,  2, command_0x1c, type_number},
    {0x20, true,  20 + 1, command_0x20, type_string},
    {0x21, true,  20 + 1, command_0x21, type_string},
    {0x22, true,   4 + 1, command_0x22, type_string},
    {0x23, true,  14 + 1, command_0x23, type_string},
    {0x2f, true,  20 + 1, command_0x2f, type_string},
    {0x3c, false, 2, command_0x3c, type_mV},
    {0x3d, false, 2, command_0x3d, type_mV},
    {0x3e, false, 2, command_0x3e, type_mV},
    {0x3f, false, 2, command_0x3f, type_mV},
    {0x70, false, 2, command_0x70, type_string},
};

const SBSCommand *SBS::m_commands = commandSet;

SBS::SBS(uint8_t address)
    : m_smbusAddress(address)
    , m_clkSpeed(40000)
{
    m_bbi2c.bWire = 0; // use bit bang, not wire library
    m_bbi2c.iSDA = BATTERY_SDA;
    m_bbi2c.iSCL = BATTERY_SCL;
    I2CInit(&m_bbi2c, m_clkSpeed);
}

uint8_t SBS::readByte(uint8_t command) const
{
    uint8_t b = 0;
    I2CReadRegister(&m_bbi2c, m_smbusAddress, command, &b, 1, true);
    return b;
}

uint16_t SBS::readWord(uint8_t command) const
{
    uint16_t w = 0;
    I2CReadRegister(&m_bbi2c, m_smbusAddress, command, reinterpret_cast<uint8_t *>(&w), 2, true);
    return w;
}

void SBS::readBlock(uint8_t command, uint8_t *buf, uint8_t size) const
{
    I2CReadRegister(&m_bbi2c, m_smbusAddress, command, buf, size, true);
}

void SBS::writeWord(uint8_t command, uint16_t value) const
{
    uint8_t buf[3] = {command, (uint8_t)value, (uint8_t)(value >> 8)};
    I2CWrite(&m_bbi2c, m_smbusAddress, buf, sizeof(buf));
}

void SBS::writeBlock(uint8_t command, uint8_t *buf, uint8_t size) const
{
    uint8_t data_to_send[32] = {command};
    const int size_limit = min(size, sizeof(data_to_send) - 1);
    memcpy(&data_to_send[1], buf, size_limit);
    I2CWrite(&m_bbi2c, m_smbusAddress, data_to_send, size_limit + 1);
}

SBSCommand SBS::command(const uint8_t idx)
{
    SBSCommand c;
    memcpy_P(&c, &m_commands[idx], sizeof(SBSCommand));
    return c;
}

SBSCommand SBS::command(const String &code)
{
    const unsigned long input = strtol(code.c_str(), 0, 16);
    SBSCommand c;

    for (int i = 0; i < COMMAND_SET_SIZE; ++i) {
        memcpy_P(&c, &m_commands[i], sizeof(SBSCommand));
        if (input == c.code) {
            return c;
        }
    }

    return SBSCommand {0xFF};
}
