/*
	LEDLightFamily -- Parent Module
	Created 2013-03-19 by Bradley Wood
	$Id:: Parent.ino 2130 2013-10-23 16:02:51Z                              $: 
	$Date:: 2013-10-23 17:02:51 +0100 (Wed, 23 Oct 2013)                    $: 
	$URL:: https://brad@svn.greyspark.com/svn/users/brad/tags/LEDLightFamil#$:

Copyright (c) 2013 Bradley Wood
Released as Open Source under GNU GPL v3, see <http://www.gnu.org/licenses/>

This project is to design an LED-driven light cube family -- I am planning one
parent and two kids, although, you could add more children.

The parent has the following:
* Arduino Pro Mini 3V3 (8MHz)
* MMA 8452 3-axix accelerometer breakout board 
* (a gyro would be nice too)
* An RF transmitter
* 3 super bright LEDs, one red, one green, one blue
* a Pololu low voltage power switch 
* (a wake-on-shake breakout would also be cool)
* Portable power pack (9V battery, for now)

The kid has exactly the same, but an RF receiver, rather than a transmitter.

All gear available from proto-pic.co.uk

Function: the parent light (mum) can have its colour/brightness changed by
tilting, moving, shaking or twisting or "dropping". It transmits its RGB value
over RF. The kids pick it up and fade their own lights into sync with mum.
Kids can also operate as independent lights, similar to their mum.

*/

//DEBUG code -- remove this compiler directive to turn off Serial debug msgs
#define DEBUG

#define DEBUGPRINTING             \
	Serial.print("S=");           \
	Serial.print(sat,4);          \
	Serial.print("\t");           \
	Serial.print("H=");           \
	Serial.print(hue,4);          \
	Serial.print("\t");           \
	Serial.print("V=");           \
	Serial.println(brightness,4); \
	Serial.print("R=");           \
	Serial.print(red);            \
	Serial.print("\t");           \
	Serial.print("G=");           \
	Serial.print(green);          \
	Serial.print("\t");           \
	Serial.print("B=");           \
	Serial.println(blue);         \
	Serial.print("R'=");          \
	Serial.print(g_red);          \
	Serial.print("\t");           \
	Serial.print("G'=");          \
	Serial.print(g_green);        \
	Serial.print("\t");           \
	Serial.print("B'=");          \
	Serial.println(g_blue);


//so that each device knows what it is.
#define PARENT 1
#define CHILD 2

const int whoami = PARENT;

//All common functions used by either Parent or Child (as well as some specific
//to either the Child or the Parent)
#include <LEDLightFamilyHelper.h>

//OPERATING MODES

//these are cycled through via trapping a tap in any dimention using the
//tap handler capability of the accelerometer

#define NUM_MODES			4 // 3 modes for now

#define MODE_FREE_COLOR		0 // free hue & sat change by tilt
#define MODE_FULL_LOCK		1 // hue, sat and value (intensity) locked
#define MODE_COLORCYCLE		2 // gently cycle through the colour wheel (mood-light mode)
#define MODE_AUDIO			3 // audio response mode

#define MODE_COLORCYCLE_SPEED	20 // no of milliseconds delay before moving to next degree of hue.

int mode = MODE_AUDIO; //start out in free colour mode

#define MODE_SWITCH_PIN		16 //Pin A2

//=== Debounce setup =====
boolean LastButtonState = HIGH; //the state of the button last time we checked
boolean CurrentButtonState = HIGH; //the current reading from the button pin
#define Debounce_Delay	5 //milliseconds

//WAKE SLEEP states
#define STATE_WAKE	0
#define STATE_SLEEP	1

int state = STATE_WAKE; //start out awake

// PROGRAMMATIC TURN-OFF
// if R, G & B go to zero and sleep has kicked in fo a specified 
//elapsed time, then turn off the device completely be send a signal to the 
// pololu switch.

#define OFFPin		4  //set high to turn the whole device off

//LEDs
#define RedLED		5
#define GreenLED	6
#define BlueLED		9

//AUDIO
//===MSGEQ7 configs===
//Set up variables for MSGEQ7 input
const int EQAnalogPin = 15; // read from multiplexer using A1
const int EQStrobePin = 10; // strobe is attached to 10
const int EQResetPin = 14; // reset is attached to digital pin A0
int EQSpectrumValue[7]; // to hold a2d values from MSGEQ7
const int EQLowerMapBound = 600; // Set the lower threshold for map() to reduce noise when "silent" 
const int EQUpperMapBound = 1023; // Set the upper threshold for map() to reduce noise when "silent" 


// RF CONFIG

 /* Pins:
 * Hardware SPI:
 * MISO -> 12
 * MOSI -> 11
 * SCK -> 13
 *
 * Configurable:
 * CE -> 8
 * CSN -> 7
 */

#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

#define TX_ON	0
#define TX_OFF	1
#define TXADDR "CHIL1" //the address we are sending to. 
//#define RXADDR "CHIL1"

int TX_state = TX_ON;
byte buf[7]; //3 bytes for mids RGB triplet, 3 bytes for treble RBG triplet, and 1 command byte.
/*
Command byte format

MSB                           LSB
  7   6   5   4   3   2   1   0
--------------------------------
| x | x | x | x | x | x | x | x |
--------------------------------

Bit 7: 1 = turn off child, 0 = don't turn off
Bit 6: 1 = child uses LPF to fall into line with parent, 0 = immediate color alignment
Bit 5: 1 = change mode, 0 = do not change mode
Bit 4,3: Mode to change to if bit 5 high.
	00 = free color
	01 = color lock
	10 = color cycle
	11 = audio mode

Bit 2-0: Reserved for later use.

									 76543210  */
const byte CMD_OFF_MASK = 			B10000000;
const byte CMD_LPFON_MASK = 		B01000000;
const byte CMD_MODE_CHANGE_MASK = 	B00100000;
const byte CMD_MODE_FREECOLOR = 	B00000000;
const byte CMD_MODE_COLORLOCK = 	B00001000;
const byte CMD_MODE_COLORCYCLE = 	B00010000;
const byte CMD_MODE_AUDIO = 		B00011000;

//ACCELEROMETER 
/*
Note: The MMA8452 is an I2C sensor, however this code does not make use of
the Arduino Wire library. Because the sensor is not 5V tolerant, we can't use
the internal pull-ups used by the Wire library. Instead use the included i2c.h
file.
*/

#include <Accelerometer.h>
#include <math.h>
#include <i2c.h>

#define SA0 1  // Breakout board defaults to 1, set to 0 if SA0 jumper is set
#if SA0
	#define MMA8452_ADDRESS 0x1D  // SA0 is high, 0x1C if low
#else
	#define MMA8452_ADDRESS 0x1C
#endif
/* Set the scale below either 2, 4 or 8*/
const byte AccelScale = 2;  // Sets full-scale range to +/-2, 4, or 8g. Used to calc real g values.
/* Set the output data rate below. Value should be between 0 and 7*/
const byte AccelDataRate = 6;  // 0=800Hz, 1=400, 2=200, 3=100, 4=50, 5=12.5, 6=6.25, 7=1.56
// low sample rate to conserve battery. 
/* Pin definitions */
const int AccelInt1Pin = 2;  // used for x, y, z accel data
const int AccelInt2Pin = 3;  // used for all other interrupts (tap, wake/sleep, portrait/landscape, etc)

byte accelData[6];  // x/y/z accel register data store here
int accelCount[3];  // Stores the 12-bit signed value
float accelValue[3];  // Stores the real accel value in g's

//stores the real accel values in g's, but smoothed out using a LP
//filter/moving average.
float smoothedValue[3]; 
const float LPF_RC = 0.6; // value between 0 and 1 used for LP filter. 0 = no filtering. 1 equals = no reading.
const int TiltSensitivity = 300; // The higher the number, the less tilt required.

/* 
Color changes will work by tracking the object's "tilt" in 3D-space using
the accelerometers x, y vectors. The tilt will be mapped to a 2D vector
which will determine the hue and saturation. 
in HSB cyclindrical space with the vector's  x- and y-components determining the
hue and the degree of tilt from the horizontal determining the saturation. 

*/
byte red, g_red, green, g_green, blue, g_blue; // number to be sent to each LED to drive PWM setting. 0-1023
byte mids_r, mids_g, mids_b, treb_r, treb_g, treb_b; // variables for tx-ing mids and treble
byte g_mids_r, g_mids_g, g_mids_b, g_treb_r, g_treb_g, g_treb_b; // gamma-corrected versions of the above
float hue = 0.0; // will hold an angle -- 
float sat, brightness = 0.0; //will hold saturation (0-254) and brightness (height) 

unsigned int hi, si, bi; //integer versions of hue, saturation and brightness.

void setup ()
{	
	
	buf[6] = 0; //initiate command byte

	#if defined DEBUG
		Serial.begin(115200);
		Serial.println("PARENT");
	#endif

	//set up mode Pin using internal pull
	pinMode(MODE_SWITCH_PIN,INPUT);
	digitalWrite(MODE_SWITCH_PIN,HIGH); //turn on pullup
	
	// make sure the off pin is low.
	pinMode(OFFPin,OUTPUT);
	digitalWrite(OFFPin,LOW);
	
	//initialise RF infrastucture
	Mirf.spi = &MirfHardwareSpi;
	Mirf.init();
	Mirf.setTADDR((byte *)TXADDR);
  
	//set up LED pins
	pinMode(RedLED, OUTPUT);
	pinMode(GreenLED, OUTPUT);
	pinMode(BlueLED, OUTPUT);

	/* Set up the interrupt pins, they're set as active high, push-pull */
	pinMode(AccelInt1Pin, INPUT);
	digitalWrite(AccelInt1Pin, LOW);
	pinMode(AccelInt2Pin, INPUT);
	digitalWrite(AccelInt2Pin, LOW);

	//initialise EQ chip
	initMSGEQ7(EQAnalogPin, EQStrobePin, EQResetPin);
	
	//initialise and calibrate the accelerometer
	initMMA8452(AccelScale, AccelDataRate);
	CalibrateAccel();
}

void loop()
{
	if(ModeButtonPressed(LastButtonState,CurrentButtonState,MODE_SWITCH_PIN,Debounce_Delay))
	{
		#if defined DEBUG
			Serial.println("Mode changed (button press).");
		#endif
		//cycle the Mode variable to the next mode.
		changeMode(mode, NUM_MODES);
		//flash a number of times to ack the mode change... 
		FlashLights(1,RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);		
	}
	
	//read Int2 on the accelerometer and then set mode and wake/sleep accordingly
	static byte accel_event_int_register;
	/* If int2 goes high, a trappable event has occured */
	//TODO: Sort out PortraitLandscape wake/sleep trigger
	if (digitalRead(AccelInt2Pin))
	{
		accel_event_int_register = readRegister(INT_SOURCE_REG);  // Read the interrupt source reg.
		
		if ((accel_event_int_register & SRC_PULSE_MASK)==SRC_PULSE_MASK)  // // tap event triggered
		{
			#if defined DEBUG
				Serial.println("--------------TAP TAP TAP TAP TAP --------------");
			#endif
			tapHandler(whoami, state, mode, buf[6]);
		}
		
		if((accel_event_int_register & SRC_FF_MT_1_MASK)==SRC_FF_MT_1_MASK) //motion detected
		{
			#if defined DEBUG
				Serial.println("-------------MOTION MOTION MOTION----------------");
			#endif
			
			motionHandler(whoami, state, mode); //read the register to clear the latch, this should force WAKE mode.
		}
		
		if((accel_event_int_register & SRC_LNDPRT_MASK)==SRC_LNDPRT_MASK) //portrait/landscape detected
		{
			#if defined DEBUG
				Serial.println("-------------PORTRAIT LANDSCAPE----------------");
			#endif
			portraitLandscapeHandler(whoami, state, mode);
		}
	
		if ((accel_event_int_register & SRC_ASLP_MASK)==SRC_ASLP_MASK) // sleep/wake event triggered
		{
			#if defined DEBUG
				Serial.println("-------------WAKE SLEEP WAKE SLEEP----------------");
			#endif
			
			wakesleepHandler(state); //set state accordingly -awake or asleep. 
		}

	}	
	// now handle the wake and sleep modes separately
	switch (state) 
	{
		case STATE_WAKE:
			
			//turn on RF TX infrastruture.
			setTX(TX_ON);
			#if defined DEBUG
				Serial.println("WAKE");
			#endif

			switch (mode) 
			{
				case MODE_FREE_COLOR:
					#if defined DEBUG
						Serial.println("MODE_FREE_COLOR");
					#endif
					/* If int1 goes high, all data registers have new data */
					if (digitalRead(AccelInt1Pin))  // Interrupt pin, should probably attach to interrupt function
					{
						readRAWAccel(accelValue);

						SmoothAccel(LPF_RC, accelValue, smoothedValue);

						CalcSat_n_Hue(TiltSensitivity, smoothedValue[0], smoothedValue[1],  sat, hue);

						//force full brightness in the this mode.
						brightness = 100.0;
						
						HSV_to_RGB(hue,sat,brightness,red,green,blue);

						g_red = gamma_correction(red);
						g_green = gamma_correction(green);
						g_blue = gamma_correction(blue);

						DEBUGPRINTING;

						//turn on the LEDs
						ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);

						//transmit RGB codes -- 3 bytes for R,G,B
						buf[0] = g_red; buf[1] = g_green; buf[2] = g_blue;  //child 1 takes mids
						buf[3] = g_red; buf[4] = g_green; buf[5] = g_blue;  //child 2 takes treble
						// in this case, mids and treble are both the same as we are not in audio mode.
						buf[6] = (buf[6] | CMD_LPFON_MASK | CMD_MODE_CHANGE_MASK | CMD_MODE_FREECOLOR); //tell child to fade gently into line
						
						#if defined DEBUG
							Serial.print("Cmd byte sent: ");
							Serial.println(buf[6], BIN);
						#endif

						TXAccel(buf,state);
					}
				break;

				case MODE_FULL_LOCK:
					#if defined DEBUG
						Serial.println("MODE_FULL_LOCK");
					#endif
					
					DEBUGPRINTING;
					
					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);

					//transmit RGB codes -- 3 bytes for R,G,B
					buf[0] = g_red; buf[1] = g_green; buf[2] = g_blue;
					buf[3] = g_red; buf[4] = g_green; buf[5] = g_blue;
					buf[6] = (buf[6] | CMD_LPFON_MASK | CMD_MODE_CHANGE_MASK | CMD_MODE_FREECOLOR); //tell child to fade gently into line
						
					#if defined DEBUG
						Serial.print("Cmd byte sent: ");
						Serial.println(buf[6], BIN);
					#endif

					TXAccel(buf,state);
					
				break;
								
				case MODE_COLORCYCLE:
					//gently cycle through colour where -- no accel in this mode.
					
					#if defined DEBUG
						Serial.println("MODE_COLORCYCLE");
					#endif

					//force full brightness in the this mode.
					brightness = 100.0;
					sat = 100.0;
					
					cycleHue(hue,MODE_COLORCYCLE_SPEED);
					HSV_to_RGB(hue,sat,brightness,red,green,blue);

					g_red = gamma_correction(red);
					g_green = gamma_correction(green);
					g_blue = gamma_correction(blue);

					DEBUGPRINTING;

					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);

					//transmit RGB codes -- 3 bytes for R,G,B
					buf[0] = g_red; buf[1] = g_green; buf[2] = g_blue;
					buf[3] = g_red; buf[4] = g_green; buf[5] = g_blue;					
					buf[6] = (buf[6] | CMD_LPFON_MASK | CMD_MODE_CHANGE_MASK | CMD_MODE_FREECOLOR); //tell child to fade gently into line
						
					#if defined DEBUG
						Serial.print("Cmd byte sent: ");
						Serial.println(buf[6], BIN);
					#endif

					TXAccel(buf,state);
				break;

				case MODE_AUDIO:
					//read the mic and display a color accordingly..
					
					#if defined DEBUG
						Serial.println("MODE_AUDIO");
					#endif
					
					readEQ(EQResetPin, EQStrobePin, EQAnalogPin, EQSpectrumValue);
										
					EQ_to_RGB(EQSpectrumValue,EQLowerMapBound,EQUpperMapBound,\
							red,green,blue,\
							mids_r,mids_g,mids_b,\
							treb_r,treb_g,treb_b);
					
					g_red = gamma_correction(red);
					g_green = gamma_correction(green);
					g_blue = gamma_correction(blue);
					
					g_mids_r = gamma_correction(mids_r);
					g_mids_g = gamma_correction(mids_g);
					g_mids_b = gamma_correction(mids_b);
					

					g_treb_r = gamma_correction(treb_r);
					g_treb_g = gamma_correction(treb_g);
					g_treb_b = gamma_correction(treb_b);

					DEBUGPRINTING;


					//transmit RGB codes -- 3 bytes for R,G,B -- one set for mids, one for treble
					buf[0] = g_mids_r; buf[1] = g_mids_g; buf[2] = g_mids_b;
					buf[3] = g_treb_r; buf[4] = g_treb_g; buf[5] = g_treb_b;
					//turn off the LPF bit -- immediage colour change, change mode on child to colourlock
					buf[6] = (buf[6] | CMD_MODE_CHANGE_MASK | CMD_MODE_COLORLOCK); 
						
					#if defined DEBUG
						Serial.print("Cmd byte sent: ");
						Serial.println(buf[6], BIN);
					#endif

					TXAccelAudioMode(buf);

					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);

				break;
			}

		break;
		
		case STATE_SLEEP:
			#if defined DEBUG
				Serial.println("SLEEP");
			#endif
			
			//turn on RX infrastructure.
			setTX(TX_ON);
			
			switch (mode) 
			{
				case MODE_FREE_COLOR:
					#if defined DEBUG
						Serial.println("MODE_FREE_COLOR");
					#endif
					/* If int1 goes high, all data registers have new data */
					if (digitalRead(AccelInt1Pin))  // Interrupt pin, should probably attach to interrupt function
					{
						readRAWAccel(accelValue);

						SmoothAccel(LPF_RC, accelValue, smoothedValue);

						CalcSat_n_Hue(TiltSensitivity, smoothedValue[0], smoothedValue[1],  sat, hue);

						//force full brightness in the this mode.
						brightness = 100.0;
						
						HSV_to_RGB(hue,sat,brightness,red,green,blue);

						g_red = gamma_correction(red);
						g_green = gamma_correction(green);
						g_blue = gamma_correction(blue);

						DEBUGPRINTING;

						//turn on the LEDs
						ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);

						//transmit RGB codes -- 3 bytes for R,G,B
						buf[0] = g_red; buf[1] = g_green; buf[2] = g_blue;  //child 1 takes mids
						buf[3] = g_red; buf[4] = g_green; buf[5] = g_blue;  //child 2 takes treble
						// in this case, mids and treble are both the same as we are not in audio mode.
						buf[6] = (buf[6] | CMD_LPFON_MASK | CMD_MODE_CHANGE_MASK | CMD_MODE_FREECOLOR); //tell child to fade gently into line
							
						#if defined DEBUG
							Serial.print("Cmd byte sent: ");
							Serial.println(buf[6], BIN);
						#endif

						TXAccel(buf,state);
					}
				break;

				case MODE_FULL_LOCK:
					#if defined DEBUG
						Serial.println("MODE_FULL_LOCK");
					#endif
					DEBUGPRINTING;

					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);

					//transmit RGB codes -- 3 bytes for R,G,B
					buf[0] = g_red; buf[1] = g_green; buf[2] = g_blue;
					buf[3] = g_red; buf[4] = g_green; buf[5] = g_blue;
					buf[6] = (buf[6] | CMD_LPFON_MASK | CMD_MODE_CHANGE_MASK | CMD_MODE_FREECOLOR); //tell child to fade gently into line
						
					#if defined DEBUG
						Serial.print("Cmd byte sent: ");
						Serial.println(buf[6], BIN);
					#endif

					TXAccel(buf,state);
				
				break;
							
				case MODE_COLORCYCLE:
					//gently cycle through colour where -- no accel in this mode.

					#if defined DEBUG
						Serial.println("MODE_COLORCYCLE");
					#endif

					//force full brightness in the this mode.
					brightness = 100.0;
					sat = 100.0;
				
					cycleHue(hue,MODE_COLORCYCLE_SPEED);
					HSV_to_RGB(hue,sat,brightness,red,green,blue);

					g_red = gamma_correction(red);
					g_green = gamma_correction(green);
					g_blue = gamma_correction(blue);

					DEBUGPRINTING;

					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);

					//transmit RGB codes -- 3 bytes for R,G,B
					buf[0] = g_red; buf[1] = g_green; buf[2] = g_blue;
					buf[3] = g_red; buf[4] = g_green; buf[5] = g_blue;					
					buf[6] = (buf[6] | CMD_LPFON_MASK | CMD_MODE_CHANGE_MASK | CMD_MODE_FREECOLOR); //tell child to fade gently into line
						
					#if defined DEBUG
						Serial.print("Cmd byte sent: ");
						Serial.println(buf[6], BIN);
					#endif

					TXAccel(buf,state);
				
				break;

				case MODE_AUDIO:
					//read the mic and display a color accordingly..
					
					#if defined DEBUG
						Serial.println("MODE_AUDIO");
					#endif
					
					readEQ(EQResetPin, EQStrobePin, EQAnalogPin, EQSpectrumValue);
										
					EQ_to_RGB(EQSpectrumValue,EQLowerMapBound,EQUpperMapBound,\
							red,green,blue,\
							mids_r,mids_g,mids_b,\
							treb_r,treb_g,treb_b);
					
					g_red = gamma_correction(red);
					g_green = gamma_correction(green);
					g_blue = gamma_correction(blue);
					
					g_mids_r = gamma_correction(mids_r);
					g_mids_g = gamma_correction(mids_g);
					g_mids_b = gamma_correction(mids_b);
					

					g_treb_r = gamma_correction(treb_r);
					g_treb_g = gamma_correction(treb_g);
					g_treb_b = gamma_correction(treb_b);

					DEBUGPRINTING;

					//transmit RGB codes -- 3 bytes for R,G,B -- one set for mids, one for treble
					buf[0] = g_mids_r; buf[1] = g_mids_g; buf[2] = g_mids_b;
					buf[3] = g_treb_r; buf[4] = g_treb_g; buf[5] = g_treb_b;
					//turn off the LPF bit -- immediage colour change, change mode on child to colourlock
					buf[6] = (buf[6] | CMD_MODE_CHANGE_MASK | CMD_MODE_COLORLOCK); 
						
					#if defined DEBUG
						Serial.print("Cmd byte sent: ");
						Serial.println(buf[6], BIN);
					#endif

					//super fast tx - prevents lag.
					TXAccelAudioMode(buf);

					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);
				break;
			}
		break;
	}
	
}	


