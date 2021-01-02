#include <Arduino.h>
#include <avr/wdt.h>
#include <AsyncDelay.h>
#include "battery_util.h"
#include "sbsproxy.h"

sbs::SBSProxy *proxy = sbs::SBSProxy::instance();
AsyncDelay readInterval;

void WDT_setup()
{
    cli();
    wdt_reset();
    /* Clear WDRF in MCUSR */
    MCUSR &= ~_BV(WDRF);
    /* Start timed equence */
    WDTCSR |= _BV(WDCE) | _BV(WDE);
    /* prescaler = 1024K cycles (~8s), WD system reset mode */
    WDTCSR = _BV(WDE) | _BV(WDP3) | _BV(WDP0);
    sei();
}

void goSleep()
{
    /* force disable ADC and comparator */
    ADCSRA &= ~_BV(ADEN); 			// ADC
    ACSR |= _BV(ACD); 				// comparator

    // IDLE sleep + sleep enable
	SMCR = _BV(SE);

    // disable BOD
    // MCUCR = (0x03 << 5);
    // MCUCR = (0x02 << 5);

    // enter sleep mode
    asm ("sleep");

    // explicitly forbid sleeping on waking up
    SMCR = 0;

	/* re-enable ADC and comparator */
	// if (!(PRR & _BV(PRADC))) {
	// 	ADCSRA |= _BV(ADEN);
	// 	ACSR &= ~ _BV(ACD);
	// }
}

void setup()
{
    WDT_setup();

    Serial.begin(115200);
    // digitalWrite(0, HIGH);
    // digitalWrite(1, HIGH);

    Serial.println(F("Read smart battery A2168"));
    Serial.print(F("    Battery SMBus address: 0x"));
    Serial.println(proxy->battery().address(), HEX);

    readInterval.start(5000, AsyncDelay::MILLIS);
    _delay_ms(100); // sanity

    // read the battery
    // Serial.println(F("\n++++++++++++ BATTERY ++++++++++++"));
    // readBattery(battery);
    // Serial.println(F("********************************************\n"));

    _delay_ms(1000);            // sanity
    proxy->enableBattery(true); // make the battery proxy visible for the laptop
}

void loop(void)
{
    handleUserInput(proxy);

    if (readInterval.isExpired()) {
        printHumanizedBatteryStatus(proxy->status());
        proxy->printPowerStats();
        readInterval.restart();
    }

    goSleep();
    wdt_reset();
}
