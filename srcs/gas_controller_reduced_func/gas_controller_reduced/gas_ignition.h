/*
	gas_ignition.h library for gas ignition control
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#ifndef GAS_IGNITION_H
#define GAS_IGNITION_H

#include "Arduino.h"

class GasIgnition
{
	public:
		GasIgnition(int pin, unsigned int delay);
		void IgnitionOn();
		void SetSparkDelay(unsigned int delay);
		unsigned int GetSparkDelay();
	private:
		unsigned int spark_delay;
		int ignition_pin;
};

#endif
