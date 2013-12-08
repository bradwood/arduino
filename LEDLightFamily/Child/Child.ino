/*
	LEDLightFamily -- Child Module
	Created 2013-03-19 by Bradley Wood
	$Id:: Child.ino 2041 2013-10-13 08:04:58Z                               $: 
	$Date:: 2013-10-13 09:04:58 +0100 (Sun, 13 Oct 2013)                    $: 
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

//DEBUG code -- remove this compiler directive to turn of Serial debug msgs
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

const int whoami = CHILD;

//IMPORTANT INCREMENT THIS FOR EACH CHILD YOU BUILD -- currently only 0 and 1
//supported 1 = treble, 0 = mids (the parent does bass)
const int child_id = 0; 

//All common functions used by either Parent or Child (as well as some specific
//to either the Child or the Parent)

#include <LEDLightFamilyHelper.h>

//OPERATING MODES

//these are cycled through via trapping a tap in any dimention using the
//tap handler capability of the accelerometer

#define NUM_MODES			3 // 3 modes for now

#define MODE_FREE_COLOR		0 // free hue & sat change by tilt
#define MODE_FULL_LOCK		1 // hue, sat and value (intensity) locked
#define MODE_COLORCYCLE		2 // gently cycle through the colour wheel (mood-light mode)

#define MODE_COLORCYCLE_SPEED	20 // no of milliseconds delay before moving to next degree of hue.

int mode = MODE_FREE_COLOR; //start out in free colour mode

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


#define RX_ON	0
#define RX_OFF	1

#define RXADDR "CHIL1" //the address we are receiving at

int RX_state = RX_ON;
byte buf[7]; //4 byte receiver buffer
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

Bit 2-0: Reserved for later use. */

const byte CMD_OFF_MASK = B1000000;
const byte CMD_LPFON_MASK = B0100000;

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
const int AccelInt1Pin = 2;  // These can be changed, 2 and 3 are the Arduinos ext int pins
const int AccelInt2Pin = 3;

byte accelData[6];  // x/y/z accel register data store here
int accelCount[3];  // Stores the 12-bit signed value
float accelValue[3];  // Stores the real accel value in g's

//stores the real accel values in g's, but smoothed out using a LP
//filter/moving average.
float smoothedValue[3]; 
const float LPF_RC_Acc = 0.6; // Acceleromter: value between 0 and 1 used for LP filter. 0 = no filtering. 1 equals = no reading.
const float LPF_RC_RGB = 0.6; // RGB: value between 0 and 1 used for LP filter. 0 = no filtering. 1 equals = no reading.

const int TiltSensitivity = 300; // The higher the number, the less tilt required.

/* 
Color changes will work by tracking the object's "tilt" in 3D-space using
the accelerometers x, y vectors. The tilt will be mapped to a 2D vector
which will determine the hue and saturation. 
in HSB cyclindrical space with the vector's  x- and y-components determining the
hue and the degree of tilt from the horizontal determining the saturation. 

*/
byte red,g_red,green,g_green,blue,g_blue; // number to be sent to each LED to drive PWM setting. 0-1023
float hue = 0.0; // will hold an angle -- range 0-767  TODO: map to/from radians
float sat, brightness = 0.0; //will hold saturation (0-254) and brightness (height) 

unsigned int hi, si, bi; //integer versions of hue, saturation and brightness.

void setup ()
{
	#if defined DEBUG
		Serial.begin(115200);
		Serial.println("CHILD");
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
	Mirf.setRADDR((byte *)RXADDR);
	Mirf.payload = sizeof(buf);
	Mirf.config();
	
	//set up LED pins
	pinMode(RedLED, OUTPUT);
	pinMode(GreenLED, OUTPUT);
	pinMode(BlueLED, OUTPUT);

	/* Set up the interrupt pins, they're set as active high, push-pull */
	pinMode(AccelInt1Pin, INPUT);
	digitalWrite(AccelInt1Pin, LOW);
	pinMode(AccelInt2Pin, INPUT);
	digitalWrite(AccelInt2Pin, LOW);

	//initialise and calibrate the accelerometer
	initMMA8452(AccelScale, AccelDataRate);
	CalibrateAccel();
}


void loop()
{
	//fix for intermitting crashes of the RF infrastructure.
	Mirf.powerDown();
	Mirf.init();

	buf[6] = 0;
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
	
	// read Int2 on the accelerometer and then set mode and wake/sleep accordingly
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
			tapHandler(whoami, state, mode,buf[6]); //will probably do nothing to the mode... or might set it to FREE_COLOR
		}
		
		if((accel_event_int_register & SRC_FF_MT_1_MASK)==SRC_FF_MT_1_MASK) //motion detected
		{
			#if defined DEBUG
				Serial.println("-------------MOTION MOTION MOTION----------------");
			#endif
			
			motionHandler(whoami, state, mode); //read the register to clear the latch, this should force the FREE_COLOR mode.
		}
		
		if((accel_event_int_register & SRC_LNDPRT_MASK)==SRC_LNDPRT_MASK) //portrait/landscape detected
		{
			#if defined DEBUG
				Serial.println("-------------PORTRAIT LANDSCAPE----------------");
			#endif
			portraitLandscapeHandler(whoami, state, mode); //will set the mode to FREE colour -- can also turn off the device if a 180 degree flip occured....
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
			
			//turn off RF TX infrastruture.
			setRX(RX_OFF);
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

						SmoothAccel(LPF_RC_Acc, accelValue, smoothedValue);

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
					}
				break;

				case MODE_FULL_LOCK:
					#if defined DEBUG
						Serial.println("MODE_FULL_LOCK");
					#endif
					
					DEBUGPRINTING;
					
					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);
					
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

				break;
			}

		break;
		
		case STATE_SLEEP:
			#if defined DEBUG
				Serial.println("SLEEP");
			#endif
			
			//turn on RX infrastructure.
			setRX(TX_ON);
			
		
			switch (mode) 
			{
				case MODE_FREE_COLOR:
					#if defined DEBUG
						Serial.println("MODE_FREE_COLOR");
					#endif
					
					//Receive RF RGB codes
				    if (RXAccel(buf)) // Non-blocking -- is there a message to process?
					{
						#if defined DEBUG
							Serial.print("Pre ProcessCommand call: mode: ");
							Serial.println(mode);
						#endif
						
						ProcessCommand(buf[6],mode); //change the mode if the parent says so
						
						#if defined DEBUG
							Serial.print("Post ProcessCommand call: mode: ");
							Serial.println(mode);
						#endif

						SmoothRGB(child_id,LPF_RC_RGB, buf, g_red, g_green, g_blue);
					}
					DEBUGPRINTING;
					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);
				break;

				case MODE_FULL_LOCK:
					#if defined DEBUG
						Serial.println("MODE_FULL_LOCK");
					#endif
					
					//Receive RF RGB codes
				    if (RXAccel(buf)) // Non-blocking -- is there a message to process?
					{
						#if defined DEBUG
							Serial.print("Pre ProcessCommand call: mode: ");
							Serial.println(mode);
						#endif
						
						ProcessCommand(buf[6],mode); //change the mode if the parent says so
						
						#if defined DEBUG
							Serial.print("Post ProcessCommand call: mode: ");
							Serial.println(mode);
						#endif

						SmoothRGB(child_id,LPF_RC_RGB, buf, g_red, g_green, g_blue);
					}
					DEBUGPRINTING;
					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);
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

					//Receive RF RGB codes
				    if (RXAccel(buf)) // Non-blocking -- is there a message to process?
					{
						#if defined DEBUG
							Serial.print("Pre ProcessCommand call: mode: ");
							Serial.println(mode);
						#endif
						
						ProcessCommand(buf[6],mode); //change the mode if the parent says so
						
						#if defined DEBUG
							Serial.print("Post ProcessCommand call: mode: ");
							Serial.println(mode);
						#endif

						SmoothRGB(child_id, LPF_RC_RGB, buf, g_red, g_green, g_blue);
					}
					else 
					{
						g_red = gamma_correction(red);
						g_green = gamma_correction(green);
						g_blue = gamma_correction(blue);
					}
					
					DEBUGPRINTING;
					//turn on the LEDs
					ShowLight(RedLED, GreenLED, BlueLED, g_red,g_green,g_blue);
				break;
			}
		break;
	}
}






