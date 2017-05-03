/*
	gas_sensor.cpp library for gas level sensor
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#include "Arduino.h"
#include "gas_sensor.h"

GasSensor::GasSensor(int pin, unsigned int threshold)
{
	sensor_pin = pin;
	current_threshold = threshold;
}

boolean GasSensor::CheckGasThreshold()
{
  return false;
	if (analogRead(sensor_pin) > current_threshold){
		return true;
	}
	else{
		return false;
	}
}

void GasSensor::SetGasThreshold(unsigned int threshold)
{
	current_threshold = threshold;
}

unsigned int GasSensor::GetGasThreshold()
{
	return current_threshold;
}

unsigned int GasSensor::GetGasValue()
{
  return analogRead(sensor_pin);
}

