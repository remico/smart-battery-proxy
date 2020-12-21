#include "sbsproxy.h"
#include <Wire.h>

#define VOLTAGE_LEVEL_HIGH 12500
#define VOLTAGE_LEVEL_LOW 9300
#define CAPACITY_DESIGN 2500

#define CHARGING_CURRENT (CAPACITY_DESIGN / 2)
#define CHARGING_VOLTAGE 12600

using namespace sbs;

namespace {

String hex(int n)
{
    char buf[10] = {0};
    sprintf(buf, "0x");
    if (n < 16) {
        sprintf(buf + strlen(buf), "%d", 0);
    }
    sprintf(buf + strlen(buf), "%x", n);
    return String(buf);
}

} // namespace

SBSProxy::SBSProxy()
    : m_battery(SBS(BATTERY_ADDRESS_DEFAULT))
    , m_command(0xFF)
    , m_status(BatteryStatusFlags::INITIALIZED)
    , m_current(0)
    , m_currentAverage(0)
    , m_voltage(0)
    , m_cell1(0)
    , m_cell2(0)
    , m_cell3(0)
    , m_logging(false)
{
    setManufacturerAccess(0);
    setRemainingCapacityAlarm(0.1 * CAPACITY_DESIGN);
    setRemainingTimeAlarm(10);
    setBatteryMode(0x0001);
    setAtRate(0);

    // for mimic smart battery
    Wire.begin(m_battery.address());

    // disable internal pull-ups
    digitalWrite(BATTERY_PROXY_SDA, LOW);
    digitalWrite(BATTERY_PROXY_SCL, LOW);

    Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);
}

SBSProxy *SBSProxy::instance()
{
    static SBSProxy p;
    return &p;
}

void SBSProxy::enableBattery(bool enable)
{
    pinMode(BATTERY_PROXY_ENABLE, OUTPUT);
    digitalWrite(BATTERY_PROXY_ENABLE, LOW);
}

void SBSProxy::onReceive(const int number)
{
    const int datalen = number - 1;
    SBSProxy *self = SBSProxy::instance();

    const CommandCode command = Wire.read();
    self->setCommand(command);

    if (self->m_logging) {
        Serial.print(F("[RC] "));
        Serial.println(hex(command));
    }

    if (datalen) { // if smth to write
        uint8_t buf[32] = {0};
        for (int i = 0; i < datalen; ++i) {
            buf[i] = Wire.read();
        }
        self->battery().writeBlock(command, buf, datalen);

        // and save values locally
        const uint16_t word = buf[0] | buf[1] << 8;
        switch (command) {
        case 0x00:
            self->setManufacturerAccess(word);
            break;
        case 0x01:
            self->setRemainingCapacityAlarm(word);
            break;
        case 0x02:
            self->setRemainingTimeAlarm(word);
            break;
        case 0x03:
            self->setBatteryMode(word);
            break;
        case 0x04:
            self->setAtRate(word);
            break;
        default:
            break;
        }
    } else {
        // track actual charging/discharging state
        switch (command) {
        case 0x09: {
            uint16_t voltage = self->battery().readWord(command);
            self->setVoltage(voltage);

            // voltage per cell
            self->setCell1(self->battery().readWord(0x3f));
            self->setCell2(self->battery().readWord(0x3e));
            self->setCell3(self->battery().readWord(0x3d));
            break;
        }
        case 0x0a: {
            int16_t current = self->battery().readWord(command);
            self->setCurrent(current);
            break;
        }
        case 0x0b: {
            int16_t averageCurrent = self->battery().readWord(command);
            self->setCurrentAverage(averageCurrent);
            break;
        }
        default:
            break;
        }
    }
}

void SBSProxy::onRequest()
{
    SBSProxy *self = SBSProxy::instance();

    if (self->m_logging) {
        Serial.print(F("  [RQ] "));
        Serial.println(hex(self->command()));
    }

    switch (self->command()) {
    case 0x00: // ManufacturerAccess
        self->answerWord(self->manufacturerAccess());
        break;

    case 0x01: // RemainingCapacityAlarm, mAh
        self->answerWord(self->remainingCapacityAlarm());
        break;
    case 0x02: // RemainingTimeAlarm, min
        self->answerWord(self->remainingTimeAlarm());
        break;

    case 0x03: // BatteryMode
        self->answerWord(self->batteryMode());
        break;

    case 0x04: // AtRate, mA
        self->answerWord(self->atRate());
        break;
    case 0x05: // AtRateTimeToFull, min
        self->answerWord(self->atRateTimeToFull());
        break;
    case 0x06: // AtRateTimeToEmpty, min
        self->answerWord(self->atRateTimeToEmpty());
        break;
    case 0x07: // AtRateOK, boolean
        self->answerWord(self->atRateOk());
        break;

    case 0x08: // Temperature, 0,1*K
        self->answerWord(3031); // ~30*C
        break;
    case 0x09: // Voltage, mV
        self->answerWord(self->voltage());
        break;
    case 0x0a: // Current, mA
        self->answerWord(self->current());
        break;
    case 0x0b: // AverageCurrent, mA
        self->answerWord(self->currentAverage());
        break;

    case 0x0c: // MaxError, %
        Wire.write(2);
        break;
    case 0x0d: // RelativeStateOfCharge, %
        Wire.write(self->relativeStateOfCharge());
        break;
    case 0x0e: // AbsoluteStateOfCharge, %
        Wire.write(self->relativeStateOfCharge() + 3);
        break;

    case 0x0f: // RemainingCapacity, mAh
        self->answerWord(self->remainingCapacity());
        break;
    case 0x10: // FullChargeCapacity, mAh
        self->answerWord(CAPACITY_DESIGN);
        break;
    case 0x11: // RunTimeToEmpty, min
        self->answerWord(self->runTimeToEmpty());
        break;
    case 0x12: // AverageTimeToEmpty, min
        self->answerWord(self->averageTimeToEmpty());
        break;
    case 0x13: // AverageTimeToFull, min
        self->answerWord(self->averageTimeToFull());
        break;

    case 0x14: // ChargingCurrent, mA
        self->answerWord(self->chargingAllowed() ? CHARGING_CURRENT : 0);
        break;
    case 0x15: // ChargingVoltage, mV
        self->answerWord(self->chargingAllowed() ? CHARGING_VOLTAGE : 0);
        break;
    case 0x16: // BatteryStatus, flags
        self->answerWord(self->status());
        break;

    case 0x17: // CycleCount
        self->answerWord(9);
        break;

    case 0x18: // DesignCapacity, mAh
        self->answerWord(CAPACITY_DESIGN);
        break;
    case 0x19: // DesignVoltage, mV
        self->answerWord(10800);
        break;
    case 0x1a: // SpecificationInfo
        self->answerWord(0b00110001);
        break;

    case 0x1b: // ManufactureDate
        self->answerWord(40 * 512 + 12 * 32 + 19);  // Y-1980/M/D
        break;
    case 0x1c: // SerialNumber
        self->answerWord(0xBEAF);
        break;

    case 0x20: // ManufacturerName
        self->answerString(PSTR("+remico"));
        break;
    case 0x21: // DeviceName
        self->answerString(PSTR("+MEGA328"));
        break;
    case 0x22: // DeviceChemistry
        self->answerString(PSTR("+LION"));
        break;
    case 0x23:               // ManufacturerData
    case 0x2f:               // Authenticate
        self->answerWord(0); // ignore
        break;
    case 0x70:  // ManufacturerInfo
        self->answerString(PSTR("+<-- manufacturer info -->"));
        break;

    case 0x3c:               // VoltageCellFour, mV
        self->answerWord(0);
        break;
    case 0x3d: // VoltageCellThree, mV
        self->answerWord(self->cell3());
        break;
    case 0x3e: // VoltageCellTwo, mV
        self->answerWord(self->cell2());
        break;
    case 0x3f: // VoltageCellOne, mV
        self->answerWord(self->cell1());
        break;

    default:
        Serial.println(F("  => ???"));
    }

    self->setCommand(0xFF);  // forget previous command
}

BatteryStatusFlags SBSProxy::status() const
{
    const int16_t _current = current();
    const uint16_t _voltage = voltage();

    considerStatusFlag(BatteryStatusFlags::DISCHARGING,
                    _current <= 0);

    considerStatusFlag(BatteryStatusFlags::REMAINING_CAPACITY_ALARM,
                    remainingCapacityAlarm() && remainingCapacity() < remainingCapacityAlarm());
    // TODO: consider calculating realistic time values instead of using capacity
    considerStatusFlag(BatteryStatusFlags::REMAINING_TIME_ALARM,
                    remainingTimeAlarm() && remainingCapacity() < remainingCapacityAlarm());

    considerStatusFlag(BatteryStatusFlags::FULLY_DISCHARGED,
                    remainingCapacity() >= 0 && remainingCapacity() < 0.2 * CAPACITY_DESIGN);

    considerStatusFlag(BatteryStatusFlags::TERMINATE_DISCHARGE_ALARM,
                    _current < 0 && remainingCapacity() == 0);

    considerStatusFlag(BatteryStatusFlags::FULLY_CHARGED,
                    _current >= 0 && relativeStateOfCharge() >= 100);

    considerStatusFlag(BatteryStatusFlags::OVER_CHARGED_ALARM | BatteryStatusFlags::TERMINATE_CHARGE_ALARM,
                    _current > 0 && _voltage > VOLTAGE_LEVEL_HIGH);

    return m_status;
}

void SBSProxy::printPowerStats()
{
    Serial.print(F("Voltage: "));
    Serial.print(voltage());
    Serial.print(F(" [ "));
    Serial.print(cell1());
    Serial.print(F(", "));
    Serial.print(cell2());
    Serial.print(F(", "));
    Serial.print(cell3());
    Serial.println(F(" ]"));
    Serial.print(F("Current: "));
    Serial.print(current());
    Serial.print(F(" @ average: "));
    Serial.println(currentAverage());
    Serial.println();

    Serial.print(F("Remaining capacity: "));
    Serial.println(remainingCapacity());
    Serial.print(F("RelativeStateOfCharge: "));
    Serial.println(relativeStateOfCharge());
    Serial.println();

    Serial.print(F("RunTimeToEmpty: "));
    Serial.println(runTimeToEmpty());
    Serial.print(F("AverageTimeToEmpty: "));
    Serial.println(averageTimeToEmpty());
    Serial.print(F("AverageTimeToFull: "));
    Serial.println(averageTimeToFull());
    Serial.println();

}

void SBSProxy::answerWord(uint16_t word)
{
    uint8_t buf[2] = {static_cast<uint8_t>(word), static_cast<uint8_t>(word >> 8)};
    Wire.write(buf, 2);
}

void SBSProxy::answerString(const char *str)
{
    uint8_t buf[32] = {0};
    strcpy_P(reinterpret_cast<char *>(buf), str);
    Wire.write(buf, sizeof(buf));
}

uint16_t SBSProxy::atRateTimeToEmpty() const
{
    uint16_t minutes = static_cast<double>(remainingCapacity()) / abs(atRate()) * 60;
    return atRate() < 0 ? minutes : 65535;
}

uint16_t SBSProxy::atRateTimeToFull() const
{
    uint16_t minutes = static_cast<double>(CAPACITY_DESIGN - remainingCapacity()) / abs(atRate()) * 60;
    return atRate() > 0 ? minutes : 65535;
}

uint16_t SBSProxy::remainingCapacity() const
{
    const uint16_t _voltage = voltage();
    const uint16_t h = VOLTAGE_LEVEL_HIGH;
    const uint16_t l = VOLTAGE_LEVEL_LOW;
    const int delta = _voltage - l;
    const double k = _voltage >= h ? 1 : static_cast<double>(max(0, delta)) / (h - l);
    return k * CAPACITY_DESIGN;
}

uint8_t SBSProxy::relativeStateOfCharge() const
{
    const uint8_t percentage = remainingCapacity() * static_cast<uint32_t>(100) / CAPACITY_DESIGN;
    return percentage;
}

uint16_t SBSProxy::runTimeToEmpty() const
{
    uint16_t minutes = static_cast<double>(remainingCapacity()) / abs(current()) * 60;
    return current() < 0 ? minutes : 65535;
}

uint16_t SBSProxy::averageTimeToEmpty() const
{
    uint16_t minutes = static_cast<double>(remainingCapacity()) / abs(currentAverage()) * 60;
    return currentAverage() < 0 ? minutes : 65535;
}

uint16_t SBSProxy::averageTimeToFull() const
{
    uint16_t minutes = static_cast<double>(CAPACITY_DESIGN - remainingCapacity()) / abs(currentAverage()) * 60;
    return currentAverage() > 0 ? minutes : 65535;
}

bool SBSProxy::chargingAllowed() const
{
    // const uint16_t _status = status();

    // // cells unbalanced
    // if ((abs(m_cell1 - m_cell2) > 100) || (abs(m_cell2 - m_cell3) > 100) || (abs(m_cell3 - m_cell1) > 100)) {
    //     return true;
    // }

    // // individual cell overcharging
    // if ((m_cell1 > 4200) || (m_cell2 > 4200) || (m_cell3 > 4200)) {
    //     return true;
    // }


    return true;  // FIXME: false prevents charging
}

void SBSProxy::considerStatusFlag(BatteryStatusFlags flag, bool set_condition) const
{
    if (set_condition) {
        m_status = m_status | flag;
    } else {
        m_status = m_status & ~(flag);
    }
}
