#include <Arduino.h>
#include <AsyncDelay.h>
#include "battery_util.h"
#include "proxy.h"

sbs::SBS &battery = sbs::SBSProxy::instance()->battery();
AsyncDelay readInterval;

void setup()
{
    Serial.begin(115200);

    Serial.println(F("Read smart battery A2168"));
    Serial.print(F("    Battery SMBus address: 0x"));
    Serial.println(battery.address(), HEX);

    readInterval.start(5000, AsyncDelay::MILLIS);
    _delay_ms(100);  // sanity

    // read the battery
    Serial.println(F("\n++++++++++++ BATTERY ++++++++++++"));
    // readBattery(battery);
    Serial.println(F("********************************************\n"));

    _delay_ms(1000);  // sanity
    // make the proxy visible for the laptop
    pinMode(PIN_A2, OUTPUT);
    digitalWrite(PIN_A2, LOW);
}

void loop(void)
{
    if (readInterval.isExpired())
    {
        // do periodic things
        readInterval.restart();
    }

    handleUserInput(battery);
}
