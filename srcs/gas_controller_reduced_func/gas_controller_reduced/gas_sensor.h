/*
	gas_sensor.h library for discrete gas valve control
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#ifndef GAS_SENSOR_H
#define GAS_SENSOR_H

#include "Arduino.h"

class GasSensor
{
	public:
		GasSensor(int pin, unsigned int threshold);
		boolean CheckGasThreshold();
		void SetGasThreshold(unsigned int threshold);
		unsigned int GetGasThreshold();
   unsigned int GetGasValue();
	private:
		unsigned int current_threshold;
		int sensor_pin;
};

#endif
