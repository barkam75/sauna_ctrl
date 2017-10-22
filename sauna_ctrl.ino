/*
  Blink
 Turns on an LED on for one second, then off for one second, repeatedly.
 
 This example code is in the public domain.
 */
//#include<Serial.h>

// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;
unsigned long  tick=0;

//Temperature control variables

byte set_temp     = 100;  //Set temperature 100 deg C by default
byte ctrl_hist    = 5;    //Temperature regulation hysteresis band in degrees by default set to 5 degrees
byte stove_temp   = 0;    //Actual air temperature read from sensor above the heater
byte ceiling_temp = 0;    //Actual air temperature read from sensor on the ceiling
byte inlet_temp   = 0;    //Actual air temperature read from sensor on inlet
byte exhaust_temp = 0;    //Actual air temperature read from sensor on exhaust

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  Serial.begin(9600);  
  
}

void temp_ctrl() {
//  switch(
}

// the loop routine runs over and over again forever:
void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(50);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(50);               // wait for a second
  tick++;
  Serial.print("Tick count:");
  Serial.println(tick,DEC);
}

