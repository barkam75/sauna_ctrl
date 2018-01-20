
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
#define ENCODER_RIGHT_PIN 10
#define ENCODER_LEFT_PIN 11
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
byte pixel_color = 10; //Set pixel color to white by default

//OneWire  ds(TEMP_SENSOR_PIN);
/*Menu - Browsing using wheel
 *Screen 1 -Default at boot  
 * Line 1 Time / Date
 * Line 2 Temperature
 * Line 3 Nothing
 * Line 4 Status line (S-SALT LAMP, C-COLOROTHERAPY)
 * 
 * Screen 2 - Colorotherapy
 * 
 */

 typedef struct 
 {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
 }RGB;

const char *color_table[] = {
                              "Red",
                              "Orange",
                              "Yellow",
                              "Green",
                              "Strong Green"
                              "Blue",
                              "Strong blue",
                              "Indigo",
                              "Purple",
                              "Pink",
                              "White"
};

 const RGB chromo_table[11] = {
                        {230,29,7},     //RED
                        {255,123,0},    //ORANGE
                        {255,248,82},   //YELLOW
                        {153,225,100},  //GREEN
                        {53,183,41},    //STRONG GREEN
                        {45,177,255},   //BLUE
                        {82,112,255},   //STRONG BLUE
                        {75,19,211},    //INDIGO
                        {159,66,230},   //PURPLE
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
  static byte pixel_cnt =0;
  pixels.setPixelColor(pixel_cnt, pixels.Color(chromo_table[pixel_color].red,
                                               chromo_table[pixel_color].green,
                                               chromo_table[pixel_color].blue)
                       );
  pixels.show();
  if(pixel_cnt < NUMPIXELS )
  {
    pixel_cnt ++;
  }
  else
  {
    pixel_cnt = 0;
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
  lcd.print(temp,DEC);
  lcd.setCursor(0,2);
  lcd.print(tick);

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
  analogWrite(SALT_LAMP_PIN, 10 );
  // initialize the digital pin as an output.
  ui_setup();
  DBG_INFO("Boot completed");
}


// the loop routine runs over and over again forever:
void loop() {
//  digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(1000);               // wait for a second
 // digitalWrite(LED_PIN, LOW);    // turn the LED off by making the voltage LOW
//  delay(1000);               // wait for a second
  tick++;
  serial_report();
  //Serial.print("Tick count:");
  Serial.println(tick,DEC);
//  pixels.setPixelColor(10, pixels.Color(0,150,0)); // Moderately bright green color.
//  pixels.show(); // This sends the updated pixel 
  temp_sensor_loop();
  ui_loop();
}

