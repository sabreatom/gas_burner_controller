/**
 * Gas burner control software main loop.
 */
//------------------------------------------------
//Includes:
//--------------------------------------------------

#include <IRremote.h> //!< IR control library

//------------------------------------------------
//Defines:
//--------------------------------------------------

//#define AIR_SENSOR_0    0
//#deifne AIR_SENSOR_1    1
#define AIR_Q_THRES     300 //!< air quality sensor's threshold value to detect gas or smoke

//#define FLAME_CH0       2
//#define FLAME_CH1       3
//#define FLAME_CH2       4
#define FLAME_THRES     600  //!< IR flame sensor threshold value to detect that flame is burning

#define IGN_CNTRL       2
#define IGN_TIME        3 //!< ignition spark on time in seconds

int RECV_PIN = 11;  //!< IR remote receiver pin number
#define RC_ON           0xC101E57B  //!< IR command to power on
#define RC_OFF          0x97483BFB  //!< IR command to power off
#define RC_UP           0x511DBB  //!< IR command to increase gas flow
#define RC_DOWN         0xA3C8EDDB  //!< IR command to decrease gas flow

#define GAS_VALVE_POS_PIN   4 //!< gas valve positive direction control pin
#define GAS_VALVE_NEG_PIN   5 //!< gas valve negative direction control pin
#define GAS_VALVE_POS_ADC   5 //!< gas valve position ADC pin
//#define GAS_VALVE_MAX_POS   980 //!< gas valve max position value
#define GAS_VALVE_MAX_POS   350 //!< gas valve max position value
#define GAS_VALVE_OFF_POS   750
#define GAS_VALVE_TYP_POS   500
#define GAS_VALVE_ON        1
#define GAS_VALVE_OFF       0
#define GAS_VALVE_STOP      2
#define FLAME_STEP          20

#define GAS_VALVE_SAFETY_SW 10

#define ACTIVE_LED          6
#define FAULT_LED           7

#define AUDIO_SEL_PIN       8
#define AUDIO_DIS_PIN       9

enum fsm_states {fsm_idle,fsm_active,fsm_fault}; //!< FSM states
fsm_states curr_state = fsm_idle;

enum audio_states {audio_in_sig,alarm_sig,audio_off};

//------------------------------------------------
//Global variables:
//------------------------------------------------

IRrecv irrecv(RECV_PIN);
decode_results results;

unsigned char rc5_prev_toogle = 1;
unsigned char fault_ok_cntr = 0;
int valve_cur_pos = GAS_VALVE_TYP_POS;

//------------------------------------------------
//Functions:
//------------------------------------------------

void audio_cntrl(audio_states nxt_state);
boolean chck_air(unsigned int air_thres);
boolean chck_flame(unsigned int f_thres);
void ignition_spark_cntrl(unsigned int spark_on_dly);
boolean gas_valve_cntrl(unsigned int valve_pos);
void gas_valve_act(unsigned char state);

//-----------------------------------------------
//Setup function:
//-----------------------------------------------

void setup() {
  //Serial port setup for debug during development:
  Serial.begin(9600);
  
  //Setup ignition spark control pin:
  pinMode(IGN_CNTRL,OUTPUT);
  digitalWrite(IGN_CNTRL,HIGH); //Ignition control is active low

  //Setup RC-5 receiver:
  irrecv.enableIRIn(); // Start the receiver

  //Gas valve control pin setup:
  pinMode(GAS_VALVE_POS_PIN,OUTPUT);
  digitalWrite(GAS_VALVE_POS_PIN,LOW);
  pinMode(GAS_VALVE_NEG_PIN,OUTPUT);
  digitalWrite(GAS_VALVE_NEG_PIN,LOW);

  //Gas valve safety switch:
  pinMode(GAS_VALVE_SAFETY_SW,OUTPUT);
  digitalWrite(GAS_VALVE_SAFETY_SW,HIGH); //control is active low
  
  pinMode(ACTIVE_LED,OUTPUT);
  digitalWrite(ACTIVE_LED,LOW);
  pinMode(FAULT_LED,OUTPUT);
  digitalWrite(FAULT_LED,LOW);

  pinMode(AUDIO_SEL_PIN,OUTPUT);
  digitalWrite(AUDIO_SEL_PIN,LOW);
  pinMode(AUDIO_DIS_PIN,OUTPUT);
  digitalWrite(AUDIO_DIS_PIN,HIGH);
  Serial.println("Starting system");
}

//-----------------------------------------------
//Main loop function:
//-----------------------------------------------

void loop() {
  switch(curr_state)
  {
    case fsm_idle:                                    //IDLE state
      if (irrecv.decode(&results)) {
        if (results.value == RC_ON) {
          gas_valve_safety_switch(true);
          Serial.println("Starting ignition");
          gas_valve_cntrl(GAS_VALVE_MAX_POS);
          ignition_spark_cntrl(IGN_TIME);
          if (chck_flame(FLAME_THRES)) {
            Serial.println("Flame OK, jumping to active");
            gas_valve_cntrl(GAS_VALVE_TYP_POS);
            curr_state = fsm_active;
            digitalWrite(ACTIVE_LED,HIGH);
            valve_cur_pos = GAS_VALVE_TYP_POS;
            audio_cntrl(audio_in_sig);
          }
          else {
            Serial.println("Flame not OK, jumping to fault");
            gas_valve_safety_switch(false);
            gas_valve_cntrl(GAS_VALVE_OFF_POS);
            curr_state = fsm_fault;
            audio_cntrl(alarm_sig);
          }
        }
        irrecv.resume();
      }

      if (chck_air(AIR_Q_THRES)) {                   //Check air quality
        Serial.println("Jumping to fault");
        gas_valve_safety_switch(false);
        curr_state = fsm_fault;
        audio_cntrl(alarm_sig);
      }
      
      break;
    case fsm_active:                                //ACTIVE state
      if (irrecv.decode(&results)) {
        if (results.value == RC_OFF) {
          Serial.println("OFF");
          curr_state = fsm_idle;
          gas_valve_safety_switch(false);
          gas_valve_cntrl(GAS_VALVE_OFF_POS);
          audio_cntrl(audio_off);
        }
        else if (results.value == RC_UP) {
          if (valve_cur_pos >= (GAS_VALVE_MAX_POS + FLAME_STEP)) {
            Serial.println("UP");
            Serial.println(valve_cur_pos);
            valve_cur_pos -= FLAME_STEP;
            gas_valve_cntrl(valve_cur_pos);
          }
        }
        else if (results.value == RC_DOWN) {
          if (valve_cur_pos <= (GAS_VALVE_OFF_POS - FLAME_STEP)) {
            Serial.println("DOWN");
            Serial.println(valve_cur_pos);
            valve_cur_pos += FLAME_STEP;
            gas_valve_cntrl(valve_cur_pos);
          }
        }
        irrecv.resume();
      }

      if (chck_air(AIR_Q_THRES)) {                   //Check air quality
        Serial.println("Active fault");
        curr_state = fsm_fault;
        gas_valve_safety_switch(false);
        gas_valve_cntrl(GAS_VALVE_OFF_POS);
        audio_cntrl(alarm_sig);
      }
      break;
    case fsm_fault:                                 //FAULT state
      if (fault_ok_cntr < 60) {
        if (chck_air(AIR_Q_THRES)) {
          fault_ok_cntr = 0;
        }
        else {
          fault_ok_cntr++;
        }
      }
      else {
        Serial.println("Jumping to idle");
        curr_state = fsm_idle;
        audio_cntrl(audio_off);
      }
      digitalWrite(FAULT_LED,HIGH); //For blink of Fault LED, can be optimized
      delay(500);
      digitalWrite(FAULT_LED,LOW);
      delay(500);
      break;
  }
}

//------------------------------------------------
//Check air quality:
//------------------------------------------------

boolean chck_air(unsigned int air_thres)
{
  unsigned int data;
  unsigned int i;

//  for (i=0;i<2;i++)
//  {
//    data = analogRead(i);
//
//    if (data > air_thres)
//    {
//      return true;
//    }
//  }

  return false;
}

//------------------------------------------------
//Check flame detection sensor:
//------------------------------------------------

boolean chck_flame(unsigned int f_thres)
{
  unsigned int data;
  unsigned int i;

//  for (i=2;i<5;i++)
//  {
//    data = analogRead(i);
//
//    if (data > f_thres)
//    {
//      return false;
//    }
//
//    delay(5);
//  }

  return true;
}

//------------------------------------------------
//Control ignition spark:
//------------------------------------------------

void ignition_spark_cntrl(unsigned int spark_on_dly) //delay in seconds
{
  digitalWrite(IGN_CNTRL,LOW); //Start ignition
  delay(spark_on_dly * 1000);
  digitalWrite(IGN_CNTRL,HIGH); //Stop ignition
}

//------------------------------------------------
//Control for gas valve safety switch:
//------------------------------------------------

void gas_valve_safety_switch(bool state)
{
  if (state) {
    digitalWrite(GAS_VALVE_SAFETY_SW,LOW);
  }
  else {
    digitalWrite(GAS_VALVE_SAFETY_SW,HIGH);
  }
}

//------------------------------------------------
//Gas valve control:
//------------------------------------------------

boolean gas_valve_cntrl(unsigned int valve_pos)
{
  unsigned int cur_pos = analogRead(GAS_VALVE_POS_ADC);
  unsigned int prev_pos = 0;
  unsigned char iter = 0;
  
  if (valve_pos > GAS_VALVE_OFF_POS) {
    while (cur_pos < GAS_VALVE_OFF_POS) {
      gas_valve_act(GAS_VALVE_OFF);
      delay(1);
      cur_pos = analogRead(GAS_VALVE_POS_ADC);
    }
    gas_valve_act(GAS_VALVE_STOP);
  }
  else if (valve_pos < GAS_VALVE_MAX_POS) {
    while (cur_pos > GAS_VALVE_MAX_POS) {
      gas_valve_act(GAS_VALVE_ON);
      delay(1);
      cur_pos = analogRead(GAS_VALVE_POS_ADC);
    }
    gas_valve_act(GAS_VALVE_STOP);
  }
  else {
    if (valve_pos > cur_pos) {
      while(valve_pos > cur_pos) {
        gas_valve_act(GAS_VALVE_OFF);
        delay(1);
        cur_pos = analogRead(GAS_VALVE_POS_ADC);
      }
      gas_valve_act(GAS_VALVE_STOP);
    }
    else if (valve_pos < cur_pos) {
      while(valve_pos < cur_pos) {
        gas_valve_act(GAS_VALVE_ON);
        delay(1);
        cur_pos = analogRead(GAS_VALVE_POS_ADC);
      }
      gas_valve_act(GAS_VALVE_STOP);
    }
  }

  return true;
}

//------------------------------------------------
//Gas valve actuation functions:
//------------------------------------------------

void gas_valve_act(unsigned char state)
{
  if (state == GAS_VALVE_ON) {
    digitalWrite(GAS_VALVE_POS_PIN,LOW);
    digitalWrite(GAS_VALVE_NEG_PIN,HIGH);
  }
  else if (state == GAS_VALVE_OFF) {
    digitalWrite(GAS_VALVE_POS_PIN,HIGH);
    digitalWrite(GAS_VALVE_NEG_PIN,LOW);
  }
  else {
    digitalWrite(GAS_VALVE_POS_PIN,LOW);
    digitalWrite(GAS_VALVE_NEG_PIN,LOW);
  }
}

//------------------------------------------------
//Audio control function:
//------------------------------------------------

void audio_cntrl(audio_states nxt_state)
{
  switch(nxt_state) {
    case audio_in_sig:
      digitalWrite(AUDIO_SEL_PIN,HIGH);
      digitalWrite(AUDIO_DIS_PIN,LOW);
      break;
    case alarm_sig:
      digitalWrite(AUDIO_SEL_PIN,LOW);
      digitalWrite(AUDIO_DIS_PIN,LOW);
      break;
    case audio_off:
      digitalWrite(AUDIO_SEL_PIN,LOW);
      digitalWrite(AUDIO_DIS_PIN,HIGH);
      break;
  }
}

//------------------------------------------------
