/*
	gas_controller.cpp library for gas controller state machine
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#include "Arduino.h"
#include "gas_controller.h"

GasController::GasController(fsm_states InitState)
{
	CurrentState = InitState;
}

fsm_states GasController::GetStateMachineState()
{
	return CurrentState;
}

void GasController::SetStateMachineState(fsm_states State)
{	
	CurrentState = State;
}
