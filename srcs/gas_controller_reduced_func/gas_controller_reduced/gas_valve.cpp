/*
	gas_valve.cpp library for discrete gas valve control
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#include "Arduino.h"
#include "gas_valve.h"

GasValve::GasValve(int valve_pin)
{
  vpin = valve_pin;
	pinMode(vpin, OUTPUT);
	digitalWrite(vpin, HIGH);
	valve_state = VALVE_OFF;
}

int GasValve::GetValveState()
{
	return valve_state;
}

void GasValve::SetValveState(int state)
{
	if (state == VALVE_ON){
		digitalWrite(vpin, LOW);
		valve_state = VALVE_ON;
	}
	else{
		digitalWrite(vpin, HIGH);
		valve_state = VALVE_OFF;
	}
}
