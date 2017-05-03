/*
	gas_pump.cpp library for gas pump control
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#include "Arduino.h"
#include "gas_pump.h"

GasPump::GasPump(int pump_pin)
{
	ppin = pump_pin;
	pinMode(ppin, OUTPUT);
	digitalWrite(ppin, HIGH);
	pump_state = PUMP_OFF;
}

int GasPump::GetPumpState()
{
	return pump_state;
}

void GasPump::SetPumpState(int state)
{
	if (state == PUMP_ON){
		digitalWrite(ppin, LOW);
		pump_state = PUMP_ON;
	}
	else{
		digitalWrite(ppin, HIGH);
		pump_state = PUMP_OFF;
	}
}