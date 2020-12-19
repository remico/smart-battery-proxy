#include "proxy.h"
#include <Wire.h>

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
    : m_battery(SBS(BATTERY_ADDRESS_DEFAULT)), m_command(0xFF), m_current(0), m_voltage(0)
{
    setCommand_0x00(0);
    setCommand_0x01(0.1 * designCapacity());
    setCommand_0x02(10);
    setCommand_0x03(0x0001);
    setCommand_0x04(0);
    // setCommand_0x0f();
    // setCommand_0x17();
    // setCommand_0x18();
    // setCommand_0x19();
    // setCommand_0x1a();
    // setCommand_0x1b();
    // setCommand_0x1c();

    Wire.begin(m_battery.address()); // mimic smart battery
    Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);
}

SBSProxy *SBSProxy::instance()
{
    static SBSProxy p;
    return &p;
}

void SBSProxy::onReceive(const int number)
{
    const int datalen = number - 1;
    SBSProxy *self = SBSProxy::instance();

    const CommandCode command = Wire.read();
    self->setCommand(command);

    if (datalen) { // if smth to send
        uint8_t buf[33] = {0};
        for (int i = 0; i < datalen; ++i) {
            buf[i] = Wire.read();
        }
        self->battery().writeBlock(command, buf, datalen);

        // and save locally
        const uint16_t word = buf[0] | buf[1] << 8;
        switch (command) {
        case 0x00:
            self->setCommand_0x00(word);
            break;
        case 0x01:
            self->setCommand_0x01(word);
            break;
        case 0x02:
            self->setCommand_0x02(word);
            break;
        case 0x03:
            self->setCommand_0x03(word);
            break;
        case 0x04:
            self->setCommand_0x04(word);
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
        default:
            break;
        }
    }

    // Serial.print(F("Wire data received: @ len: "));
    // Serial.print(number);
    // Serial.print(" @ ");
    // Serial.print(" " + hex(command));
    // for (int i = 0; i < datalen; ++i) {
    //     Serial.print(" " + hex(buf[i]));
    // }
    // Serial.println();
}

void SBSProxy::onRequest()
{
    SBSProxy *self = SBSProxy::instance();

    Serial.print(F("Requested: "));
    Serial.println(hex(self->command()));

    switch (self->command()) {
    case 0x00:
        self->answerWord(self->command_0x00());
        break;

    case 0x01: // RemainingCapacityAlarm, mAh
        self->answerWord(self->command_0x01());
        break;
    case 0x02: // RemainingTimeAlarm, min
        self->answerWord(self->command_0x02());
        break;

    case 0x03: // BatteryMode
        self->answerWord(self->command_0x03());
        break;

    case 0x04: // AtRate, mA
        self->answerWord(self->command_0x04());
        break;
    case 0x05: // AtRateTimeToFull, min
        self->answerWord(self->command_0x04() > 0 ? 20 : 65535);
        break;
    case 0x06: // AtRateTimeToEmpty, min
        self->answerWord(self->command_0x04() < 0 ? 80 : 65535);
        break;
    case 0x07: // AtRateOK, boolean
        self->answerWord(self->command_0x04() >= 0 ? true : true);
        break;

    case 0x08: // Temperature, 0,1*K
        self->answerWord(3031);
        break;
    case 0x09: // Voltage, mV
        self->answerWord(self->voltage());
        break;
    case 0x0a: // Current, mA
    case 0x0b: // AverageCurrent, mA
    {
        const int16_t current = self->current();
        Wire.write(static_cast<uint8_t>(current));
        Wire.write(static_cast<uint8_t>(current >> 8));
        Serial.print(" => answered: ");
        Serial.println(current);
        break;
    }

    case 0x0c: // MaxError, %
        Wire.write(2);
        Serial.print(" => answered: ");
        Serial.println(2);
        break;
    case 0x0d: // RelativeStateOfCharge, %
    {
        const uint8_t percentage = self->capacity() * static_cast<uint32_t>(100) / self->designCapacity();
        Wire.write(percentage);
        Serial.print(" => answered: ");
        Serial.println(percentage);
        break;
    }
    case 0x0e: // AbsoluteStateOfCharge, %
    {
        const uint8_t percentage = self->capacity() * static_cast<uint32_t>(100) / self->designCapacity();
        Wire.write(percentage + 3);
        Serial.print(" => answered: ");
        Serial.println(percentage + 3);
        break;
    }

    case 0x0f: // RemainingCapacity, mAh
        self->answerWord(self->capacity());
        break;
    case 0x10: // FullChargeCapacity, mAh
        self->answerWord(self->designCapacity());
        break;
    case 0x11: // RunTimeToEmpty, min
        self->answerWord(self->current() < 0 ? 73 : 65535);
        break;
    case 0x12: // AverageTimeToEmpty, min
        self->answerWord(self->current() < 0 ? 70 : 65535);
        break;
    case 0x13: // AverageTimeToFull, min
        self->answerWord(self->current() > 0 ? 20 : 65535);
        break;

    case 0x14: // ChargingCurrent, mA
        self->answerWord(self->alarm() ? 0 : 3800);
        break;
    case 0x15: // ChargingVoltage, mV
        self->answerWord(self->alarm() ? 0 : 12600);
        break;
    case 0x16: // BatteryStatus
        self->answerWord(self->status());
        break;

    case 0x17: // CycleCount
        self->answerWord(9);
        break;

    case 0x18: // DesignCapacity, mAh
        self->answerWord(self->designCapacity());
        break;
    case 0x19: // DesignVoltage, mV
        self->answerWord(10800);
        break;
    case 0x1a: // SpecificationInfo
        self->answerWord(0b00110001);
        break;

    case 0x1b: // ManufactureDate
        self->answerWord(40 * 512 + 12 * 32 + 19);
        break;
    case 0x1c: // SerialNumber
        self->answerWord(54321);
        break;

    case 0x20: // ManufacturerName
        self->answerString("remico");
        break;
    case 0x21: // DeviceName
        self->answerString("MO06062");
        break;
    case 0x22: // DeviceChemistry
        self->answerString("LION");
        break;
    case 0x23:               // ManufacturerData
    case 0x2f:               // Authenticate
        self->answerWord(0); // ignore
        break;

    case 0x3c:               // VoltageCellFour, mV
        self->answerWord(0); // ignore
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
        Serial.print(" => ???");
    }

    self->setCommand(0xFF);
}

void SBSProxy::answerWord(uint16_t word)
{
    Wire.write(reinterpret_cast<uint8_t *>(word), 2);
    Serial.print(" => answered: ");
    Serial.println(word);
}

void SBSProxy::answerString(const char *str)
{
    Serial.print(" => answered: ");
    Serial.println(str);
    Wire.write(reinterpret_cast<const uint8_t *>(str), strlen(str));
}

uint16_t SBSProxy::status() const
{
    const int16_t _current = current();
    const uint16_t _voltage = voltage();

    uint16_t status = BatteryStatusFlags::INITIALIZED;

    if (_current <= 0) {
        status |= BatteryStatusFlags::DISCHARGING;
    }
    if (_current < 0 && capacity() < 0.1 * m_designCapacity) {
        status |= BatteryStatusFlags::REMAINING_CAPACITY_ALARM;
        status |= BatteryStatusFlags::REMAINING_TIME_ALARM;
    }
    if (_current < 0 && capacity() == 0) {
        status |= BatteryStatusFlags::FULLY_DISCHARGED | BatteryStatusFlags::TERMINATE_DISCHARGE_ALARM;
    }
    if (_current >= 0 && capacity() == 100) {
        status |= BatteryStatusFlags::FULLY_CHARGED;
    }
    if (_current > 0 && _voltage > 12600) {
        status |= BatteryStatusFlags::OVER_CHARGED_ALARM | BatteryStatusFlags::TERMINATE_CHARGE_ALARM;
    }

    return status;
}

uint16_t SBSProxy::capacity() const
{
    const uint16_t _voltage = voltage();
    const uint16_t h = 12600;
    const uint16_t l = 9300;
    const int delta = _voltage - l;
    const double k = _voltage >= h ? 100 : static_cast<double>(max(0, delta)) / (h - l);
    return k * designCapacity();
}

bool SBSProxy::alarm() const
{
    const uint16_t _status = status();
    return false;
}
