/*
	gas_valve.h library for discrete gas valve control
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#ifndef GAS_VALVE_H
#define GAS_VALVE_H

#include "Arduino.h"

#define VALVE_ON	1
#define VALVE_OFF	0

class GasValve
{
	public:
		GasValve(int valve_pin);
		int GetValveState();
		void SetValveState(int state);
	private:
		int valve_state;
    int vpin;
};

#endif
