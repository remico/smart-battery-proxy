#include <Arduino.h>
#include <SoftWire.h>
#include <Wire.h>

#if 1
int sdaPin = PIN_A0;
int sclPin = PIN_A1;
#else
int sdaPin = SDA;
int sclPin = SCL;
#endif

// I2C address of A2168
const uint8_t BATTERY_I2C_ADDRESS = 0x0B;

SoftWire sw(sdaPin, sclPin);
// These buffers must be at least as large as the largest read or write you perform.
char swTxBuffer[16];
char swRxBuffer[16];

AsyncDelay readInterval;

// Print with leading zero, as expected for time
void printTwoDigit(int n)
{
    if (n < 10)
    {
        Serial.print('0');
    }
    Serial.print(n);
}

void readBattery(void)
{
    uint8_t b1, b2 = 0;
    uint16_t voltage = 0;
    sw.beginTransmission(BATTERY_I2C_ADDRESS);
    sw.write(uint8_t(0x09));  // battery voltage
    if (sw.endTransmission() == 0)
    {
        sw.requestFrom(BATTERY_I2C_ADDRESS, 2, true);
        b1 = sw.read();
        b2 = sw.read();
        voltage = b1 | (b2 << 8);
    }

    Serial.print("Battery voltage: ");
    Serial.print(voltage);
    Serial.println("mV");
}

void setup()
{
    Serial.begin(57600);

    Serial.println("Read smart battery A2168");
    Serial.print("    SDA pin: ");
    Serial.println(int(sdaPin));
    Serial.print("    SCL pin: ");
    Serial.println(int(sclPin));
    Serial.print("    I2C address: ");
    Serial.println(int(BATTERY_I2C_ADDRESS), HEX);

    sw.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
    sw.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
    sw.setClock(40000);
    sw.setTimeout(1000);
    sw.begin();

    readInterval.start(2000, AsyncDelay::MILLIS);
}

void loop(void)
{
    if (readInterval.isExpired())
    {
        readBattery();
        readInterval.restart();
    }
}
