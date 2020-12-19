#pragma once

#include "sbs.h"

namespace sbs {

class SBSProxy {
public:
    static SBSProxy *instance();
    static SBS &battery() { return m_battery; }

private:
    SBSProxy();
    static void onReceive(const int number);
    static void onRequest();

    static SBS m_battery;
    static uint8_t m_command;
    static uint16_t m_word;
};

} // namespace sbs
