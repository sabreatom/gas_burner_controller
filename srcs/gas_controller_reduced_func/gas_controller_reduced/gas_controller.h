/*
	gas_controller.h library for discrete gas valve control
	Created by Aleksandrs Maklakovs, March 22, 2017
*/

#ifndef GAS_CONTROLLER_H
#define GAS_CONTROLLER_H

#include "Arduino.h"

enum fsm_states {fsm_idle,fsm_active,fsm_fault}; //!< FSM states

class GasController
{
	public:
		GasController(fsm_states InitState);
		fsm_states GetStateMachineState();
		void SetStateMachineState(fsm_states State); 
	private:
		fsm_states CurrentState;
};

#endif
