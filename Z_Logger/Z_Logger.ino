/*
--------------------------------------------------------------------------------
Z_Logger

Goal for this version:
Configure and execute single impedance measurement command, report the result,
and repeat, at a sample rate of 20 times/second. (Future TODO: 200Hz)

Current functionalty:
1.) Initialize the AD5933.
2.) Calculate a gain factor, using a calibration resistance.
3.) Take an impedance measurement and report through the serial monitor.
4.) Repeat #3
5.) Optional verbose output for debugging.

*/

//--- Hard-coded inputs for the sketch:

#define TWI_FREQ 400000L // Setting TWI/I2C Frequency to 400MHz to speed up.

#define VERBOSE 0 //Toggles verbose debugging output via the serial monitor.
                  //1 = On, 0 = Off.

#define cycles_base 511      //First term to set a number of cycles to ignore
                             //to dissipate transients before a measurement is
                             //taken. The max value for this is 511.

#define cycles_multiplier 1  //Set a multiple for the cycles_base which
                             //is used to calculate the desired number
                             //of settling cycles. Values can be 1, 2, or 4.
                             

#define start_frequency 50000 //Set the start frequency, the only one of
                              //interest here(50 kHz).

#define cal_resistance 1960.6 //Set a calibration resistance for the gain
                            //factor. This will have to be measured before any
                            //other measurements are performed.
                           
#define cal_samples 10 //Set a number of measurements to take of the calibration
                       //resistance. These are used to get an average gain
                       //factor.
                           
//---

double gain_factor = 0;

#include "AD5933.h" //Library for AD5933 functions (must be installed)
#include <Wire.h> //Library for I2C communications

void setup()
{
  Serial.begin(9600); //Initialize serial communication for debugging.
  #if VERBOSE
  Serial.println("Program start!");
  delay(100);
  #endif
  
  TWBR=1;
  Wire.begin();
  //TWBR=1;
  delay(1000);
  
  AD5933.setExtClock(false);
  //[A.X] Send a reset command to the AD5933.
  if(AD5933.resetAD5933() == true)
  {
    #if VERBOSE
    Serial.println("Reset command sent.");
    delay(100);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.println("Error sending reset command!");
    delay(1000);
    #endif
  }
  //End [A.X]
  
  //--- A. Initialization and Calibration Meassurement ---
  
  //[A.0] Set the clock for internal/external frequency
  //Set the operational clock to internal
  //AD5933.setExtClock(0);
  
  //[A.1] Set the measurement frequency
  if (AD5933.setStartFreq(start_frequency) == true)
  {
    #if VERBOSE
    Serial.print("Start frequency set to: ");
    Serial.print(start_frequency);
    Serial.println (" Hz.");
    delay(100);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error setting start frequency!");
    delay(1000);
    #endif
  }
  //End [A.1]


  //[A.2] Set a number of settling time cycles
  if (AD5933.setSettlingCycles(cycles_base, cycles_multiplier) == true)
  {
    #if VERBOSE
    Serial.print("Settling cycles set to: ");
    Serial.print(cycles_base*cycles_multiplier);
    Serial.println(" cycles.");
    delay(100);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error setting settling cycles!");
    delay(1000);
    #endif
  }
  //End [A.2]
  
  double temp = AD5933.getTemperature();
  #if VERBOSE
  Serial.print("Temperature is ");
  Serial.print(temp);
  Serial.println(" degree celcius.");
  #endif
  
  //[A.3] Calculate the gain factor (needs cal resistance, # of measurements)
  //Note: The gain factor finding function returns the INVERSE of the factor
  //as defined on the datasheet!
  gain_factor = AD5933.getGainFactor(cal_resistance, cal_samples, false);
  if (gain_factor != -1)
  {
    #if VERBOSE
    Serial.print("Gain factor (");
    Serial.print(cal_samples);
    Serial.print(" samples) is: ");
    Serial.println(gain_factor);
    delay(100);
    #endif
  }
  else
  {
    #if VERBOSE
    Serial.print("Error calculating gain factor!");
    delay(1000);
    #endif
  } 
  //End [A.3]

}

void loop()
{
  unsigned long timerMicro1 = micros();
  //--- B. Repeated single measurement ---
  //Gain factor calibration already sets the frequency, so just send 
  //repeat single magnitude capture command.
 
  #if VERBOSE
  double temp = AD5933.getTemperature();
  Serial.print("Temperature is ");
  Serial.print(temp);
  Serial.println(" degree celcius.");
  #else
  AD5933.tempUpdate();
  #endif
  
  //[B.1] Issue a "repeat frequency" command.
  #if VERBOSE
  if (AD5933.setCtrMode(REPEAT_FREQ) == true)
  {
    Serial.println("Repeat_Frequency command sent.");
    delay(100);
  }
  else
  {
    Serial.println("Error sending Repeat_Frequency command!");
    delay(1000);
  }
  #else
  AD5933.setCtrMode(REPEAT_FREQ);
  #endif
  //End [B.1]
   

  //delay(100);
  #if VERBOSE
  //[B.2.1] Capture the magnitude from real & imaginary registers.
  double Z_magnitude = AD5933.getMagOnce();
  Serial.print("Magnitude found to be: ");
  Serial.println(Z_magnitude);
  delay(100);
  //End [B.2.1]
  
  //[B.2.2] Calculate the impedance using the magnitude and gain factor.
  double Z_value = gain_factor/Z_magnitude;
  Serial.print("Impedance found to be: ");
  Serial.print(Z_value);
  Serial.println(" Ohms.");
  delay(100);
  //End [B.2.2]
  
  #else
  double Z_value = gain_factor/AD5933.getMagOnce();
  
  
  #endif
  
    
  //Serial.print("Time: ");
  Serial.print(timerMicro1 / 1000000.0);  
    
  //[B.2.3] Output the impedance value (serial, array, etc.)
  
  //Serial.print(" Impedance = ");
  Serial.print("\t");
  Serial.print(Z_value);
  //Serial.println(" Ohms.");
  Serial.println();
  
  //unsigned long timerMicro2 = micros();
  //delayMicroseconds(16666 - (timerMicro2-timerMicro1));
  
  //End [B.3]
  
  // --- End B ---

  #if VERBOSE
  Serial.println("End Loop!");
  //delay(1000);
  #endif
}

