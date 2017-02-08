/* MPL3115A2 Example Code
 by: Kris Winer adapted from skeleton codes by Jim Lindblom, A. Weiss, and Nathan Seidle (Thanks guys!)
 Modified: February 12, 2014 by Kris Winer
 Original date: November 17, 2011, Jim Lindblom, SparkFun Electronics
 
 IDE example usage for most features of the MPL3115A2 I2C Precision Altimeter.
 
 Basic functions are implemented including absolute pressure (50 to 110 kPa), altimeter pressure (mmHg), 
 altitude (meters or feet), and temperature (-40 to 85 C).
 
 In addition, provision for the FIFO mode in the initialization, watermark/overflow setting, and register
 read functions for autonomous data logging over as many as nine hours with 32 data samples of P, T.
 
 Hardware setup:
 MPL3115A2 Breakout ------------ Arduino Mini Pro 3.3 V
 3.3V --------------------- 3.3V
 SDA ----------------------- A4
 SCL ----------------------- A5
 INT2 ---------------------- D4
 INT1 ---------------------- D5
 GND ---------------------- GND
 
 SDA and SCL should have external pull-up resistors (to 3.3V) if using a 5 V Arduino.
 They should be on the MPL3115A2 SparkFun breakout board.
 I didn't need any for the 3.3 V Pro Mini.
 
 Jim Lindblom's Note: The MMA8452 is an I2C sensor, however this code does
 not make use of the Arduino Wire library. Because the sensor
 is not 5V tolerant, we can't use the internal pull-ups used
 by the Wire library. Instead use the included i2c.h, defs.h and types.h files.
 
 The MPL3115A2 in this sketch is interfaced with a Pro Mini operating at 3.3 V, so we could use the
 Arduino Wire library. However, I will follow Jim Lindblom's example.
 
 The MPL3115A2 has internal First in/First out data storage enabling autonomous data logging for up to 32 samples
 of pressure/altitude and temperature. I programmed the Pro Mini through a FTDI Basic interface board and noticed
 every time I hooked it up to the Arduino/Sensor or opened a serial monitor the devices were re-initialized, 
 defeating the autonomous logging function.

 This was remedied by placing a 10 uF capacitor between reset and ground before either reconnecting the sensor 
 through the FTDI board or opening a serial monitor. Either event drives the reset low, the capacitor keeps the
 reset high long enough to avoid resetting. Of course, the capacitor must be removed when uploading a new or updated 
 sketch or an error will be generated.  

 I got this idea from:
 http://electronics.stackexchange.com/questions/24743/arduino-resetting-while-reconnecting-the-serial-terminal
 where there is a little more discussion. Thanks 0xAKHIL!
 
 Lastly, I put a piece of porous foam over the sensor to block ambient light since thre is some indication the 
 pressure and altitude reading are light sensitive.
 
 */

#include "i2c.h"  // not the wire library, can't use pull-ups
#include <Adafruit_CharacterOLED.h> //include the OLED library to control LCD

Adafruit_CharacterOLED lcd(OLED_V2, 6, 7, 8, 9, 10, 11, 12); // initialize the LCD library with the numbers of the interface pins

// make a custom character for degree symbol:
byte degree[8] = {
  0b00100,
  0b01010,
  0b00100,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// make a custom character for Mercury symbol:
byte mercury[8] = {
  0b10100,
  0b11100,
  0b10100,
  0b10100,
  0b00011,
  0b00011,
  0b00001,
  0b00011
};

// Standard 7-bit I2C slave address is 1100000 = 0x60, 8-bit read address is 0xC1, 8-bit write is 0xC0
#define MPL3115A2_ADDRESS 0x60  // SA0 is high, 0x1C if low

// Register defines courtesy A. Weiss and Nathan Seidle, SparkFun Electronics
#define STATUS     0x00
#define OUT_P_MSB  0x01
#define OUT_P_CSB  0x02
#define OUT_P_LSB  0x03
#define OUT_T_MSB  0x04
#define OUT_T_LSB  0x05
#define DR_STATUS  0x06
#define OUT_P_DELTA_MSB  0x07
#define OUT_P_DELTA_CSB  0x08
#define OUT_P_DELTA_LSB  0x09
#define OUT_T_DELTA_MSB  0x0A
#define OUT_T_DELTA_LSB  0x0B
#define WHO_AM_I   0x0C
#define F_STATUS   0x0D
#define F_DATA     0x0E
#define F_SETUP    0x0F
#define TIME_DLY   0x10
#define SYSMOD     0x11
#define INT_SOURCE 0x12
#define PT_DATA_CFG 0x13
#define BAR_IN_MSB 0x14 // Set at factory to equivalent sea level pressure for measurement location, generally no need to change
#define BAR_IN_LSB 0x15 // Set at factory to equivalent sea level pressure for measurement location, generally no need to change
#define P_TGT_MSB  0x16
#define P_TGT_LSB  0x17
#define T_TGT      0x18
#define P_WND_MSB  0x19
#define P_WND_LSB  0x1A
#define T_WND      0x1B
#define P_MIN_MSB  0x1C
#define P_MIN_CSB  0x1D
#define P_MIN_LSB  0x1E
#define T_MIN_MSB  0x1F
#define T_MIN_LSB  0x20
#define P_MAX_MSB  0x21
#define P_MAX_CSB  0x22
#define P_MAX_LSB  0x23
#define T_MAX_MSB  0x24
#define T_MAX_LSB  0x25
#define CTRL_REG1  0x26
#define CTRL_REG2  0x27
#define CTRL_REG3  0x28
#define CTRL_REG4  0x29
#define CTRL_REG5  0x2A
#define OFF_P      0x2B
#define OFF_T      0x2C
#define OFF_H      0x2D

// Define Device inputs
// Integer values between 0 < n < 7 give oversample ratios 2^n and 
// sampling intervals of 0=6 ms , 1=10, 2=18, 3=34, 4=66, 5=130, 6=258, and 7=512 
const byte SAMPLERATE = 7;  // maximum oversample = 7
// Set time between FIFO data points from 1 to 16328 s (For some reason I get a spurious result for ST_Value = 15) 
const byte ST_VALUE = 1; // Set auto time step (2^ST_VALUE) seconds
int FIFOon = 1; // Choose realtime data acquisition or FIFO delayed data acquisition; default is real time 
int AltimeterMode = 0; // use to choose between altimeter and barometer modes for FIFO data

// Define device outputs
float altitude = 0.;
float pressure = 0.;
float temperature = 0.;

// Interrupt Pin definitions
const int int1Pin = 4;  // New data available interrupt
// Interrupt events: FIFO, Pressure window, temperature window, pressure threshold, 
// temperature threshold, pressure change, and temperature change set in CNTL_REG5
const int int2Pin = 5;  
const int readoutPin = 2; // We will allow FIFO data to be serial printed when the readout pin is set to high

void setup()
{

  Serial.begin(9600);

  // create a degree, mercury symbol characters
  lcd.createChar(4, degree);
  lcd.createChar(5, mercury); 

  lcd.begin(16, 2);// Initialize the LCD with 16 characters and 2 lines
    
  // Read the WHO_AM_I register, this is a good test of communication
  byte c = readRegister(WHO_AM_I);  // Read WHO_AM_I register
  if (c == 0xC4) // WHO_AM_I should always be 0xC4
  {  

    MPL3115A2Reset();  // Start off by resetting all registers to the default
    
    // Set up the interrupt pins, they're set as active high, push-pull
    pinMode(int1Pin, INPUT);
    digitalWrite(int1Pin, LOW); // Initialize interrupt pins to LOW
    pinMode(int2Pin, INPUT);
    digitalWrite(int2Pin, LOW);// Initialize interrupt pins to LOW
    pinMode(readoutPin, INPUT);
    digitalWrite(readoutPin, LOW);// Initialize readout pin to LOW
    
    SampleRate(SAMPLERATE); // Set oversampling rate
    Serial.print("Oversampling Rate is "); Serial.println(SAMPLERATE);
    TimeStep(ST_VALUE); // Set data update interval
    Serial.print("Data update interval is "); Serial.print((unsigned long) (1 << ST_VALUE)); Serial.println(" seconds");
    MPL3115A2enableEventflags();
    Serial.println("MPL3115A2 event flags enabled...");

  }
  else
  {
    Serial.print("Could not connect to MPL3115A2: 0x");
    Serial.println(c, HEX);
    while(1) ; // Loop forever if communication doesn't happen
  }
}

void loop()
{  
  
// Currently configured to continuously log data if FIFOon = 1; the FIFO buffer is read and output to a serial monitor
// when digital pin 2 is momentarily set HIGH. The data begins accumulating again after the data read. In the
// initFIFOMPL3115A2() function, the overflow interrupt can be set instead of watermark, then the data log only
// occurs once until the overflow interrupt is triggered on FIFO data register full condition; for some reason, this happens
// after reading 31 not 32 data points.

if(FIFOon == 1) {

   initFIFOMPL3115A2();  // initialize the accelerometer for delayed (FIFO) acquisition if communication is OK
   Serial.println("MPL3115A2 FIFO data acquisition active...");
   if(AltimeterMode) {ActiveAltimeterMode(); Serial.println("Active Altimeter Mode");} // Choose either barometer or altimeter mode
   else {ActiveBarometerMode(); Serial.println("Active Barometer Mode");}
 
   byte rawData[160];

// This is similar to the interrupt method in the active data acquisition routine
// Watermark mode should continuously log data and download to serial monitor after readoutPin set HIGH
// Overflow mode should write out the data just once, when the 32 samples have been acquired
// Sensor presumably takes one data read at start; can wait for other 31 data reads before checking interrupts
//
      unsigned long count = 1 << ST_VALUE; // Use unsigned long because this number can get quite large!
      unsigned long i = 0;
   for (i=0; i < 31*count; i++) {
     lcd.setCursor(0,0); lcd.print("Wait ");
     lcd.setCursor(6,0); lcd.print((31*count - i)); lcd.print("s    "); 
     lcd.setCursor(0,1); lcd.print("in FIFO mode    "); 
     delay(1000);
   }
   while(digitalRead(int2Pin) == LOW); // Wait for interrupt event on intPin2
   while ((readRegister(INT_SOURCE) & 0x40) == 0); Serial.println("Interrupt on FIFO source"); // Wait for interrupt source = FIFO on bit 6
   while (readRegister(F_STATUS) & 0x80 | 0x40 == 0);  // Should clear the INT_SOURCE bit
   Clock(); // capture time overflow condition reached
   lcd.setCursor(0,1); lcd.print("Data pts = "); lcd.print((int) (readRegister(F_STATUS)<<2)/4); // Print number of data points successfully acquired
   
// We will allow delayed read of FIFO registers by requiring digital pin to go HIGH before read;
// This is useful for autonomous data acquisition applications where the data is collected at one time
// and downloaded to the serial monitor or other output device at another time. This is a poor man's data logger!
   while(digitalRead(readoutPin) == LOW); // Wait for the readout pin to go momentarily HIGH before data read
 
   unsigned long c = readRegister(TIME_DLY); // Check how much time has elapsed since last write; useful for overflow (read once) mode
   Serial.print("Time since last write = "); Serial.print(c*count); Serial.println(" seconds"); 
 
   readRegisters(F_DATA, 160, &rawData[0]); // If overflow reached, dump the FIFO data registers
        
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Process the FIFO data buffer contents and print to serial monitor
// Convert all the raw FIFO data to pressure/altitude and temperature       
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int j;
  for(j = 0; j < 160; j+=5) {

// Pressure/Altitude bytes
  byte msb = rawData[j];
  byte csb = rawData[j+1];
  byte lsb = rawData[j+2];
// Temperature bytes
  byte msbT = rawData[j+3];
  byte lsbT = rawData[j+4]; 
 
  if(AltimeterMode) {
 // Calculate altitude, check for negative sign in altimeter data
 long foo = 0;
 if(msb > 0x7F) {
   foo = ~((long)msb << 16 | (long)csb << 8 | (long)lsb) + 1; // 2's complement the data
   altitude = (float) (foo >> 8) + (float) ((lsb >> 4)/16.0); // Whole number plus fraction altitude in meters for negative altitude
   altitude *= -1.;
 }
 else {
   foo = ((msb << 8) | csb);
   altitude = (float) (foo) + (float) ((lsb >> 4)/16.0);  // Whole number plus fraction altitude in meters
 }
 }
  else {
  long pressure_whole =  ((long)msb << 16 | (long)csb << 8 | (long)lsb) ; // Construct whole number pressure
  pressure_whole >>= 6;
 
  lsb &= 0x30; 
  lsb >>= 4;
  float pressure_frac = (float) lsb/4.0;

  pressure = (float) (pressure_whole) + pressure_frac; 
   }
   
// Calculate temperature, check for negative sign
long foo = 0;
if(msbT > 0x7F) {
 foo = ~(msbT << 8 | lsbT) + 1 ; // 2's complement
 temperature = (float) (foo >> 8) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
 temperature *= -1.;
 }
 else {
   temperature = (float) (msbT) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
 }
  
// Output data array to serial printer; comma delimits useful for importing into excel spreadsheet
 Serial.print("Time ,"); Serial.print((j/5)*(1<<ST_VALUE)); Serial.print(", seconds");
 Serial.print(", Temperature = ,"); Serial.print(temperature, 1); Serial.print(", C,");
 if(AltimeterMode) {Serial.print(" Altitude = ,"); Serial.print(altitude, 1); Serial.println(", m");}
 else {Serial.print(" Pressure = ,"); Serial.print(pressure/1000., 2); Serial.println(", kPa");}

 }
}
 
else {
  
    initRealTimeMPL3115A2();  // initialize the accelerometer for realtime data acquisition if communication is OK
    Serial.println("MPL3115A2 realtime data acquisition active...");
  
  // Toggle beteen Active Altimeter Mode and Active Barometer Mode
   
    ActiveAltimeterMode();
    Serial.println("Active Altimeter Mode");
    readAltitude();  // Read the altitude

    Serial.print("altitude is ");
    Serial.print(altitude, 2);  // Print altitude in meters
    Serial.print(" m");
    Serial.print("  temperature is ");
    Serial.print(temperature, 2);  // Print altitude in meters
    Serial.print(" C");
    Serial.println(); 

  int i = 0;
  
  for(i = 0; i <= 9; i++) {
    if (i <= 4) { //delay set to 1 sec in pressure reads; will toggle after 5 sec  Clock(); // get current time
   Clock(); // get current time
   lcd.setCursor(0,1);
   lcd.print((altitude*3.28083989), 0) ; lcd.print(" ft");
   lcd.setCursor(10,1);
   lcd.print((temperature*(9./5.) + 32.), 1) ; lcd.write(4); lcd.print("F");
   delay(500);
               }
    else {
   Clock(); // get current time
   lcd.setCursor(0,1);
   lcd.print(altitude, 1) ; lcd.print(" m");
   lcd.setCursor(10,1);
   lcd.print(temperature, 1) ; lcd.write(4); lcd.print("C");
   delay(500);
         }
                }
                
    ActiveBarometerMode();
    Serial.println("Active Barometer Mode");
    readPressure();  // Read the pressure
    
  const int station_elevation_m = 1050.0*0.3048; // Accurate for the roof on my house; convert from feet to meters

  float baroin  = pressure/100; //pressure is now in millibars

  // Formula to correct absolute pressure in millbars to "altimeter pressure" in inches of mercury 
  // comparable to weather report pressure
  float part1 = baroin - 0.3; //Part 1 of formula
  const float part2 = 8.42288 / 100000.0;
  float part3 = pow((baroin - 0.3), 0.190284);
  float part4 = (float)station_elevation_m / part3;
  float part5 = (1.0 + (part2 * part4));
  float part6 = pow(part5, (1.0/0.190284));
  float altimeter_setting_pressure_mb = part1 * part6; //Output is now in adjusted millibars
  baroin = altimeter_setting_pressure_mb * 0.02953;
  
    Serial.print("pressure is ");
    Serial.print(pressure, 2);
    Serial.print(" Pa");  // Print altitude in meters
    Serial.print("  temperature is ");
    Serial.print(temperature, 2);  // Print altitude in meters
    Serial.print(" C");
    Serial.println(); 

  i = 0;
  
  for(i = 0; i <= 9; i++) {
    if (i <= 4) { //delay set to 1 sec in pressure reads; will toggle after 5 sec  Clock(); // get current time
   Clock(); // get current time
   lcd.setCursor(0,1);
   lcd.print((pressure/1000.), 2) ; lcd.print(" kPa");
   lcd.setCursor(10,1);
   lcd.print(temperature, 1) ; lcd.write(4); lcd.print("C");
   delay(500);
               }
    else {
   Clock(); // get current time
   lcd.setCursor(0,1);
   lcd.print(baroin, 2) ; lcd.print(" in"); lcd.write(5);
   lcd.setCursor(10,1);
   lcd.print((temperature*(9./5.) + 32.), 1) ; lcd.write(4); lcd.print("F");
   delay(500);
         }
                }
}
}

void readAltitude() // Get altitude in meters and temperature in centigrade
{
  byte rawData[5];  // msb/csb/lsb pressure and msb/lsb temperature stored in five contiguous registers 

// We can read the data either by polling or interrupt; see data sheet for relative advantages
// First we try hardware interrupt, which should take less power, etc.
 while (digitalRead(int1Pin) == LOW); // Wait for interrupt pin int1Pin to go HIGH
 digitalWrite(int1Pin, LOW);  // Reset interrupt pin int1Pin
 while((readRegister(INT_SOURCE) & 0x80) == 0); // Check that the interrupt source is a data ready interrupt
// or use a polling method
// Check data read status; if PTDR (bit 4) not set, then
// toggle OST bit to cause sensor to immediately take a reading
// Setting the one shot toggle is the way to get faster than 1 Hz data read rates
// while ((readRegister(STATUS) & 0x08) == 0); // toggleOneShot(); 
  
  readRegisters(OUT_P_MSB, 5, &rawData[0]);  // Read the five raw data registers into data array

// Altutude bytes-whole altitude contained defined by msb, csb, and first two bits of lsb, fraction by next two bits of lsb
  byte msbA = rawData[0];
  byte csbA = rawData[1];
  byte lsbA = rawData[2];
// Temperature bytes
  byte msbT = rawData[3];
  byte lsbT = rawData[4];
 
 // Calculate altitude, check for negative sign in altimeter data
 long foo = 0;
 if(msbA > 0x7F) {
   foo = ~((long)msbA << 16 | (long)csbA << 8 | (long)lsbA) + 1; // 2's complement the data
   altitude = (float) (foo >> 8) + (float) ((lsbA >> 4)/16.0); // Whole number plus fraction altitude in meters for negative altitude
   altitude *= -1.;
 }
 else {
   altitude = (float) ( (msbA << 8) | csbA) + (float) ((lsbA >> 4)/16.0);  // Whole number plus fraction altitude in meters
 }

// Calculate temperature, check for negative sign
if(msbT > 0x7F) {
 foo = ~(msbT << 8 | lsbT) + 1 ; // 2's complement
 temperature = (float) (foo >> 8) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
 temperature *= -1.;
 }
 else {
   temperature = (float) (msbT) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
 }
}

void readPressure()
{
  byte rawData[5];  // msb/csb/lsb pressure and msb/lsb temperature stored in five contiguous registers
 

// We can read the data either by polling or interrupt; see data sheet for relative advantages
// First we try hardware interrupt, which should take less power, etc.
 while (digitalRead(int1Pin) == LOW); // Wait for interrupt pin int1Pin to go HIGH
 digitalWrite(int1Pin, LOW);  // Reset interrupt pin int1Pin
 while((readRegister(INT_SOURCE) & 0x80) == 0); // Check that the interrupt source is a data ready interrupt
// or use a polling method
// Check data read status; if PTDR (bit 4) not set, then
// toggle OST bit to cause sensor to immediately take a reading
// Setting the one shot toggle is the way to get faster than 1 Hz data read rates
// while ((readRegister(STATUS) & 0x08) == 0); // toggleOneShot(); 
 
  readRegisters(OUT_P_MSB, 5, &rawData[0]);  // Read the five raw data registers into data array

// Pressure bytes
  byte msbP = rawData[0];
  byte csbP = rawData[1];
  byte lsbP = rawData[2];
// Temperature bytes
  byte msbT = rawData[3];
  byte lsbT = rawData[4]; 
 
  long pressure_whole =   ((long)msbP << 16 |  (long)csbP << 8 |  (long)lsbP) ; // Construct whole number pressure
  pressure_whole >>= 6;
 
  lsbP &= 0x30; 
  lsbP >>= 4;
  float pressure_frac = (float) lsbP/4.0;

  pressure = (float) (pressure_whole) + pressure_frac; 

// Calculate temperature, check for negative sign
long foo = 0;
if(msbT > 0x7F) {
 foo = ~(msbT << 8 | lsbT) + 1 ; // 2's complement
 temperature = (float) (foo >> 8) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
 temperature *= -1.;
 }
 else {
   temperature = (float) (msbT) + (float)((lsbT >> 4)/16.0); // add whole and fractional degrees Centigrade
 }
}

/*
=====================================================================================================
Define functions according to 
"Data Manipulation and Basic Settings of the MPL3115A2 Command Line Interface Drive Code"
by Miguel Salhuana
Freescale Semiconductor Application Note AN4519 Rev 0.1, 08/2012
=====================================================================================================
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Clears then sets OST bit which causes the sensor to immediately take another reading
void toggleOneShot()
{
    MPL3115A2Active();  // Set to active to start reading
    byte c = readRegister(CTRL_REG1);
    writeRegister(CTRL_REG1, c & ~(1<<1)); // Clear OST (bit 1)
    c = readRegister(CTRL_REG1);
    writeRegister(CTRL_REG1, c | (1<<1)); // Set OST bit to 1
}
    
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set the Outputting Sample Rate
void SampleRate(byte samplerate)
{
  MPL3115A2Standby();  // Must be in standby to change registers

  byte c = readRegister(CTRL_REG1);
  writeRegister(CTRL_REG1, c & ~(0x38)); // Clear OSR bits 3,4,5
  if(samplerate < 8) { // OSR between  and 7
  writeRegister(CTRL_REG1, c | (samplerate << 3));  // Write OSR to bits 3,4,5
  }
  
  MPL3115A2Active();  // Set to active to start reading
 }
 
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Initialize the MPL3115A2 registers for FIFO mode
void initFIFOMPL3115A2()
{
  // Clear all interrupts by reading the data output registers
  byte temp;
  temp = readRegister(OUT_P_MSB);
  temp = readRegister(OUT_P_CSB);
  temp = readRegister(OUT_P_LSB);
  temp = readRegister(OUT_T_MSB);
  temp = readRegister(OUT_T_LSB);
  temp = readRegister(F_STATUS);
  
   MPL3115A2Standby();  // Must be in standby to change registers
  
  // Set CTRL_REG4 register to configure interupt enable
  // Enable data ready interrupt (bit 7), enable FIFO (bit 6), enable pressure window (bit 5), temperature window (bit 4),
  // pressure threshold (bit 3), temperature threshold (bit 2), pressure change (bit 1) and temperature change (bit 0)
  writeRegister(CTRL_REG4, 0x40);  // enable FIFO
  
  //  Configure INT 1 for data ready, all other (inc. FIFO) interrupts to INT2
  writeRegister(CTRL_REG5, 0x80); 
  
  // Set CTRL_REG3 register to configure interupt signal type
  // Active HIGH, push-pull interupts INT1 and INT 2
  writeRegister(CTRL_REG3, 0x22); 
  
  // Set FIFO mode
  writeRegister(F_SETUP, 0x00); // Clear FIFO mode
//  writeRegister(F_SETUP, 0x80); // Set F_MODE to interrupt when overflow = 32 reached
  writeRegister(F_SETUP, 0x60); // Set F_MODE to accept 32 data samples and interrupt when watermark = 32 reached

  MPL3115A2Active();  // Set to active to start reading
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Initialize the MPL3115A2 for realtime data collection 
void initRealTimeMPL3115A2()
{
  // Clear all interrupts by reading the data output registers
  byte temp;
  temp = readRegister(OUT_P_MSB);
  temp = readRegister(OUT_P_CSB);
  temp = readRegister(OUT_P_LSB);
  temp = readRegister(OUT_T_MSB);
  temp = readRegister(OUT_T_LSB);
  temp = readRegister(F_STATUS);
  
   MPL3115A2Standby();  // Must be in standby to change registers
  
  // Set CTRL_REG4 register to configure interupt enable
  // Enable data ready interrupt (bit 7), enable FIFO (bit 6), enable pressure window (bit 5), temperature window (bit 4),
  // pressure threshold (bit 3), temperature threshold (bit 2), pressure change (bit 1) and temperature change (bit 0)
  writeRegister(CTRL_REG4, 0x80);  
  
  //  Configure INT 1 for data ready, all other interrupts to INT2
  writeRegister(CTRL_REG5, 0x80); 
  
  // Set CTRL_REG3 register to configure interupt signal type
  // Active HIGH, push-pull interupts INT1 and INT 2
  writeRegister(CTRL_REG3, 0x22); 
  
  // Set FIFO mode
  writeRegister(F_SETUP, 0x00); // disable FIFO mode
  
  MPL3115A2Active();  // Set to active to start reading
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set the Auto Acquisition Time Step
void TimeStep(byte ST_Value)
{
 MPL3115A2Standby(); // First put device in standby mode to allow write to registers
 
 byte c = readRegister(CTRL_REG2); // Read contents of register CTRL_REG2
 if (ST_Value <= 0xF) {
 writeRegister(CTRL_REG2, (c | ST_Value)); // Set time step n from 0x0 to 0xF (bits 0 - 3) for time intervals from 1 to 32768 (2^n) seconds
 }
 
 MPL3115A2Active(); // Set to active to start reading
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Enable the pressure and temperature event flags
 // Bit 2 is general data ready event mode on new Pressure/Altitude or temperature data
 // Bit 1 is event flag on new Pressure/Altitude data
 // Bit 0 is event flag on new Temperature data
void MPL3115A2enableEventflags()
{
  MPL3115A2Standby();  // Must be in standby to change registers
  writeRegister(PT_DATA_CFG, 0x07); //Enable all three pressure and temperature event flags
  MPL3115A2Active();  // Set to active to start reading
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Enter Active Altimeter mode
void ActiveAltimeterMode()
{
 MPL3115A2Standby(); // First put device in standby mode to allow write to registers
 byte c = readRegister(CTRL_REG1); // Read contents of register CTRL_REG1
 writeRegister(CTRL_REG1, c | (0x80)); // Set ALT (bit 7) to 1
 MPL3115A2Active(); // Set to active to start reading
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Enter Active Barometer mode
void ActiveBarometerMode()
{
 MPL3115A2Standby(); // First put device in standby mode to allow write to registers
 byte c = readRegister(CTRL_REG1); // Read contents of register CTRL_REG1
 writeRegister(CTRL_REG1, c & ~(0x80)); // Set ALT (bit 7) to 0
 MPL3115A2Active(); // Set to active to start reading
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Software resets the MPL3115A2.
// It must be in standby to change most register settings
void MPL3115A2Reset()
{
  writeRegister(CTRL_REG1, (0x04)); // Set RST (bit 2) to 1
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sets the MPL3115A2 to standby mode.
// It must be in standby to change most register settings
void MPL3115A2Standby()
{
  byte c = readRegister(CTRL_REG1); // Read contents of register CTRL_REG1
  writeRegister(CTRL_REG1, c & ~(0x01)); // Set SBYB (bit 0) to 0
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Sets the MPL3115A2 to active mode.
// Needs to be in this mode to output data
void MPL3115A2Active()
{
  byte c = readRegister(CTRL_REG1); // Read contents of register CTRL_REG1
  writeRegister(CTRL_REG1, c | 0x01); // Set SBYB (bit 0) to 1
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Read i registers sequentially, starting at address into the dest byte array
void readRegisters(byte address, int i, byte * dest)
{
  i2cSendStart();
  i2cWaitForComplete();

  i2cSendByte((MPL3115A2_ADDRESS<<1)); // write 0xB4
  i2cWaitForComplete();

  i2cSendByte(address);	// write register address
  i2cWaitForComplete();

  i2cSendStart();
  i2cSendByte((MPL3115A2_ADDRESS<<1)|0x01); // write 0xB5
  i2cWaitForComplete();
  for (int j=0; j<i; j++)
  {
    i2cReceiveByte(TRUE);
    i2cWaitForComplete();
    dest[j] = i2cGetReceivedByte(); // Get MSB result
  }
  i2cWaitForComplete();
  i2cSendStop();

  cbi(TWCR, TWEN); // Disable TWI
  sbi(TWCR, TWEN); // Enable TWI
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Read a single byte from address and return it as a byte
byte readRegister(uint8_t address)
{
  byte data;

  i2cSendStart();
  i2cWaitForComplete();

  i2cSendByte((MPL3115A2_ADDRESS<<1)); // Write 0xB4
  i2cWaitForComplete();

  i2cSendByte(address);	// Write register address
  i2cWaitForComplete();

  i2cSendStart();

  i2cSendByte((MPL3115A2_ADDRESS<<1)|0x01); // Write 0xB5
  i2cWaitForComplete();
  i2cReceiveByte(TRUE);
  i2cWaitForComplete();

  data = i2cGetReceivedByte();	// Get MSB result
  i2cWaitForComplete();
  i2cSendStop();

  cbi(TWCR, TWEN);	// Disable TWI
  sbi(TWCR, TWEN);	// Enable TWI

  return data;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Writes a single byte (data) into address
void writeRegister(unsigned char address, unsigned char data)
{
  i2cSendStart();
  i2cWaitForComplete();

  i2cSendByte((MPL3115A2_ADDRESS<<1)); // Write 0xB4
  i2cWaitForComplete();

  i2cSendByte(address);	// Write register address
  i2cWaitForComplete();

  i2cSendByte(data);
  i2cWaitForComplete();

  i2cSendStop();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Poor man's system clock that must be set to the correct time upon start of the program
// A real time clock on board the circuit would be better
void Clock()
{
  //this function prints the current time
  lcd.clear(); // start by clearing the LCD

  // Initialize clock

  // set up some variables for the clock function // Define variables for seconds, minutes, and hours 

  unsigned long seconds = 0;
  unsigned long minutes = 0;
  unsigned long hours   = 0;
  // Define variables to set initial values for seconds, minutes, and hours 
  unsigned long setseconds = 30; // set clock seconds 
  unsigned long setminutes = 52; // set clock minutes
  unsigned long sethours = 10; // set clock hours
  // Define variables to set alarm values for seconds, minutes, and hours 
  unsigned long tseconds = 0; // set clock seconds 
  unsigned long tminutes = 0; // set clock minutes
  unsigned long thours   = 0; // set clock hours

  // Set the time units using the millisecond utility 
  // Remember to add the fractions of a minute (setseconds/60) to the minute tally and 
  // the fractions of an hour (setseconds/3600 + setminutes/60) to the hour tally.

  seconds = setseconds + (millis()/1000); // 1 second = 1000 milliseconds
  minutes = setminutes + ((millis() + setseconds*1000)/60000); // 1 minute = 60,000 milliseconds
  hours   = sethours   + ((millis() + setseconds*1000 + setminutes*60000)/3600000); // 1 hour = 3,600,000 milliseconds

  // print colons separating hours, minutes, and seconds

  lcd.setCursor(2,0);
  lcd.print(":");
  lcd.setCursor(5,0);
  lcd.print(":");

  // Check if the seconds/minutes are greater than 60, // if so, truncate to less than 60; // Check if the hours are greater than 12, if so truncate to // less than 12.

  while(seconds >= 60) {
    seconds = seconds - 60;
  } // end while

  while(minutes >= 60) {
    minutes = minutes - 60;
  } // end while

  while(hours > 12) {
    hours = hours - 12;
  } // end while

  // If seconds less than 10, print a zero in the tens place and seconds thereafter

  if(seconds <= 9) {
    lcd.setCursor(6,0);
    lcd.print("0");
    lcd.setCursor(7,0);
    lcd.print(seconds);
  }

  // Otherwise, if seconds greater than 9 print

  else {
    lcd.setCursor(6,0);
    lcd.print(seconds);
  }

  // If minutes less than 10, print a zero in the tens place and minutes thereafter

  if(minutes <= 9) {
    lcd.setCursor(3,0);
    lcd.print("0");
    lcd.setCursor(4,0);
    lcd.print(minutes);
  }

  // Otherwise, if minutes greater than 9 print

  else {
    lcd.setCursor(3,0);
    lcd.print(minutes);
  }

  // If hours less than 10, print a zero in the tens place and hours thereafter

  if(hours <= 9) {
    lcd.setCursor(0,0);
    lcd.print("0");
    lcd.setCursor(1,0);
    lcd.print(hours);
  }

  // Otherwise, if hours greater than 9 print

  else {
    lcd.setCursor(0,0);
    lcd.print(hours);
  }

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// End of Sketch MPL3115A2
