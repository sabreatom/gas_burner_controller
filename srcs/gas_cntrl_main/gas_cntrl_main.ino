//------------------------------------------------
//Includes:
--------------------------------------------------

#include "RC5.h"

//------------------------------------------------
//Defines:
--------------------------------------------------

//#define AIR_SENSOR_0    0
//#deifne AIR_SENSOR_1    1
#define AIR_Q_THRES     150

//#define FLAME_CH0       2
//#define FLAME_CH1       3
//#define FLAME_CH2       4
#define FLAME_THRES     50

#define IGN_CNTRL       2
#define IGN_TIME        3 //ignition spark on time in seconds

#define IR_PIN          3
#define RC5_ADDR        16
#define RC_ON_OFF       12

#define GAS_VALVE_POS_PIN   4
#define GAS_VALVE_NEG_PIN   5
#define GAS_VALVE_POS_ADC   5
#define GAS_VALVE_MAX_POS   1023
#define GAS_VALVE_OFF_POS   0
#deifne GAS_VALVE_TYP_POS   512

enum fsm_states {fsm_idle,fsm_active,fsm_fault};
fsm_states curr_state = fsm_idle;

//------------------------------------------------
//Global variables:
//------------------------------------------------

unsigned char rc5_prev_toogle = 1;
unsigned char fault_ok_cntr = 0;

//-----------------------------------------------
//Setup function:
//-----------------------------------------------

void setup() {
  //Serial port setup for debug during development:
  Serial.begin(9600);
  
  //Setup ignition spark control pin:
  pinMode(IGN_CNTRL,OUTPUT);
  digitalWrite(IGN_CNTRL,LOW);

  //Setup RC-5 receiver:
  RC5 rc5(IR_PIN);

  //Gas valve control pin setup:
  pinMode(GAS_VALVE_POS_PIN,OUTPUT);
  digitalWrite(GAS_VALVE_POS_PIN,LOW);
  pinMode(GAS_VALVE_NEG_PIN,OUTPUT);
  digitalWrite(GAS_VALVE_NEG_PIN,LOW);
}

//-----------------------------------------------
//Main loop function:
//-----------------------------------------------

void loop() {
  unsigned char rc5_toogle;
  unsigned char rc5_address;
  unsigned char rc5_command;
  
  switch(curr_state)
  {
    case fsm_idle:
      if (rc5.read(&rc5_toogle,&rc5_address,&rc5_command))
      {
        if (rc5_address == RC5_ADDR)
        {
          if (rc5_toogle != rc5_prev_toogle)
          {
            if (rc5_command == RC_ON_OFF)
            {
              gas_valve_cntrl(GAS_VALVE_MAX_POS);
              ignition_spark_cntrl(IGN_TIME);
              if (chck_flame(FLAME_THRES))
              {
                gas_valve_cntrl(GAS_VALVE_TYP_POS);
                curr_state = fsm_active;
              }
              else
              {
                gas_valve_cntrl(GAS_VALVE_OFF_POS);
                curr_state = fsm_fault;
              }
            }
          }

          rc5_prev_toogle = rc5_toogle;
        }
      }

      if (chck_air(AIR_Q_THRES))
      {
        curr_state = fsm_fault;
        //Need to enable alarm audio signal
      }
      
      break;
    case fsm_active:
      break;
    case fsm_fault:
      while (fault_ok_cntr < 60)
      {
        if (chck_air(AIR_Q_THRES))
        {
          fault_ok_cntr = 0;
        }
        else
        {
          fault_ok_cntr++;
        }
      }
      delay(1000);
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

  for (i=0;i<2;i++)
  {
    data = analogRead(i);

    if (data > air_thres)
    {
      return true;
    }
  }

  return false;
}

//------------------------------------------------
//Check flame detection sensor:
//------------------------------------------------

boolean chck_flame(unsigned int f_thres)
{
  unsigned int data;
  unsigned int i;

  for (i=2;i<5;i++)
  {
    data = analogRead(i);

    if (data <= f_thres)
    {
      return false
    }

    delay(5);
  }

  return true
}

//------------------------------------------------
//Control ignition spark:
//------------------------------------------------

void ignition_spark_cntrl(unsigned int spark_on_dly) //delay in seconds
{
  digitalWrite(IGN_CNTRL,HIGH); //Start ignition
  delay(spark_on_dly*1000);
  digitalWrite(IGN_CNTRL,LOW); //Start ignition
}

//------------------------------------------------
//Gas valve control:
//------------------------------------------------

boolean gas_valve_cntrl(unsigned int valve_pos)
{
  unsigned int cur_pos = analogRead(GAS_VALVE_POS_ADC);
  //Control if valve needs to be implemented in the future for the safety reasons
  if (valve_pos < cur_pos)
  {
    digitalWrite(GAS_VALVE_POS_PIN,HIGH);
    digitalWrite(GAS_VALVE_NEG_PIN,LOW);
    cur_pos = analogRead(GAS_VALVE_POS_ADC);
    while (valve_pos < cur_pos)
    {
      cur_pos = analogRead(GAS_VALVE_POS_ADC);
      delay(5);
    }
    digitalWrite(GAS_VALVE_POS_PIN,LOW);
  }
  else if (valve_pos > cur_pos)
  {
    digitalWrite(GAS_VALVE_POS_PIN,LOW);
    digitalWrite(GAS_VALVE_NEG_PIN,HIGH);
    cur_pos = analogRead(GAS_VALVE_POS_ADC);
    while (valve_pos > cur_pos)
    {
      cur_pos = analogRead(GAS_VALVE_POS_ADC);
      delay(5);
    }
    digitalWrite(GAS_VALVE_NEG_PIN,LOW);
  }
  else
  {
    return true;
  }

  return true;
}

//------------------------------------------------
