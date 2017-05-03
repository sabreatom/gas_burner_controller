/*
	flame_check.cpp library for flame check on the burner
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#include "Arduino.h"
#include "flame_check.h"

FlameCheck::FlameCheck(int pin, unsigned int threshold)
{
	current_threshold = threshold;
	sensor_pin = pin;
}

boolean FlameCheck::CheckFlameThreshold()
{
  return true;
	if (analogRead(sensor_pin) > current_threshold){
		return false;
	}
	else{
		return true;
	}
}

void FlameCheck::SetFlameThreshold(unsigned int threshold)
{
	current_threshold = threshold;
}

unsigned int FlameCheck::GetFlameThreshold()
{
	return current_threshold;
}

unsigned int FlameCheck::GetFlameValue()
{
  return analogRead(sensor_pin);
}

