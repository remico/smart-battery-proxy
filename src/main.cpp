#include <Arduino.h>
#include <AsyncDelay.h>
#include "battery.h"
#include "proxy.h"

AsyncDelay readInterval;

void setup()
{
    Serial.begin(115200);

    Serial.println(F("Read smart battery A2168"));
    Serial.print(F("    Battery SMBus address: 0x"));
    Serial.println(sbs::SBSProxy::instance()->battery().address(), HEX);

    readInterval.start(2000, AsyncDelay::MILLIS);

    // read battery
    Serial.println(F("\n++++++++++++ BATTERY ++++++++++++"));
    _delay_ms(100);  // sanity
    readBattery(sbs::SBSProxy::instance()->battery());
    Serial.println(F("********************************************\n"));
}

void loop(void)
{
    if (readInterval.isExpired())
    {
        // do periodic things
        readInterval.restart();
    }

    // handleUserInput(battery);
}
