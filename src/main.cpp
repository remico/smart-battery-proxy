#include <Arduino.h>
#include <FastSoftWire.h>
#include <Wire.h>

int sdaPin = PIN_A0;
int sclPin = PIN_A1;

// I2C address of A2168
const uint8_t BATTERY_I2C_ADDRESS = 0x0B;
uint8_t command = 0xFF;

FastSoftWire sw(sdaPin, sclPin);
AsyncDelay readInterval;

// These buffers must be at least as large as the largest read or write you perform.
char swTxBuffer[16];
char swRxBuffer[16];


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
}

void onRequest()
{
    Serial.println("Command requested: " + hex(command));

    if (command == 0x09) // command: voltage
    {
        uint16_t voltage = 12041; // mV
        Wire.write((uint8_t)voltage);
        Wire.write((uint8_t)(voltage >> 8));
        Serial.println("Command 0x09 => answered");
    }
    command = 0xFF;
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

#define I2C_SPEED 40000

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

    // master
    sw.setTxBuffer(swTxBuffer, sizeof(swTxBuffer));
    sw.setRxBuffer(swRxBuffer, sizeof(swRxBuffer));
    // sw.setClock(I2C_SPEED);
    sw.setDelay_us(0);
    sw.setTimeout(1000);
    sw.begin();

    readInterval.start(2000, AsyncDelay::MILLIS);

    // slave
    Wire.begin(0x0B);  // mimic a smart battery
    // Wire.setClock(I2C_SPEED);
    Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);
}

void loop(void)
{
    if (readInterval.isExpired())
    {
        readBattery();
        readInterval.restart();
    }
}
