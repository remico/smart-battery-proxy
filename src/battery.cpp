#include "battery.h"

void readBattery(sbs::SBS &battery)
{
    for (int i = 0; i < COMMAND_SET_SIZE; ++i)
    {
        sbs::SBSCommand cmd = battery.command(i);
        const String writability = cmd.writeable ? "[R/W] " : "[R]     ";

        Serial.print("0x");
        Serial.print(cmd.code < 16 ? "0" : "");
        Serial.print(cmd.code, HEX);
        Serial.print(" " + writability + " " + cmd.name() + ":   ");

        switch (cmd.code)
        {
        case 0x00:
            Serial.print("0b");
            Serial.println(battery.readWord(cmd.code), BIN);
            break;

        case 0x03:
        {
            const uint16_t batteryMode = battery.readWord(cmd.code);
            Serial.print("0b");
            Serial.println(batteryMode, BIN);
            Serial.println(F("   ===== BatteryMode flags: ====="));
            Serial.print(F("   @ INTERNAL_CHARGE_CONTROLLER: "));   Serial.println(0x0001 & batteryMode);
            Serial.print(F("   @ PRIMARY_BATTERY_SUPPORT: "));      Serial.println(bool(0x0002 & batteryMode));
            Serial.print(F("   @ CONDITION_FLAG: "));               Serial.println(bool(0x0080 & batteryMode));
            Serial.print(F("   @ CHARGE_CONTROLLER_ENABLED: "));    Serial.println(bool(0x0100 & batteryMode));
            Serial.print(F("   @ PRIMARY_BATTERY: "));              Serial.println(bool(0x0200 & batteryMode));
            Serial.print(F("   @ ALARM_MODE: "));                   Serial.println(bool(0x2000 & batteryMode));
            Serial.print(F("   @ CHARGER_MODE: "));                 Serial.println(bool(0x4000 & batteryMode));
            Serial.print(F("   @ CAPACITY_MODE: "));                Serial.println(bool(0x8000 & batteryMode));
            Serial.println(F("   ======================"));
            break;
        }

        case 0x07:
            Serial.println(battery.readByte(cmd.code) == 0x00 ? "FALSE" : "TRUE");
            break;

        case 0x08:
            //register is in Kelvin, convert to C
            Serial.print(battery.readWord(cmd.code) / 10 - 273.15);
            Serial.println(" *C");
            break;

        case 0x0a:  // signed values
        case 0x0b:
            Serial.print(static_cast<int>(battery.readWord(cmd.code)));
            Serial.println(cmd.units());
            break;

        case 0x0c:  // one-byte percentage
        case 0x0d:
        case 0x0e:
            Serial.print(battery.readByte(cmd.code));
            Serial.println(cmd.units());
            break;

        case 0x16:
        {
            Serial.print("0b");
            const uint16_t batteryStatus = battery.readWord(cmd.code);
            String errorCode;

            switch (0x000F & batteryStatus)
            {
            case 0: errorCode = "OK"; break;
            case 1: errorCode = "Busy"; break;
            case 2: errorCode = "ReservedCommand"; break;
            case 3: errorCode = "UnsupportedCommand"; break;
            case 4: errorCode = "AccessDenied"; break;
            case 5: errorCode = "Overflow/Underflow"; break;
            case 6: errorCode = "BadSize"; break;
            case 7: errorCode = "UnknownError"; break;
            default: errorCode = "?";
            }

            Serial.println(batteryStatus, BIN);
            Serial.println(F("   ===== BatteryStatus flags: ====="));
            Serial.print(F("   @ ErrorCode: "));
            Serial.println(errorCode.c_str());
            Serial.println(F("   ---- Status: ----"));
            Serial.print(F("   @ FULLY DISCHARGED: ")); Serial.println(bool(0x0010 & batteryStatus));
            Serial.print(F("   @ FULLY CHARGED: "));    Serial.println(bool(0x0020 & batteryStatus));
            Serial.print(F("   @ DISCHARGING: "));      Serial.println(bool(0x0040 & batteryStatus));
            Serial.print(F("   @ INITIALIZED: "));      Serial.println(bool(0x0080 & batteryStatus));
            Serial.println(F("   ---- Alarm: ----"));
            Serial.print(F("   @ REMAINING TIME ALARM: "));     Serial.println(bool(0x0100 & batteryStatus));
            Serial.print(F("   @ REMAINING CAPACITY ALARM: ")); Serial.println(bool(0x0200 & batteryStatus));
            Serial.print(F("   @ TERMINATE DISCHARGE ALARM: "));Serial.println(bool(0x0800 & batteryStatus));
            Serial.print(F("   @ OVER TEMP ALARM: "));          Serial.println(bool(0x1000 & batteryStatus));
            Serial.print(F("   @ TERMINATE CHARGE ALARM: "));   Serial.println(bool(0x4000 & batteryStatus));
            Serial.print(F("   @ OVER CHARGED ALARM: "));       Serial.println(bool(0x8000 & batteryStatus));
            Serial.println(F("   ======================"));
            break;
        }

        case 0x1a:
        {
            const uint16_t specificationInfo = battery.readWord(cmd.code);
            Serial.print("0b");
            Serial.println(specificationInfo, BIN);
            Serial.println(F("   ===== SpecificationInfo: ====="));
            Serial.print(F("   @ Revision: ")); Serial.println((0x000F & specificationInfo));
            Serial.print(F("   @ Version: "));  Serial.println((0x00F0 & specificationInfo) >> 4);
            Serial.print(F("   @ VScale: "));   Serial.println((0x0F00 & specificationInfo) >> 8);
            Serial.print(F("   @ IPScale: "));  Serial.println((0xF000 & specificationInfo) >> 12);
            Serial.println(F("   ======================"));
            break;
        }

        case 0x1b:
        {
            const uint16_t raw = battery.readWord(cmd.code);
            Serial.print((raw >> 5) & 0xF);
            Serial.print("/"); // month
            Serial.print(raw & 0xF);
            Serial.print("/");                 // day
            Serial.println((raw >> 9) + 1980); // year
            break;
        }

        case 0x1c:  // serial number
        case 0x17:  // counter
            Serial.println(battery.readWord(cmd.code));
            break;

        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x2f:
        {
            char s[33] = {0};   // max len is 32 chars + null terminator
            battery.readString(s, sizeof(s) - 1, cmd.code);
            for (uint8_t i = 0; i < sizeof(s); ++i) {
                if (isprint(s[i])) {
                    Serial.print(s[i]);
                }
            }
            Serial.println();
            break;
        }

        default:
            Serial.print(battery.readWord(cmd.code));
            Serial.println(cmd.units());
        }
    }
}

void handleUserInput(sbs::SBS &battery)
{
    if (Serial.available())
    {
        const String input = Serial.readString();

        if (input.length() != 5)
        {
            Serial.println(F("Bad input"));
            return;
        }

        sbs::SBSCommand cmd = battery.command(input);

        if (cmd.code == 0xFF) {
            Serial.print(F("Bad command: "));
            Serial.println(input);
            return;
        }
        else if (cmd.writeable) {
            Serial.print(F("Ready to write '"));
            Serial.print(cmd.name());
            Serial.println(F("'. Enter a new value or an empty string to abort."));
        }
        else {
            Serial.print(F("Read-only command '"));
            Serial.print(cmd.name());
            Serial.println("'");
            return;
        }

        // wait for data to write or operation cancellation
        while (!Serial.available())
            ;
        const String data = Serial.readString();

        if (data == "\n")
        {
            Serial.println(F("Operation cancelled"));
            return;
        }

        const short newValue = data.toInt();

        Serial.print(F("TO WRITE: 0x"));
        Serial.print(cmd.code < 16 ? "0" : "");
        Serial.print(cmd.code, HEX);
        Serial.print(" ");
        Serial.println(newValue);

        // wait for confirmation
        Serial.println(F("Write? [y/N]"));
        while (!Serial.available())
            ;
        const String confirmation = Serial.readString();

        if (confirmation != "y\n")
        {
            Serial.println(F("Operation cancelled"));
            return;
        }

        battery.writeWord(cmd.code, newValue);
        Serial.println("Done");
    }
}
