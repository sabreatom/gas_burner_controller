//------------------------------------------------
//Includes:
--------------------------------------------------



//------------------------------------------------
//Defines:
--------------------------------------------------

#define AIR_SENSOR_0    0
#deifne AIR_SENSOR_1    1
#define AIR_Q_THRES     150

#define FLAME_CH0       2
#define FLAME_CH0       3
#define FLAME_CH0       4
#define FLAME_THRES     50

//------------------------------------------------
//Global variables:
//------------------------------------------------



//-----------------------------------------------
//Setup function:
//-----------------------------------------------

void setup() {

}

//-----------------------------------------------
//Main loop function:
//-----------------------------------------------

void loop() {

}

//------------------------------------------------
//Check air quality:
//------------------------------------------------

boolean chck_air(adc_ch,air_thres)
{
  int data;

  data = analogRead(adc_ch);

  if (data <= air_thres)
  {
    return false;
  }
  else
  {
    return true;
  }
}

//------------------------------------------------
//Check flame detection sensor:
//------------------------------------------------

boolean chck_flame(f_ch,f_thres)
{
  int data;

  data = analogRead(f_ch);

  if (data <= f_thres)
  {
    return false;
  }
  else
  {
    return true;
  }
}

//------------------------------------------------
//Control ignition spark:
//------------------------------------------------



//------------------------------------------------
