/**
 * Gas burner control software main loop.
 */
//------------------------------------------------
//Includes:
//------------------------------------------------

#include <IRremote.h> //!< IR control library

#include "gas_valve.h"
#include "gas_controller.h"
#include "gas_sensor.h"
#include "gas_ignition.h"
#include "flame_check.h"
#include "gas_pump.h"

//------------------------------------------------
//Defines:
//------------------------------------------------

#define AIR_Q_PIN       0
#define AIR_Q_THRES     300 //!< air quality sensor's threshold value to detect gas or smoke

#define FLAME_PIN       1
#define FLAME_THRES     600  //!< IR flame sensor threshold value to detect that flame is burning

#define IGN_CNTRL       2
#define IGN_TIME        3 //!< ignition spark on time in seconds

#define PUMP_CNTRL_PIN  5 //gas pump control pin

int RECV_PIN = 11;  //!< IR remote receiver pin number
#define RC_ON           0xC101E57B  //!< IR command to power on
#define RC_OFF          0x97483BFB  //!< IR command to power off
#define RC_PUMP         0x511DBB  //!< IR command to ON/OFF gas pump
//#define RC_DOWN         0xA3C8EDDB  //!< IR command to decrease gas flow

#define GAS_VALVE_PIN   4 //!< gas valve control pin

//------------------------------------------------
//Global variables:
//------------------------------------------------

IRrecv irrecv(RECV_PIN);
decode_results results;

GasValve gas_valve(GAS_VALVE_PIN);

GasController gas_controller(fsm_idle);

GasIgnition gas_ign(IGN_CNTRL, IGN_TIME);

GasSensor gas_sensor(AIR_Q_PIN, AIR_Q_THRES);

FlameCheck flame_check(FLAME_PIN, FLAME_THRES);

GasPump gas_pump(PUMP_CNTRL_PIN);

unsigned char fault_ok_cntr = 0;

//------------------------------------------------

void setup() {
  //Serial port setup for debug during development:
  Serial.begin(9600);

  //Setup RC-5 receiver:
  irrecv.enableIRIn(); // Start the receiver
}

void loop() {
  switch(gas_controller.GetStateMachineState())
  {
    case fsm_idle:                                    //IDLE state
      if (irrecv.decode(&results)) {
        if (results.value == RC_ON) {
          Serial.println("Starting ignition");
          
          if (gas_pump.GetPumpState() == PUMP_ON) { //turn off pump if ignition is starting
            gas_pump.SetPumpState(PUMP_OFF);
          }
          
          gas_valve.SetValveState(VALVE_ON);
          gas_ign.IgnitionOn();
          if (flame_check.CheckFlameThreshold()) {
            Serial.println("Flame OK, jumping to active");
            gas_controller.SetStateMachineState(fsm_active);
          }
          else {
            Serial.println("Flame not OK, jumping to fault");
            gas_valve.SetValveState(VALVE_OFF);
            gas_controller.SetStateMachineState(fsm_fault);
          }
        }
        else if (results.value == RC_PUMP) { //gas pump IR control
          if (gas_pump.GetPumpState() == PUMP_ON) {
            gas_pump.SetPumpState(PUMP_OFF);
          }
          else {
            gas_pump.SetPumpState(PUMP_ON);
          }
        }
        irrecv.resume();
      }
      if (gas_sensor.CheckGasThreshold()) {                   //Check air quality
        Serial.println("Jumping to fault");
        gas_valve.SetValveState(VALVE_OFF);
        gas_controller.SetStateMachineState(fsm_fault);
      }  
         
      break;
    case fsm_active:                                //ACTIVE state
      if (irrecv.decode(&results)) {
        if (results.value == RC_OFF) {
          Serial.println("OFF");
          gas_controller.SetStateMachineState(fsm_idle);
          gas_valve.SetValveState(VALVE_OFF);
        }
        irrecv.resume();
      }
      if (gas_sensor.CheckGasThreshold()) {                   //Check air quality
        Serial.println("Jumping to fault");
        gas_valve.SetValveState(VALVE_OFF);
        gas_controller.SetStateMachineState(fsm_fault);
      }
      break;
    case fsm_fault:                                 //FAULT state
      if (fault_ok_cntr < 60) {
        if (gas_sensor.CheckGasThreshold()) {
          fault_ok_cntr = 0;
        }
        else {
          fault_ok_cntr++;
        }
      }
      else {
        Serial.println("Jumping to idle");
        gas_controller.SetStateMachineState(fsm_idle);
        gas_valve.SetValveState(VALVE_OFF);
      }
      break;
  }

  Serial.println("Gas:");
  Serial.println(gas_sensor.GetGasValue());
  Serial.println("Flame:");
  Serial.println(flame_check.GetFlameValue());
}
