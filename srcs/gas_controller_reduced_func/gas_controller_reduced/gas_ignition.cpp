/*
	gas_ignition.cpp library for gas level sensor
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#include "Arduino.h"
#include "gas_ignition.h"

#define IGNITION_ON		LOW
#define IGNITION_OFF	HIGH

GasIgnition::GasIgnition(int pin, unsigned int delay)
{
	spark_delay = delay;
	ignition_pin = pin;
	
	pinMode(ignition_pin, OUTPUT);
	digitalWrite(ignition_pin, IGNITION_OFF);
}

void GasIgnition::IgnitionOn()
{
	digitalWrite(ignition_pin, IGNITION_ON); //Start ignition
	delay(spark_delay * 1000);
	digitalWrite(ignition_pin, IGNITION_OFF); //Stop ignition
}

void GasIgnition::SetSparkDelay(unsigned int delay)
{
	spark_delay = delay;
}

unsigned int GasIgnition::GetSparkDelay()
{
	return spark_delay;
}
