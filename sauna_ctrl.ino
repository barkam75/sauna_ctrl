
/*To do
1-LCD - OK
2-NeoPixel -OK
3-Salt
*/

#define _DBG_NFO
#ifdef _DBG_NFO
#define DBG_INFO(text) Serial.println(text);
#else
#define DBG_INFO(text)
#endif

#include <Wire.h>
#include <OneWire.h>   
#include <LiquidCrystal_I2C.h>
#include <Adafruit_NeoPixel.h>

float Version = 0.1;

//Sauna controller pinout configuration
#define AUDIO_LEFT_CH_RELAY_PIN 4
#define AUDIO_RIGHT_CH_RELAY_PIN 7
#define SALT_LAMP_PIN 9
#define TEMP_SENSOR_PIN 12
#define LED_STRING_PIN 6
#define ENCODER_RIGHT_PIN 11
#define ENCODER_LEFT_PIN 5
#define ENCODER_PUSH 4
#define LED_PIN 13


#define TEMP_SENSOR_DELAY 100

//Other configuration
#define NUMPIXELS 144
#define HEATER 4

#define DS1820_ADDR 0x2855A3E308 00 00 42


LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_STRING_PIN, NEO_GRB + NEO_KHZ800);
OneWire temp_sensor(10);
byte temp_sensor_addr[] = {0x28,0x55,0xA3,0xE3,0x08,00,00,0x42};
byte temp_data[12];
byte pixel_color = 11; //Set pixel color to white by default
byte pixel_color_prev = 0;

//OneWire  ds(TEMP_SENSOR_PIN);
/*Connections:
Temp sensor cable: RED=+5V, YELLOW=Q, BLACK=GND
Display cable:GREEN=+5V, GRAY=GND, WHITE=SDA, BROWN=SCL
Encoder cable:WHITE-GREEN=S3, GREEN=S2, WHITE-BLACK=S1B, BLACK=S1A, BLUE=GND, RED=+5V 
 *
 */

 typedef struct 
 {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
 }RGB;

const char *color_table[] = { "OFF",
                              "Red",
                              "Orange",
                              "Yellow",
                              "Green",
                              "Strong Green",
                              "Blue",
                              "Strong blue",
                              "Indigo",
                              "Purple",
                              "Pink",
                              "White",
                              "Error"
};

 const RGB chromo_table[12] = {
                        {0,0,0},
                        {255,0,0},     //RED
                        {255,165,0},    //ORANGE
                        {255,255,0},   //YELLOW
                        {153,225,100},  //GREEN
                        {0,255,0},    //STRONG GREEN
                        {45,177,255},   //BLUE
                        {0,0,255},   //STRONG BLUE
                        {75,0,130},    //INDIGO
                        {128,0,128},   //PURPLE
                        {255,126,211},  //PINK
                        {255,255,255}   //WHITE
 };
 

unsigned long  tick=0;

//Temperature control variables
float stove_temp   = 0.0;    //Actual air temperature read from sensor above the heater

void pin_setup()
{
  //Setup Relay pins
  pinMode(AUDIO_LEFT_CH_RELAY_PIN, OUTPUT );
  pinMode(AUDIO_RIGHT_CH_RELAY_PIN, OUTPUT );
  digitalWrite(AUDIO_LEFT_CH_RELAY_PIN, HIGH );
  digitalWrite(AUDIO_RIGHT_CH_RELAY_PIN, HIGH );
  pinMode(LED_PIN, OUTPUT);
  pinMode(ENCODER_RIGHT_PIN, INPUT);
  pinMode(ENCODER_LEFT_PIN, INPUT);
  pinMode(ENCODER_PUSH, INPUT);
}

void temp_sensor_setup()
{
  
}

void temp_sensor_loop()
{
  enum temp_states{TEMP_INIT,TEMP_CONV, TEMP_READ};
  static temp_states temp_state=TEMP_INIT;  
  static unsigned long time_stamp;
  byte cnt;
  switch( temp_state )
  {
    case TEMP_INIT:{
      temp_sensor.reset();
      temp_sensor.select(temp_sensor_addr);
      temp_sensor.write(0x44, 1);        // start conversion, with parasite power on at the end
      time_stamp = millis();
      temp_state = TEMP_CONV;
    };break;
    case TEMP_CONV:{
      if((millis()-time_stamp) >= 999 )
         temp_state = TEMP_READ;
    };break;
    case TEMP_READ:{
       temp_sensor.reset();
      temp_sensor.select(temp_sensor_addr);    
      temp_sensor.write(0xBE);         // Read Scratchpad
      for ( cnt = 0; cnt < 9; cnt++)             // we need 9 bytes
        temp_data[cnt] = temp_sensor.read();

      int16_t raw = (temp_data[1] << 8) | temp_data[0];
      byte cfg = (temp_data[4] & 0x60);
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      //// default is 12 bit resolution, 750 ms conversion time
      stove_temp = (float)raw / 16.0;
      temp_state = TEMP_INIT;
      };break;
      default:{};break;
  }
 
}

void pixel_setup()
{
    pixels.begin();
}

void pixel_loop()
{
  static byte prev_color=0;
  byte cnt;
  if(pixel_color != prev_color)
  {
    for(cnt=0; cnt<NUMPIXELS; cnt++ )
    pixels.setPixelColor(cnt, pixels.Color(    chromo_table[pixel_color].red,
                                             chromo_table[pixel_color].green,
                                             chromo_table[pixel_color].blue)
                       );
    pixels.show();
    prev_color = pixel_color;
  }
}

void ui_setup()
{
  //Initialize LCD
   lcd.begin(20,4);
  //Initialize Serial port 
  Serial.begin(9600);  
  lcd.backlight();   //Backlight on 
  lcd.setCursor(0,0); //Set cursor position to 0,0
  lcd.print("Sterownik sauny");
}

void ui_loop()
{
  byte temp=stove_temp;
  unsigned long time_stamp;
  lcd.setCursor(0,1);
  lcd.print("Temperatura:");
  lcd.print(stove_temp);
  lcd.print((char)223);
  lcd.print( "C" );
  lcd.setCursor(0,2);
 // lcd.print(tick);
  if(pixel_color != pixel_color_prev )
  {
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("Kolor:");
    lcd.print(color_table[pixel_color]);
    pixel_color_prev = pixel_color;
  }
}

void encoder_loop()
{
 if(digitalRead(ENCODER_RIGHT_PIN) == 1)  
    {
      if(pixel_color<11) pixel_color++;
    } 
 if(digitalRead(ENCODER_LEFT_PIN) == 1)
    {
      if(pixel_color>0) pixel_color--;
    }
}

void serial_report()
{
  Serial.print("Sauna controller report....\n");
  Serial.print("Software version:"); Serial.println(Version);
  Serial.print("Heater temperature [deg C]:"); Serial.println(stove_temp);
  Serial.print("\n");
}

// the setup routine runs once when you press reset:
void setup() {         
  pin_setup();  
  analogWrite(SALT_LAMP_PIN, 255 );
  // initialize the digital pin as an output.
  ui_setup();
  pixel_setup();
  DBG_INFO("Boot completed");
}

void blink_loop()
{
  static unsigned long prv_time = 0;
  static byte shift=0;
  static int pattern=0x0440;
  byte test;
  if((millis()-prv_time)>20)
  {
    test = (pattern & 0x1) == 1 ? HIGH:LOW;
    digitalWrite(LED_PIN, test);
    if(shift < 15 )
    {
      pattern = pattern >> 1;
      shift++;
    }
    else
    {
      pattern =0x0440;
      shift = 0;
    }
    prv_time = millis();
  }
  
}


// the loop routine runs over and over again forever:
void loop() {
  //digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);               // wait for a second
 // digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);               // wait for a second
  tick++;
  Serial.println(tick,DEC);
  temp_sensor_loop();
  ui_loop();
  encoder_loop();
  pixel_loop();
  blink_loop();
}

