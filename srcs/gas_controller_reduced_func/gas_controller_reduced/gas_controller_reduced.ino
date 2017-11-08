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

//Alternative IR codes:
#define RC_ALT_VALVE    0x2C2E6663
#define RC_ALT_IGN      0x5AAD0C1
#define RC_ALT_AUDIO    0xA480D5C7
#define RC_ALT_PUMP     0x19272021

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
  if (irrecv.decode(&results)) {
    if (results.value == RC_ALT_VALVE) {
      Serial.println("Valve command");
      
      if (gas_valve.GetValveState() == VALVE_ON) {  //gas valve command
        Serial.println("Gas valve off");
        gas_valve.SetValveState(VALVE_OFF);
      }
      else {
        Serial.println("Gas valve on");
        gas_valve.SetValveState(VALVE_ON);
      }
    }
    else if (results.value == RC_ALT_IGN) { //ignition command
      Serial.println("Ignition command");

      gas_ign.IgnitionOn();
    }
    else if (results.value == RC_ALT_AUDIO) { //audio control command
      Serial.println("Audio command");

      if (digitalRead(AUDIO_AMP_CNTRL) == AUDIO_ON) {
        Serial.println("Audio off");
        digitalWrite(AUDIO_AMP_CNTRL, AUDIO_OFF);
      }
      else {
        Serial.println("Audio on");
        digitalWrite(AUDIO_AMP_CNTRL, AUDIO_ON);
      }
    }
    else if (results.value == RC_ALT_PUMP) { //pump control command
      Serial.println("Pump command");

      if (gas_pump.GetPumpState() == PUMP_ON) {
        Serial.println("Pump off");
        gas_pump.SetPumpState(PUMP_OFF);
      }
      else {
        Serial.println("Pump on");
        gas_pump.SetPumpState(PUMP_ON);
      }
    }
    else {
      Serial.println("Unknown command");
    }

    delay(500);
    irrecv.resume();
  }
}
