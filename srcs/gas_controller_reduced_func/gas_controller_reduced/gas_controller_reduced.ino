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
#define FLAME_THRES     100  //!< IR flame sensor threshold value to detect that flame is burning

#define IGN_CNTRL       2
#define IGN_TIME        3 //!< ignition spark on time in seconds

#define PUMP_CNTRL_PIN  5 //gas pump control pin

#define AUDIO_AMP_CNTRL 6 //audio amplifier control pin
#define AUDIO_ON      LOW
#define AUDIO_OFF     HIGH

int RECV_PIN = 11;  //!< IR remote receiver pin number
#define RC_ON           0xCE1FCDDD  //!< IR command to power on
#define RC_OFF          0xCE1FCDDD  //!< IR command to power off
#define RC_PUMP         0x2C2E6663  //!< IR command to ON/OFF gas pump
#define RC_COLD_START   0x5AAD0C1
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

unsigned int dly_counter = 0;

//------------------------------------------------

void setup() {
  //Serial port setup for debug during development:
  Serial.begin(9600);

  pinMode(AUDIO_AMP_CNTRL, OUTPUT);
  digitalWrite(AUDIO_AMP_CNTRL, AUDIO_OFF);
  
  //Setup RC-5 receiver:
  irrecv.enableIRIn(); // Start the receiver
  Serial.println("System initialization finished!");
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
          delay(1000); //need delay between valve open and start sparking so that there will be gas and also reduce problem with transiant
          gas_ign.IgnitionOn();
          if (flame_check.CheckFlameThreshold()) {
            digitalWrite(AUDIO_AMP_CNTRL, AUDIO_ON);
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
            Serial.println("Pump off");
          }
          else {
            gas_pump.SetPumpState(PUMP_ON);
            delay(500);
            gas_ign.IgnitionOn();
            Serial.println("Pump on");
          }
        }
        else if (results.value == RC_COLD_START) { //for cold start to fill tube with gas
          Serial.println("Filling tube with gas");
          if (gas_pump.GetPumpState() == PUMP_ON) { //turn off pump if ignition is starting
            gas_pump.SetPumpState(PUMP_OFF);
          }
          
          gas_valve.SetValveState(VALVE_ON);
          delay(3000);
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
    case fsm_active:                                //ACTIVE state
      if (irrecv.decode(&results)) {
        if (results.value == RC_OFF) {
          digitalWrite(AUDIO_AMP_CNTRL, AUDIO_OFF);
          Serial.println("OFF");
          gas_controller.SetStateMachineState(fsm_idle);
          gas_valve.SetValveState(VALVE_OFF);
        }
        irrecv.resume();
      }
      if (gas_sensor.CheckGasThreshold()) {                   //Check air quality
        Serial.println("Jumping to fault");
        digitalWrite(AUDIO_AMP_CNTRL, AUDIO_OFF); //TODO in final product will need to start alert
        gas_valve.SetValveState(VALVE_OFF);
        gas_controller.SetStateMachineState(fsm_fault);
      }
      if (!flame_check.CheckFlameThreshold()) { //TODO: need to implement timeout if gas doesn't ignitite go to fault state
        gas_ign.IgnitionOn();
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

  if (dly_counter >= 60000){ //temporary for testing gas sensor and flame sensor
    dly_counter = 0;
    Serial.println("Gas:");
    Serial.println(gas_sensor.GetGasValue());
    Serial.println("Flame:");
    Serial.println(flame_check.GetFlameValue());
  }
  else{
    dly_counter++;
  }
}
