/*
	gas_pump.h library for gas pump control
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#ifndef GAS_PUMP_H
#define GAS_PUMP_H

#include "Arduino.h"

#define PUMP_ON	1
#define PUMP_OFF	0

class GasPump
{
	public:
		GasPump(int pump_pin);
		int GetPumpState();
		void SetPumpState(int state);
	private:
		int pump_state;
		int ppin;
};

#endif
