
/*
 * Sketch for Water Quality KIT  Project: Metti in Circolo
   Hardware: Wemos D1 mini, ADS1115 (ADC 16 bit), pH probe, Conductivity probe, Temperature probe, 
             charge/discharge board
 *                                     
 *   Compiler IDE: LOLIN(WEMOS) D1 R2 & mini
 *   Cable: micro USB
 *   
 *   Before uploading set the Probe numbers and the LCD address
 *   
 *   Paolo Bonelli 01/11/2020
 *   Paolo Bonelli 16/11/2020
 *   Paolo Bonelli 26/11/2020
 *   Paolo Bonelli 30/12/2020
 *   Paolo Bonelli 11/01/2021
 *   Paolo Bonelli 01/05/2021
 *   Paolo Bonelli 08/05/2021
 *   
 *   ///////////////////////////////////////////////////////////////////////////
 *   Novità sulla libreria Adafruit ADS1115
 *   https://learn.adafruit.com/adafruit-4-channel-adc-breakouts/arduino-code

Construction and Initialization:

Adafruit_ADS1115();  
Construct an instance of an ADS1115 

begin();
Initialize the ADC for operation using the default address and I2C bus (0x48).
begin(0x49);
Initialize the ADC for operation using specified address of 0x49.
La scheda ADS1115 consente l’inserimento di 4 di queste sul bus I2C con indirizzi diversi. 
L’indirizzo di default è 0x48. Gli altri indirizzi si ottengono con dei ponticelli sulle schede.
 
///////////////////////////////////////////////////////////////////////////
 */

//*******************************************************************************
//  Put here the Probe numbers
byte pHprobe = 6;
byte ECprobe = 6;
//*******************************************************************************
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
//Adafruit_ADS1115 ads_0(0x48);  //  ADDR pin is connected with GND for the address 0x48
Adafruit_ADS1115 ads1115_0;  

//  Analog Input of ADS1115
#define OUT        0     //  pH sensor  A0  analog ADS1115
#define PA         1     //  EC sensor  A1  analog ADS1115
#define pinBAT     2     //  Battery voltage  A2  analog ADS1115
#define pinCharge  3     //  Charging voltage A3  analog ADS1115

//  Digital pin of wemos
//  WARNING  D3 and D4 must be HIGH at boot, don't use them
#define ContPin1    D6       //  Control1 pin for EC probe power (P1)
#define ContPin2    D7       //  Control2 pin for EC probe power (P2)
#define tempDatPin  D5       //  One Wire DAT pin for Temperature Probe

// Display
// some I2C display address: 0x27, 0x3F, 0x22
#include <LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x27,16,2);  //   (da #1 a #5)
LiquidCrystal_I2C lcd(0x3F,16,2);  //   (verde #6)

//  Water temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS tempDatPin
#define TEMPERATURE_PRECISION 12
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature Temp(&oneWire);
int numberOfDevices; // Number of temperature devices found
// We'll use this variable to store a found device address
DeviceAddress tempDeviceAddress; 

// define global variables
float CS =        0.;  // Conductivity at 20°  uS (microSiemens)
float CSspec =    0.;  // Specific Conductivity at 20°  uS/cm
float Temp0 =     0.;  // Water temperature in degree centigrade (°C)
float PH =        0.;  // Water pH
float pHvolt =    0.;   // voltage from pH probe (mV)
float vBat =      0.;  // Battery Voltage (V)
float vCharge =   0.;  // Charging Voltage (V)
float Vconv =     0.125;  // Conversion factor for ADS1115 analog input:  output number * Vconv = Voltage (mV)
byte  chargeFlag= 0;
uint16_t nAverage = 5;   // number of measure cycles for the average
uint16_t nmeno =    2;   // number of the first cycles to be excluded from the average
//////////////////////////////////////////////////////////////
void setup() {

// init Serial Monitor
  Serial.begin(57600);
  delay(1000);
  Serial.println(" WemosLCD_KIT_Temp ");
  
//  init display LCD
  lcd.init();      
  delay(1000);
  Serial.println("LCD OK");

// Print a message to the LCD.
  lcd.backlight();
  lcd.clear();
  lcd.print("Wemake WaterKIT");
  lcd.setCursor(0, 1);
  lcd.print("pH & EC sensors");
  delay(3000);

// init Temp sensor
  Temp.begin();
  // Grab a count of devices on the wire
  numberOfDevices = Temp.getDeviceCount();
  // locate devices on the bus
    Serial.println("DS18B20");
    Serial.print("Locating devices..."); 
    Serial.print("Found ");
    Serial.print(numberOfDevices, DEC);
    Serial.println(" devices.");
    lcd.clear();
    if(numberOfDevices == 1){
        lcd.print("T sensor OK");
    }else{
        lcd.print("No T sensor");
    }
    // Loop through each device, print out address
     for(int i=0; i < numberOfDevices; i++){
        if(Temp.getAddress(tempDeviceAddress, i)){
         Serial.print("Found device ");
         Serial.print(i, DEC);
         Serial.print(" with address: ");
         printAddress(tempDeviceAddress);
         Serial.println();
         Serial.print("Setting resolution to ");
         Serial.println(TEMPERATURE_PRECISION, DEC);
    // set the resolution to TEMPERATURE_PRECISION bit (
    // Each Dallas/Maxim device is capable of several different resolutions)
     Temp.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
     Serial.print("Resolution actually set to: ");
     Serial.print(Temp.getResolution(tempDeviceAddress), DEC); 
     Serial.println();
    }else{
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
   }
  
// Initialize ADS1115 gain and conversion factors
  ads1115_0.begin(0x48);   //  default address  you can omit it, but if you use more than one boards you must 
  delay(1000);
 /* The ADS115 board is in Single ended configuration the resolution is 16 bit with sign,
  * The input voltage range must not exceed the limit of the table below after
  * you have set the gain
  *                                                                 ADC 16 bit (mV/digit)
  * ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain  +/- 6.144V          0.1875mV (default)
    ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V           0.125mV
    ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V           0.0625mV
    ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V           0.03125mV
    ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V           0.015625mV
    ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V           0.0078125mV
*/
  ads1115_0.setGain(GAIN_ONE);     // 1x gain     +/- 4.096V  1 digit =  0.125mV

//  init pins
  pinMode(ContPin1,OUTPUT);
  pinMode(ContPin2,OUTPUT);
  
// Read pH to check if the sensor work
  ReadPH();
  lcd.setCursor(0,1);
  if(pHvolt < 1000 ){
     lcd.print("No pH sensor");
  }else{
     lcd.print("pH OK #");lcd.print(pHprobe);
  }
  delay(2000);
  
// Read EC to check if the sensor work or it is inside water
  lcd.clear();
  lcd.print("Wait for EC");
  lcd.setCursor(0,1);
  ReadConduct(1,0);
  if(CS <= 0 || CS >= 99999.0){
     lcd.print("No EC");
  }else{
     lcd.print("EC OK #");lcd.print(ECprobe);
  }
  delay(2000);
}
//-------------------------------------------------------------------------
void loop() {
  
// check if Battery is charging
    lcd.clear();
    ReadCharge();
    ReadBattery();
    if(vCharge < 3.5){
      lcd.print("No charging");      
      chargeFlag = 0;
      lcd.setCursor(0,1);
      lcd.print("vBat ");lcd.print(vBat);
      if(vBat < 3.0){
         delay(2000);
         lcd.clear();
         lcd.print("Battery too low");
         lcd.setCursor(0,1);
         lcd.print("I stop execution");
         while(1){
             delay(100);  // stop the program
         }
      }
    }else{
      lcd.print("Charging:");
      chargeFlag = 1;
      lcd.setCursor(0,1);
      lcd.print("vBat ");lcd.print(vBat);
    } 
    delay(2000);
   
  //  Check Battery
    lcd.clear();
    if(!chargeFlag){
      lcd.print("Wait for measure");   
      Serial.println(" Compute pH");
      ReadPH();
      Serial.print(" pH ");Serial.println(PH);
//
      Serial.println(" Compute Conductivity ");
      ReadTemp();
      ReadConduct(nAverage,nmeno);
      Serial.print(" Conductivity at 20°C          ");Serial.print(CS,0);Serial.println(" uS");
      Serial.print(" Specific Conductivity at 20°C ");Serial.print(CSspec,0);Serial.println(" uS/cm");
      Serial.print(" Water temperature ");Serial.print(Temp0,1);Serial.println(" °C");
      
// Display results
      lcd.clear();
      lcd.setCursor(0,0);
      if(PH < 14){
        lcd.print("pH ");lcd.print(PH);
      }else{
        lcd.print("no pH   ");
      }
      if(Temp0 < 900){
        lcd.print(" T ");lcd.print(Temp0,1);
      }else{
        lcd.print(" no T   ");        
      }
      
      lcd.setCursor(0,1);
      if(CSspec < 90000){
        lcd.print("SpEC ");lcd.print(CSspec,0);lcd.print(" uS/cm");
      }else{
        lcd.print("no SpEC         ");
      }
      delay(5000);
      lcd.clear();
      lcd.print("EC20C ");lcd.print(CS,0);lcd.print(" uS");   
      lcd.setCursor(0,1); 
      lcd.print("pHvolt ");lcd.print(pHvolt,0);lcd.print(" mV");    
    }
    delay(5000);
}
////////////////////// function to print a device address//////////////////////////////////
void printAddress(DeviceAddress deviceAddress){
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
