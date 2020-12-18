#include <Arduino.h>

#include <AsyncDelay.h>
#include <BitBang_SMBus.h>
#include <Wire.h>

#include "battery.h"

sbs::SBS battery(BATTERY_ADDRESS);
uint8_t command = 0xFF;

AsyncDelay readInterval;

String hex(int n)
{
    char buf[10] = {0};
    sprintf(buf, "0x");
    if (n < 16)
    {
        sprintf(buf + strlen(buf), "%d", 0);
    }
    sprintf(buf + strlen(buf), "%x", n);
    return String(buf);
}

void onReceive(int number)
{
    command = Wire.read();
    Serial.print(F("Wire data received: "));
    Serial.println(hex(command) + " @ len: " + number);
}

void onRequest()
{
    Serial.print(F("Wire command requested: "));
    Serial.println(hex(command));

    // if (command == 0x09) // command: voltage
    // {
    //     uint16_t voltage = 12041; // mV
    //     Wire.write((uint8_t)voltage);
    //     Wire.write((uint8_t)(voltage >> 8));
    //     Serial.println("Command 0x09 => answered");
    // }
    // command = 0xFF;
}

void setup()
{
    Serial.begin(115200);

    Serial.println(F("Read smart battery A2168"));
    Serial.print(F("    Battery SMBus address: "));
    Serial.println(BATTERY_ADDRESS, HEX);

    // master


    // slave
    Wire.begin(0x0B); // mimic a smart battery
    Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);

    readInterval.start(2000, AsyncDelay::MILLIS);

    _delay_ms(100); // sanity

    // read battery
    Serial.println(F("\n++++++++++++ BATTERY ++++++++++++"));
    readBattery(battery);
    Serial.println(F("********************************************\n"));
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
