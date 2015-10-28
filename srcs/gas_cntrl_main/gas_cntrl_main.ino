//------------------------------------------------
//Includes:
//--------------------------------------------------

#include <IRremote.h>

//------------------------------------------------
//Defines:
//--------------------------------------------------

//#define AIR_SENSOR_0    0
//#deifne AIR_SENSOR_1    1
#define AIR_Q_THRES     800

//#define FLAME_CH0       2
//#define FLAME_CH1       3
//#define FLAME_CH2       4
#define FLAME_THRES     50

#define IGN_CNTRL       2
#define IGN_TIME        3 //ignition spark on time in seconds

int RECV_PIN = 11; 
#define RC_ON           0xC101E57B
#define RC_OFF          0x97483BFB

#define GAS_VALVE_POS_PIN   4
#define GAS_VALVE_NEG_PIN   5
#define GAS_VALVE_POS_ADC   5
#define GAS_VALVE_MAX_POS   950
#define GAS_VALVE_OFF_POS   350
#define GAS_VALVE_TYP_POS   700
#define GAS_VALVE_ON        1
#define GAS_VALVE_OFF       0
#define GAS_VALVE_STOP      2

#define ACTIVE_LED          6
#define FAULT_LED           7

#define AUDIO_SEL_PIN       8
#define AUDIO_DIS_PIN       9

enum fsm_states {fsm_idle,fsm_active,fsm_fault};
fsm_states curr_state = fsm_idle;

enum audio_states {audio_in_sig,alarm_sig,audio_off};

//------------------------------------------------
//Global variables:
//------------------------------------------------

IRrecv irrecv(RECV_PIN);
decode_results results;

unsigned char rc5_prev_toogle = 1;
unsigned char fault_ok_cntr = 0;

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
      if (irrecv.decode(&results))
      {
        if (results.value == RC_ON)
        {
          Serial.println("Starting ignition");
          gas_valve_cntrl(GAS_VALVE_MAX_POS);
          ignition_spark_cntrl(IGN_TIME);
          if (chck_flame(FLAME_THRES))
          {
            Serial.println("Flame OK, jumping to active");
            gas_valve_cntrl(GAS_VALVE_TYP_POS);
            curr_state = fsm_active;
            digitalWrite(ACTIVE_LED,HIGH);
          }
          else
          {
            Serial.println("Flame not OK, jumping to fault");
            gas_valve_cntrl(GAS_VALVE_OFF_POS);
            curr_state = fsm_fault;
          }

          irrecv.resume();
        }
      }

      if (chck_air(AIR_Q_THRES))                   //Check air quality
      {
        Serial.println("Jumping to fault");
        curr_state = fsm_fault;
        //Need to enable alarm audio signal
      }
      
      break;
    case fsm_active:                                //ACTIVE state
      if (irrecv.decode(&results))
      {
        if (results.value == RC_OFF)
        {
          Serial.println("OFF");
          curr_state = fsm_idle;
          gas_valve_cntrl(GAS_VALVE_OFF_POS);
        }
        //Add commands parsing for flame increase and decrease
        irrecv.resume();
      }

      if (chck_air(AIR_Q_THRES))                    //Check air quality
      {
        Serial.println("Active fault");
        curr_state = fsm_fault;
        gas_valve_cntrl(GAS_VALVE_OFF_POS);
        //Need to enable alarm audio signal
      }
      break;
    case fsm_fault:                                 //FAULT state
      if (fault_ok_cntr < 60)
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
      else
      {
        Serial.println("Jumping to idle");
        curr_state = fsm_idle;
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
      return false;
    }

    delay(5);
  }

  return true;
}

//------------------------------------------------
//Control ignition spark:
//------------------------------------------------

void ignition_spark_cntrl(unsigned int spark_on_dly) //delay in seconds
{
  digitalWrite(IGN_CNTRL,LOW); //Start ignition
  delay(spark_on_dly*1000);
  digitalWrite(IGN_CNTRL,HIGH); //Start ignition
}

//------------------------------------------------
//Gas valve control:
//------------------------------------------------

boolean gas_valve_cntrl(unsigned int valve_pos)
{
  unsigned int cur_pos = analogRead(GAS_VALVE_POS_ADC);
  unsigned int prev_pos = 0;
  unsigned char iter = 0;
  
  if (valve_pos < GAS_VALVE_OFF_POS)
  {
    while (cur_pos > GAS_VALVE_OFF_POS)
    {
      gas_valve_act(GAS_VALVE_OFF);
      delay(50);
      cur_pos = analogRead(GAS_VALVE_POS_ADC);
    }
    gas_valve_act(GAS_VALVE_STOP);
  }
  else if (valve_pos > GAS_VALVE_MAX_POS)
  {
    while (cur_pos < GAS_VALVE_MAX_POS)
    {
      gas_valve_act(GAS_VALVE_ON);
      delay(50);
      cur_pos = analogRead(GAS_VALVE_POS_ADC);
    }
    gas_valve_act(GAS_VALVE_STOP);
  }
  else
  {
    if (valve_pos > cur_pos)
    {
      while(valve_pos > cur_pos)
      {
        gas_valve_act(GAS_VALVE_ON);
        delay(50);
        cur_pos = analogRead(GAS_VALVE_POS_ADC);
      }
      gas_valve_act(GAS_VALVE_STOP);
    }
    else if (valve_pos < cur_pos)
    {
      while(valve_pos < cur_pos)
      {
        gas_valve_act(GAS_VALVE_OFF);
        delay(50);
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
  if (state == GAS_VALVE_ON)
  {
    digitalWrite(GAS_VALVE_POS_PIN,HIGH);
    digitalWrite(GAS_VALVE_NEG_PIN,LOW);
  }
  else if (state == GAS_VALVE_OFF)
  {
    digitalWrite(GAS_VALVE_POS_PIN,LOW);
    digitalWrite(GAS_VALVE_NEG_PIN,HIGH);
  }
  else
  {
    digitalWrite(GAS_VALVE_POS_PIN,LOW);
    digitalWrite(GAS_VALVE_NEG_PIN,LOW);
  }
}

//------------------------------------------------
//Audio control function:
//------------------------------------------------

void audio_cntrl(audio_states nxt_state)
{
  switch(nxt_state)
  {
    case audio_in_sig:
      digitalWrite(AUDIO_SEL_PIN,LOW);
      digitalWrite(AUDIO_DIS_PIN,LOW);
      break;
    case alarm_sig:
      digitalWrite(AUDIO_SEL_PIN,HIGH);
      digitalWrite(AUDIO_DIS_PIN,LOW);
      break;
    case audio_off:
      digitalWrite(AUDIO_SEL_PIN,LOW);
      digitalWrite(AUDIO_DIS_PIN,HIGH);
      break;
  }
}

//------------------------------------------------
