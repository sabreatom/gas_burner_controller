/*
	flame_check.h library for flame detection on the burner
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#ifndef FLAME_CHECK_H
#define FLAME_CHECK_H

#include "Arduino.h"

class FlameCheck
{
	public:
		FlameCheck(int pin, unsigned int threshold);
		boolean CheckFlameThreshold();
		void SetFlameThreshold(unsigned int threshold);
		unsigned int GetFlameThreshold();
	private:
		unsigned int current_threshold;
		int sensor_pin;
};

#endif
