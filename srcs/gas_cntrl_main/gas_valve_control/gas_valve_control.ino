#define GAS_VALVE_POS_PIN   4
#define GAS_VALVE_NEG_PIN   5
#define GAS_VALVE_POS_ADC   5
#define GAS_VALVE_MAX_POS   950
#define GAS_VALVE_OFF_POS   350
#define GAS_VALVE_TYP_POS   600
#define GAS_VALVE_ON        1
#define GAS_VALVE_OFF       0
#define GAS_VALVE_STOP      2

void gas_valve_act(unsigned char state);
boolean gas_valve_cntrl(unsigned int valve_pos);

void setup() {
  // put your setup code here, to run once:
  //Serial port setup for debug during development:
  Serial.begin(9600);

  //Gas valve control pin setup:
  pinMode(GAS_VALVE_POS_PIN,OUTPUT);
  digitalWrite(GAS_VALVE_POS_PIN,LOW);
  pinMode(GAS_VALVE_NEG_PIN,OUTPUT);
  digitalWrite(GAS_VALVE_NEG_PIN,LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  gas_valve_cntrl(1010);
  Serial.println("Valve is 1010");
  Serial.print("Current sensor value is: ");
  Serial.print(analogRead(GAS_VALVE_POS_ADC));
  Serial.print("\n");
  delay(5000);
  gas_valve_cntrl(10);
  Serial.println("Valve is 10");
  Serial.print("Current sensor value is: ");
  Serial.print(analogRead(GAS_VALVE_POS_ADC));
  Serial.print("\n");
  delay(5000);
  gas_valve_cntrl(GAS_VALVE_TYP_POS);
  Serial.println("Valve is 800");
  Serial.print("Current sensor value is: ");
  Serial.print(analogRead(GAS_VALVE_POS_ADC));
  Serial.print("\n");
  delay(5000);
}

//-------------------------------------------------------

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

//---------------------------------------------------------

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
