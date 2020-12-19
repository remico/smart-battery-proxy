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
uint16_t SBSProxy::m_word = 0;

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
        battery().writeBlock(m_command, buf, datalen);
    } else {
        switch (m_command) {
        case 0x0c:
        case 0x0d:
        case 0x0e:
            m_word = battery().readByte(m_command);
            break;
        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x2f:
            break;
        case 0x08:
        case 0x09:
        case 0x0a:
        case 0x03:
        default:
            m_word = battery().readWord(m_command);
            break;
        }
    }

    // Serial.print(F("Wire data received: @ len: "));
    // Serial.print(number);
    // Serial.print(" @ ");
    // Serial.print(" " + hex(m_command));
    // for (int i = 0; i < datalen; ++i) {
    //     Serial.print(" " + hex(buf[i]));
    // }
    // Serial.println();
}

void SBSProxy::onRequest()
{
    switch (m_command) {
    case 0x0c:
    case 0x0d:
    case 0x0e:
        Wire.write(static_cast<uint8_t>(m_word));
        break;
    case 0x20:
    case 0x21:
    case 0x22:
    case 0x23:
    case 0x2f:
        break;
    case 0x08:
    case 0x09:
    case 0x0a:
    case 0x03:
    default:
        Wire.write(static_cast<uint8_t>(m_word));
        Wire.write(static_cast<uint8_t>(m_word >> 8));
    }

    m_word = 0;
    m_command = 0xFF;

    // Serial.print(F("Wire command requested: "));
    // Serial.println(hex(m_command));
    // Serial.print(" => answered: ");
    // Serial.println(m_word);
}
