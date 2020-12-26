#include "sbsproxy.h"
#include <Wire.h>

#define abs_16(x) abs(static_cast<int16_t>(x))

#define VOLTAGE_LIMIT_HIGH 12600
#define VOLTAGE_LIMIT_LOW 9300
#define CAPACITY_DESIGN 2500

#define CHARGING_CURRENT (CAPACITY_DESIGN / 7)
#define CHARGING_VOLTAGE 12600

using namespace sbs;

namespace {

const uint16_t max_disbalance_mV = 200;
const uint16_t max_cell_voltage_mV = 4200;
const uint16_t charging_threshold_percentage = 80;

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

    updateBatteryData();
}

void SBSProxy::updateBatteryData()
{
    uint16_t temperature = battery().readWord(0x08); // ignore now

    uint16_t voltage = battery().readWord(0x09);
    setVoltage(voltage);

    setCell1(battery().readWord(0x3f));
    setCell2(battery().readWord(0x3e));
    setCell3(battery().readWord(0x3d));

    int16_t current = battery().readWord(0x0a);
    setCurrent(current);

    int16_t averageCurrent = battery().readWord(0x0b);
    setCurrentAverage(averageCurrent);
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

    considerStatusFlag(BatteryStatusFlags::OVER_CHARGED_ALARM,
                    _current > 0 && _voltage > VOLTAGE_LIMIT_HIGH);

    considerStatusFlag(BatteryStatusFlags::TERMINATE_CHARGE_ALARM,
                    (_current > 0 && _voltage > VOLTAGE_LIMIT_HIGH) || !chargingAllowed());

    return m_status;
}

void SBSProxy::printPowerStats()
{
    // updateBatteryData();  don't uncomment: i2c (Wire) ISR can interfere with it and break data

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

    // ======================================
    // cells voltage disbalance
    const uint16_t d12 = abs_16(cell1() - cell2());
    const uint16_t d23 = abs_16(cell2() - cell3());
    const uint16_t d31 = abs_16(cell3() - cell1());

    // print disbalance
    Serial.print(F("Cells voltage disbalance: "));
    Serial.print(F(" cells 1-2 - "));
    Serial.print(d12);
    Serial.print(F(", cells 2-3 - "));
    Serial.print(d23);
    Serial.print(F(", cells 3-1 - "));
    Serial.println(d31);

    if ((cell1() > max_cell_voltage_mV) || (cell2() > max_cell_voltage_mV) || (cell3() > max_cell_voltage_mV)) {
        Serial.println(F("[WW] Cells overvoltage"));
    }

    Serial.print(F("[II] CHARGING: "));
    if (!chargingAllowed()) {
        Serial.print(F("not "));
    }
    Serial.print(F("permitted, "));
    if (status() & BatteryStatusFlags::DISCHARGING) {
        Serial.print(F("in"));
    }
    Serial.println(F("active"));
    Serial.println();

    // ======================================
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
    uint16_t minutes = static_cast<double>(remainingCapacity()) / abs_16(atRate()) * 60;
    return atRate() < 0 ? minutes : 65535;
}

uint16_t SBSProxy::atRateTimeToFull() const
{
    uint16_t minutes = static_cast<double>(CAPACITY_DESIGN - remainingCapacity()) / abs_16(atRate()) * 60;
    return atRate() > 0 ? minutes : 65535;
}

uint16_t SBSProxy::remainingCapacity() const
{
    // capacity simulation for simplicity (based on voltage values only)
    const uint16_t _voltage = voltage();
    const uint16_t h = VOLTAGE_LIMIT_HIGH;
    const uint16_t l = VOLTAGE_LIMIT_LOW;
    const double delta_to_low_limit = _voltage <= l ? 0 : _voltage - l;  // prevent values below 0
    const double k = _voltage >= h ? 1 : delta_to_low_limit / (h - l);  // prevent values above 1
    return k * CAPACITY_DESIGN;
}

uint8_t SBSProxy::relativeStateOfCharge() const
{
    const uint8_t percentage = remainingCapacity() * static_cast<uint32_t>(100) / CAPACITY_DESIGN;
    return min(100, percentage);  // 0..100%
}

uint16_t SBSProxy::runTimeToEmpty() const
{
    uint16_t minutes = static_cast<double>(remainingCapacity()) / abs_16(current()) * 60;
    return current() < 0 ? minutes : 65535;
}

uint16_t SBSProxy::averageTimeToEmpty() const
{
    uint16_t minutes = static_cast<double>(remainingCapacity()) / abs_16(currentAverage()) * 60;
    return currentAverage() < 0 ? minutes : 65535;
}

uint16_t SBSProxy::averageTimeToFull() const
{
    uint16_t minutes = static_cast<double>(CAPACITY_DESIGN - remainingCapacity()) / abs_16(currentAverage()) * 60;
    return currentAverage() > 0 ? minutes : 65535;
}

bool SBSProxy::chargingAllowed() const
{
    static bool was_fully_charged = true; // assume charged by default

    if (!was_fully_charged) {
        was_fully_charged = m_status & BatteryStatusFlags::FULLY_CHARGED;  // !!! use m_status, not status()
    }

    if (was_fully_charged) {
        // don't start charging until the battery gets discharged below the threshold
        if (relativeStateOfCharge() > charging_threshold_percentage) {
            return false;
        } else {
            was_fully_charged = false;
        }
    }

    // cells voltage disbalance
    const uint16_t d12 = abs_16(cell1() - cell2());
    const uint16_t d23 = abs_16(cell2() - cell3());
    const uint16_t d31 = abs_16(cell3() - cell1());

    if (d12 > max_disbalance_mV || d23 > max_disbalance_mV || d31 > max_disbalance_mV) {
        was_fully_charged = relativeStateOfCharge() > charging_threshold_percentage; // temporary solution
        return false;
    }

    // individual cell overcharging
    if ((cell1() > max_cell_voltage_mV) || (cell2() > max_cell_voltage_mV) || (cell3() > max_cell_voltage_mV)) {
        was_fully_charged = relativeStateOfCharge() > charging_threshold_percentage; // temporary solution
        return false;
    }

    return true;
}

void SBSProxy::considerStatusFlag(BatteryStatusFlags flag, bool set_condition) const
{
    if (set_condition) {
        m_status = m_status | flag;
    } else {
        m_status = m_status & ~(flag);
    }
}
