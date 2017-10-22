/*
  Blink
 Turns on an LED on for one second, then off for one second, repeatedly.
 
 This example code is in the public domain.
 */
//#include<Serial.h>
float Version = 0.1;
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;
unsigned long  tick=0;


//Temperature control variables
byte stab_point   = 0;    //Air temp stabilisation point (0-Heater, 1-Ceiling, 3-Exhaust)
byte set_temp     = 100;  //Set temperature 100 deg C by default

byte ctrl_hist    = 5;    //Temperature regulation hysteresis band in degrees by default set to 5 degrees
byte stove_temp   = 0;    //Actual air temperature read from sensor above the heater
byte ceiling_temp = 0;    //Actual air temperature read from sensor on the ceiling
byte inlet_temp   = 0;    //Actual air temperature read from sensor on inlet
byte exhaust_temp = 0;    //Actual air temperature read from sensor on exhaust
byte heater_state = 0;    //Heater power (0-OFF, 1-ON)

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  Serial.begin(9600);  
  
}

void temp_ctrl() {
  byte temp_error; //Temperature error
  //Temperature measurement read
  temp_error = set_temp;
  switch(stab_point)
  {
    case 0: temp_error -= stove_temp;break;    //Stabilize heater air temperature
    case 1: temp_error -= ceiling_temp;break;  //Stabilize ceiling air temperature
    case 2: temp_error -= exhaust_temp;break;  //Stabilize exhaust air temperature
    default: temp_error = 2*-ctrl_hist;        //In case of error turn off the heater
  }
  if( heater_state == 0)
  {
    if( temp_error > ctrl_hist) heater_state = 1; //If temperature error is higher than hysteresis turn the heater ON
  }
  else
  {
    if( temp_error < -ctrl_hist) heater_state = 0; //If temperature error is higher than hysteresis turn the heater OFF
  }
}

void serial_report()
{
  Serial.print("Sauna controller report....\n");
  Serial.print("Software version:"); Serial.println(Version);
  Serial.print("Inlet temperature [deg C]:"); Serial.println(inlet_temp);
  Serial.print("Heater temperature [deg C]:"); Serial.println(stove_temp);
  Serial.print("Ceiling temperature [deg C]:"); Serial.println(ceiling_temp);
  Serial.print("Exhaust temperature [deg C]:"); Serial.println(exhaust_temp);
  Serial.print("Set temperature[deg C]:"); Serial.println(set_temp);
  Serial.print("Stabilisation point:"); if(stab_point==0){ Serial.println("Heater air");} else { if(stab_point==1){Serial.println("Ceiling air");}else{Serial.println("Exhaust air");}}
  Serial.print("Control hysteresis [deg C]:"); Serial.println(ctrl_hist);
  Serial.print("Heater");if(heater_state == 0 ){Serial.println("OFF");} else {Serial.println("OFF");}
  Serial.print("\n");
  
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
  tick++;
  serial_report();
  //Serial.print("Tick count:");
  Serial.println(tick,DEC);
}

