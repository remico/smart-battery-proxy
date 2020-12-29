#include <Arduino.h>
#include <AsyncDelay.h>
#include <GyverPower.h>
#include "battery_util.h"
#include "sbsproxy.h"

sbs::SBSProxy *proxy = sbs::SBSProxy::instance();
AsyncDelay readInterval;

void goSleep()
{

}

void onWakeUp()
{

}

void setup()
{
    Serial.begin(115200);
    // digitalWrite(0, HIGH);
    // digitalWrite(1, HIGH);

    Serial.println(F("Read smart battery A2168"));
    Serial.print(F("    Battery SMBus address: 0x"));
    Serial.println(proxy->battery().address(), HEX);

    readInterval.start(5000, AsyncDelay::MILLIS);
    _delay_ms(100);  // sanity

    // read the battery
    // Serial.println(F("\n++++++++++++ BATTERY ++++++++++++"));
    // readBattery(battery);
    // Serial.println(F("********************************************\n"));

    _delay_ms(1000);  // sanity
    proxy->enableBattery(true);  // make the battery proxy visible for the laptop

    power.setSleepMode(IDLE_SLEEP);
}

void loop(void)
{
    handleUserInput(proxy);

    if (readInterval.isExpired())
    {
        printHumanizedBatteryStatus(proxy->status());
        proxy->printPowerStats();
        readInterval.restart();
    }

    power.sleep(SLEEP_FOREVER);
}
