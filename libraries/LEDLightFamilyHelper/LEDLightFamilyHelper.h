/*
 * LEDLightFamilyHelper.h
 * Header file for the misc functions used by both Parent and Child sketched
 * from the LED Light family project.
 */

#ifndef LEDLightFamilyHelp_h
#define Accelerometer_h
#endif

#include <Arduino.h>
#include <math.h>
#include <i2c.h>

#define DEBUG
#define TX_ON	0
#define TX_OFF	1
#define RX_ON	0
#define RX_OFF	1

//WAKE SLEEP states
#define STATE_WAKE	0
#define STATE_SLEEP	1
#define TX_SLEEP_DELAY_MILLI	10  //when in SLEEP state, only tx every n milliseconds
#define TX_WAKE_DELAY_MILLI		0  //when in WAKE state, only tx every n milliseconds

extern const byte CMD_OFF_MASK;
extern const byte CMD_LPFON_MASK;

extern byte accelData[6];  // x/y/z accel register data store here
extern int accelCount[3];  // Stores the 12-bit signed value
extern const byte AccelScale;
extern const byte AccelDataRate;


// reads the accelerometer and returns an array of 3 floating point readings.
void readRAWAccel(float (&data)[3]);

//takes in and array of 3 raw readings and returns 3 smoothed readings (LP filter/moving average)
void SmoothAccel(float RC, float rawdata[3], float (&smoothdata)[3]);

//takes in 4 byte RGB+cmd buffer and smoothes them out using LP filter/moving average if cmd bit is set
void SmoothRGB(int child_id, float RC, byte (&buf)[7], byte &out_r, byte &out_g, byte &out_b);

//given x, y and sensitivity parameters, returns a saturation and hue.
void CalcSat_n_Hue(int sens, float x, float y, float &sat, float &hue);

//given x and y parameters, returns a brightness
void CalcBrightness(int sens, float x, float y, float &brightness);

//slowly cycles through all the hues (colors)
void cycleHue(float &hue, int speed);

//converts HSV to RGB in readiness to write PWM...
void HSV_to_RGB(float h, float s, float v, byte &r, byte &g, byte &b);

//Gamma corrects an R, G, or B value passed in
byte gamma_correction(byte input);

//turns TX hardware on or off
void setTX(int state);

//transmits an array buffer over TX pin
//sends only infrequently when in SLEEP state
void TXAccel(byte (&buf)[7], int state);

//same as above, but MEGA fast (no delay)
void TXAccelAudioMode(byte (&buf)[7]);


//turns RX hardware on or off
void setRX(int state);

//receives an array of data from RX pin. 
//returns true if data was received, false otherwise (this implies its non-blocking)
boolean RXAccel(byte (&buf)[7]);

//initialise MSGEQ7 chip
void initMSGEQ7(int analog, int strobe, int reset);

//read the EQ
void readEQ(int reset, int strobe, int analog, int (&bands)[7]);

//convert the 7 bands from the EQ into an RGB code. This routine, which runs on the parent, calculates RGBs for perent (bass)
//as well as the two children (mids and treble). These values then get send to the children via RF. 
void EQ_to_RGB(int bands[7], int lower, int upper, \
				byte &bass_r, byte &bass_g, byte &bass_b, \
				byte &mids_r, byte &mids_g, byte &mids_b, \
				byte &treb_r, byte &treb_g, byte &treb_b  );


//takes in 1 analog audio reading values and smoothes them out (LP filter/moving average)
//void SmoothAudio(float RC, int in_audio, int &out_audio);

//write PWN values out for RGB LEDs
void ShowLight(int RedLED, int GreenLED, int BlueLED, byte r, byte g, byte b);

//write quick light flash
void FlashLights(int count, int RedLED, int GreenLED, int BlueLED, byte r, byte g, byte b);

//check for mode button being presses.
boolean ModeButtonPressed(boolean &last, boolean &current,int pin, int btn_delay);

//cyles the mode.
void changeMode(int &mode, int no_modes);

/**
 * processes the command by and sets the mode accordingly
 * @param cmd  command byte
 * @param mode resulting mode.
 */
void ProcessCommand(byte cmd,int &mode);

//process tap event -- 
//changes the mode depending on whether parent or child
void tapHandler(int whoami, int state, int &mode, byte &cmd);

//process P/L event
//changes the mode depending on whether parent or child
void portraitLandscapeHandler(int whoami, int state, int &mode);

//process motion event
//changes the mode depending on whether parent or child
void motionHandler(int whoami, int state, int &mode);

//process wake/sleep event
void wakesleepHandler(int &state);

//turn off the device by writing high to the pololu power switch
//void TurnOFF();

