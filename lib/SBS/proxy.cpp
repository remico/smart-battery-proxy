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

SBS SBSProxy::m_battery = SBS(BATTERY_ADDRESS_DEFAULT);
uint8_t SBSProxy::m_command = 0xFF;

SBSProxy::SBSProxy()
{
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
    uint8_t buf[33] = {0};
    const int datalen = number - 1;
    m_command = Wire.read();

    for (int i = 0; i < datalen; ++i) {
        buf[i] = Wire.read();
    }

    if (number > 1) {
        // battery()->writeBlock(m_command, buf, datalen);
    }

    Serial.print(F("Wire data received: @ len: "));
    Serial.print(number);
    Serial.print(" @ ");
    Serial.print(" " + hex(m_command));
    for (int i = 0; i < datalen; ++i) {
        Serial.print(" " + hex(buf[i]));
    }
    Serial.println();
}

void SBSProxy::onRequest()
{
    Serial.print(F("Wire command requested: "));
    Serial.println(hex(m_command));

    // if (m_command == 0x09) // command: voltage
    // {
    //     uint16_t voltage = 12041; // mV
    //     Wire.write((uint8_t)voltage);
    //     Wire.write((uint8_t)(voltage >> 8));
    //     Serial.println("Command 0x09 => answered");
    // }
    // m_command = 0xFF;
}
