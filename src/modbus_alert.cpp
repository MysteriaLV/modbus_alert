//
// Created by VermutMac on 2018-08-18.
//

#include "Automaton.h"

extern void modbus_setup(), modbus_loop();

Atm_led alarm;
Atm_button slaves[7];

void setup() {
	Serial.begin(115200);
	modbus_setup();

	slaves[0].begin(A0);
	slaves[1].begin(13);
	slaves[2].begin(12);
	slaves[3].begin(11);
	slaves[4].begin(5);
	slaves[5].begin(4);
	slaves[6].begin(3);

	alarm.begin(2);
}

void processSlavePing(uint8_t slaveId, boolean isOk) {
	if (isOk)
		return;

	if (slaves[slaveId - 1].state() == Atm_button::PRESSED) {
		alarm.blink(100, 100, slaveId).start();
	} else {
		Serial.println("...ignoring");
	}
}

void loop() {
	modbus_loop();
	automaton.run();
}