#pragma once

#include "sbs.h"

namespace sbs {

class SBSProxy {
public:
    static SBSProxy *instance();

    const SBS &battery() const { return m_battery; }

    bool isLoggingEnabled() const { return m_logging; }
    void enableLogging(bool enable) { m_logging = enable; }

    void enableBattery(bool enable);

    BatteryStatusFlags status() const;
    void printPowerStats();

private:
    void answerWord(uint16_t word);
    void answerString(const char *str);

    inline uint16_t manufacturerAccess() const { return m_manufacturerAccess; }
    inline void setManufacturerAccess(uint16_t value) { m_manufacturerAccess = value; }

    inline uint16_t remainingCapacityAlarm() const { return m_remainingCapacityAlarm; }
    inline void setRemainingCapacityAlarm(uint16_t value) { m_remainingCapacityAlarm = value; }

    inline uint16_t remainingTimeAlarm() const { return m_remainingTimeAlarm; }
    inline void setRemainingTimeAlarm(uint16_t value) { m_remainingTimeAlarm = value; }

    inline uint16_t batteryMode() const { return m_batteryMode | 0b0000000000000001; }
    inline void setBatteryMode(uint16_t value) { m_batteryMode = value & 0b1110001100000000; }

    inline int16_t atRate() const { return m_atRate; }
    inline void setAtRate(int16_t value) { m_atRate = value; }

    uint16_t atRateTimeToEmpty() const;
    uint16_t atRateTimeToFull() const;
    bool atRateOk() const { return atRate() >= 0 ? true : true; }

    uint16_t remainingCapacity() const;
    uint8_t relativeStateOfCharge() const;

    uint16_t runTimeToEmpty() const;
    uint16_t averageTimeToEmpty() const;
    uint16_t averageTimeToFull() const;

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

private:
    SBSProxy();

    void updateBatteryData();

    static void onReceive(const int number);
    static void onRequest();

    bool chargingAllowed() const;
    void considerStatusFlag(BatteryStatusFlags flag, bool set_condition) const;

    SBS m_battery;
    CommandCode m_command;

    mutable volatile BatteryStatusFlags m_status;

    int16_t m_current;
    int16_t m_currentAverage;

    uint16_t m_voltage;
    uint16_t m_cell1;
    uint16_t m_cell2;
    uint16_t m_cell3;

    uint16_t m_manufacturerAccess;
    uint16_t m_remainingCapacityAlarm;
    uint16_t m_remainingTimeAlarm;
    uint16_t m_batteryMode;
    int16_t m_atRate;

    volatile bool m_logging;
};

} // namespace sbs
