/*
	LED Monster
	Created 2013-02-17 by Bradley Wood
	$Id:: LEDMonster.ino 1820 2013-05-11 18:40:42Z                          $: 
	$Date:: 2013-05-11 19:40:42 +0100 (Sat, 11 May 2013)                    $: 
	$URL:: https://brad@svn.greyspark.com/svn/users/brad/trunk/LEDMonster/L#$:

TODO: Add longer introduction to the code and libs.

Copyright (c) 2013 Bradley Wood
Released as Open Source under GNU GPL v3, see <http://www.gnu.org/licenses/>

A lot of this was inspired by Graham Richter's Chronos project - dankie boet!
*/

//TODO: decide between define's and constants -- its a bit of a mishmash at the moment...

//DEBUG code -- remove this compiler directive to turn of Serial debug msgs
#define DEBUG

//turn on the below directive to have the RTC chip set from the compiler time
#define SETRTCFROMPC

//Number for each major mode
#define Mode_EQ			0
#define Mode_Clock		1
#define NoOfModes		2 // the number of modes to cycle through on button press
#define Start_Mode		Mode_Clock

//Number for each EQ (sub) mode
#define EQ_Mode_0		0
#define EQ_Mode_1		1
#define EQ_Mode_2		2
#define EQ_Mode_3		3
#define EQ_Mode_4		4
#define EQ_Mode_5		5
#define EQ_Mode_6		6
#define EQ_Mode_7		7
#define EQ_Start_Mode	EQ_Mode_0
#define EQ_NoOfModes	8 // the number of EQ modes to cycle through on timer
#define EQ_Mode_Time_Period 10000 //1 min per EQ mode

//Mode button configs
#define Pin_Mode_Button	8  //digital pin
#define Debounce_Delay	5 //milliseconds

//SPI pin constants - common to *all* SPI devices used in the sketch
#define SPI_MOSI	11
#define SPI_MISO	12
#define SPI_CLCK	13

//note SS pins are defined for each SPI bus user further down...

//Define how many LED matrices are connected to create the frame.
#define NUM_BOARDS	4
//LED Matrix SPI SS Pin
#define LED_MATRIX_SS_PIN	6

//define speakjet constants
// DO NOT CHANGE THE BELOW  3 lines WITHOUT ALSO CHANGING THE define in Crond.h!!!
#define SJ_TX_PIN	5
#define SJ_RX_PIN	9
#define SPEAKER_POWER	A1 //set high to turn on speaker, low to turn off....

#include <avr/pgmspace.h>
#include <SFRGBLEDMatrix.h> // Fabio Ornella's excellent lib + firmware - see https://github.com/fornellas/RGB_Backpack_4096_Colors
#include <SPI.h>
#include <Wire.h>  
#include <RTClib.h>
#include <SoftwareSerial.h> //needed by the speakjet

//Custom libraries - one for each mode...
#include <Crond.h>
#include <LEDClock.h> 
#include <LEDEqualiser.h>

//used to make the cron run only once every second (and not more frequently)
boolean ranCronCommand = false;

//===LED Matrix configs===
SFRGBLEDMatrix *ledMatrix; //array of LED matrices

//===SPEAKJET configs===
//TODO: sort out speaker on, speaker off timer -- rather than using delay()
SoftwareSerial SJ =  SoftwareSerial(SJ_RX_PIN, SJ_TX_PIN);

//===MSGEQ7 configs===
//Set up variables for MSGEQ7 input
const int EQAnalogPin = 0; // read from multiplexer using analog input 0
const int EQStrobePin = 2; // strobe is attached to digital pin 2
const int EQResetPin = 3; // reset is attached to digital pin 3
int EQSpectrumValue[7]; // to hold a2d values from MSGEQ7
int EQHeight[7]; // to hold the map()'ed values from the above array.
const int EQLowerMapBound = 150; // Set the lower threshold for map() to reduce noise when "silent" 
const int EQUpperMapBound = 1023; // Set the upper threshold for map() to reduce noise when "silent" 

//===RTC configs====
//see library for details.
RTC_DS1307 RTC; //instantiate RTC object
//used to store the colour for the current time
int ClockFaceColor; 

//global time vars
unsigned int hour, minute, second, week; 
unsigned int lastmin = 99; //set it something it could never ordinary be to triger a cron run on first loop.
RGBT time_col;
DateTime now;

//=== Debounce setup =====
boolean LastButtonState = HIGH; //the state of the button last time we checked
boolean CurrentButtonState = HIGH; //the current reading from the button pin

//=== Mode change timer setup =====
//used to time the cycling through modes...
long StopWatch;
//set the current mode to the Start_Mode...
int Mode = Start_Mode; //sets the start mode...
int EQ_Mode = EQ_Start_Mode; //set the start EQ Mode
boolean ModeChanged = 1; //used to flag a mode-specific init routine must be run...
boolean EQ_ModeChanged = 1; //used to flag a mode-specific init routine must be run...

int OldMode;
int EQ_OldMode;

//TODO: turn these into defines (minor optimisation, probably overkill)
void LED_ON()
{
	digitalWrite(LED_MATRIX_SS_PIN,LOW); //enable LED comms.
}

void LED_OFF()
{
	digitalWrite(LED_MATRIX_SS_PIN,HIGH); //disable LED comms.
}

/*
	This function  cycles to the next mode
	Variables passed:
		amode == the current Mode
	Returns:
		the next mode in the cycle.
	
	TODO: Merge these to Mode Cyclers and parametrise which mode to cycle.

*/
int CycleEQMode(int amode)
{
	//TODO: replace this with a clever 1 line modulo formula.
	#if defined DEBUG
		Serial.print("amode=");
		Serial.print(amode);
	#endif
	
	amode=amode++;
	#if defined DEBUG
		Serial.print("Amode incremented; amode=");
		Serial.print(amode);
	#endif
	
	if(amode>EQ_NoOfModes-1)
	{
		amode=0;
	}
	#if defined DEBUG
		Serial.print("Amode moduloed returning amode=");
		Serial.print(amode);
	#endif
	return amode;
}


/*
	This function  cycles to the next mode
	Variables passed:
		amode == the current Mode
	Returns:
		the next mode in the cycle.

	TODO: Merge these to Mode Cyclers and parametrise which mode to cycle.


*/
int CycleMajorMode(int amode)
{
	//TODO: replace this with a clever 1 line modulo formula.
	#if defined DEBUG
		Serial.print("amode=");
		Serial.print(amode);
	#endif
	
	amode=amode++;
	#if defined DEBUG
		Serial.print("Amode incremented; amode=");
		Serial.print(amode);
	#endif
	
	if(amode>NoOfModes-1)
	{
		amode=0;
	}
	#if defined DEBUG
		Serial.print("Amode moduloed returning amode=");
		Serial.print(amode);
	#endif
	return amode;
}

/*
	This function returns true after the EQ timeout has passed 
*/
boolean EQ_ModeTimeUp()
{
	if((millis() - StopWatch > EQ_Mode_Time_Period))
	{
		#if defined DEBUG
			Serial.println("Timed out");
		#endif
		
		StopWatch = millis();
		return 1;
	
	} else
	{
		return 0;
	}
}

/*
	This function returns the debounced state of the Mode button.
	
	It takes in the button state from the last loop.
	
	Note, as we are using the onboard pullup resistor, the pin reads:
		HIGH (1) when the switch is open and
		LOW (0) when it is closed (pressed)
*/
boolean ModeButtonDebounce(boolean last)
{
	boolean current = digitalRead(Pin_Mode_Button);
	if (last != current)
	{
		delay(Debounce_Delay);
		current = digitalRead(Pin_Mode_Button);
	}
	return current;
}
/*
	This function returns true if the button was pressed
*/
boolean ModeButtonPressed()
{
	CurrentButtonState = ModeButtonDebounce(LastButtonState);
	if (LastButtonState == HIGH && CurrentButtonState == LOW)
	{
		LastButtonState = CurrentButtonState;
		return 1;
	} else
	{
		LastButtonState = CurrentButtonState;
		return 0;
	}
}

/*
	MAIN SETUP ROUTINE
*/
void setup() 
{
	//initialise DEBUG Serial connection
	#if defined DEBUG
		Serial.begin(57600);
	#endif
	
	//---- RTC setup ----
	Serial.begin(57600); //set up RTC via I2C protocol
	Wire.begin();
	RTC.begin();
	if(!RTC.isrunning())
	{
		#if defined DEBUG
			Serial.println("RTC is NOT running!");
		#endif
	}
	
	// following line sets the RTC to the date & time this sketch was
	// compiled but only if SETRTCFROMPC is defined
	#if defined SETRTCFROMPC
		RTC.adjust(DateTime(__DATE__, __TIME__));	
	#endif
			
	//---- SpeakJet & Speaker setup ----
	SJ.begin(9600); //start the speakjet serial connection.
	// delay a bit
	delay(1000); //let it say "ready"
	pinMode(SPEAKER_POWER,OUTPUT);
	digitalWrite(SPEAKER_POWER,HIGH); //turn on the speaker.
	SJ.println("HELLO");
	delay(1000);
	digitalWrite(SPEAKER_POWER,LOW); //turn off the speaker.

	//---- Push button setup ----
	//TODO:  call analogReference(INTERNAL) here -- remove from whichever lib it's in/		
	//set up Mode Button for input
	pinMode(Pin_Mode_Button,INPUT);
	//write HIGH to this INPUT put to turn on the pullup resistor
	digitalWrite(Pin_Mode_Button,HIGH); 

	//---- SPI setup ----	
	//set up SPI -- note these settings are *shared* for the SD, WiFi and LED MATRIX
	//they should never change between SPI devices.
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV4);
	SPI.setBitOrder(MSBFIRST);
		
	pinMode(SPI_MOSI, OUTPUT);
	pinMode(SPI_MISO, INPUT);
	pinMode(SPI_CLCK, OUTPUT);
		
	//---- LED Matrixes setup ----
	
	#if defined DEBUG
		Serial.println("starting LED Matrix.");
	#endif
	
	ledMatrix=new SFRGBLEDMatrix(LED_MATRIX_SS_PIN, NUM_BOARDS, 1);
	#if defined DEBUG
		Serial.println("instantiated LED Matrix object");
	#endif
	
	LED_ON(); //enable LED matrix comms	
	#if defined DEBUG
		Serial.println("Called LED_ON()");
	#endif
	
	ledMatrix->clear();
	#if defined DEBUG
		Serial.println("called ledMatrix->clear()");
	#endif

	ledMatrix->print(WHITE,1,1,5,"HELLO!");
	#if defined DEBUG
		Serial.println("sent HELLO to matrix");
	#endif
	ledMatrix->show();	
	#if defined DEBUG
		Serial.println("called ledMatrix->show()");
	#endif

	LED_OFF(); //disable LED matrix comms
	#if defined DEBUG
		Serial.println("called LED_OFF()");
	#endif

#if defined DEBUG
	Serial.println("setup() complete... ");
#endif

}

/*
	MAIN SYSTEM LOOP
*/
void loop() 
{	
	now = RTC.now();
	hour = now.hour();
	minute = now.minute();
	second = now.second();
	week = now.dayOfWeek();
	time_col = getTimeColor(hour,minute);
	
	ClockFaceColor = RGB(time_col.r,time_col.g,time_col.b);
	
	//--- Run cronjobs, only every minute----
	if((!ranCronCommand) && (lastmin!=minute))
	{
		runCron();
		lastmin = minute;
		ranCronCommand = true;
	} else 
	{
		ranCronCommand = false;
	}
	//---check for button presses--
	if(ModeButtonPressed())
	{
		#if defined DEBUG
			Serial.println("Mode changed (button press).");
		#endif
		//cycle the Mode variable to the next mode.
		
		OldMode = Mode;
		Mode = CycleMajorMode(OldMode);
		//flag the mode as having changed in order to trigger the inits...
		ModeChanged = 1; 
	}

#if defined DEBUG
	Serial.println("Debug test goes here ...");
#endif
	
	#if defined DEBUG
		Serial.print("Mode = ");
		Serial.print(Mode);
		Serial.print(" EQ Mode = ");
		Serial.println(EQ_Mode);

	#endif
		
	switch(Mode)
	{
		//TODO: if Mode changed, clean up after last mode..
		
		case Mode_EQ:	
			// EQ code 
			if(ModeChanged)
			{
				#if defined DEBUG
					Serial.println("Mode just changed to EQ, runing init...");
				#endif
				digitalWrite(SPEAKER_POWER,HIGH); //turn on the speaker.
				SJ.println("Equal-I");
				delay(1000);
				SJ.println("ZER");
				delay(1000);
				digitalWrite(SPEAKER_POWER,LOW); //turn off the speaker.
				
				mode_Equaliser_init();
				ModeChanged = 0;
			}
			#if defined DEBUG
				Serial.println("Now running the regular EQ routine");
			#endif
			//if the EQ Mode time up, change the EQ Mode variable...
			if(EQ_ModeTimeUp())
			{
				#if defined DEBUG
					Serial.println("EQ Mode timed out.");
				#endif
				//cycle the Mode variable to the next mode.
				int EQ_OldMode = EQ_Mode;
				EQ_Mode = CycleEQMode(EQ_OldMode);
				//flag the mode as having changed in order to trigger the EQ 
				//mode transition animation... 
				EQ_ModeChanged = 1; 
				//reset the stopwatch
				StopWatch = millis();
				#if defined DEBUG
					Serial.print("Current EQ mode is:");
					Serial.print(EQ_Mode);
				#endif
			}
			
			switch(EQ_Mode)
			{
				case EQ_Mode_0:
				if(EQ_ModeChanged){
					//mode just changed, do the wipe
					ledMatrix->CRT(GREEN);
					ledMatrix->clear();
					
					EQ_ModeChanged = 0; 
				}
				//call with width = 2;
				mode_Equaliser1(2);
				break;


				case EQ_Mode_1:
				if(EQ_ModeChanged){
					//mode just changed, do the wipe
					ledMatrix->CRT(RED);
					ledMatrix->clear();
					
					EQ_ModeChanged = 0; 

				}
				//call with width = 4;
				mode_Equaliser2(4);
				break;

				case EQ_Mode_2:
				if(EQ_ModeChanged){
					//mode just changed, do the wipe
					ledMatrix->CRT(BLUE);
					ledMatrix->clear();
					
					EQ_ModeChanged = 0; 
				}
				mode_Equaliser1(2);
				break;

				case EQ_Mode_3:
				if(EQ_ModeChanged){
					//mode just changed, do the wipe
					ledMatrix->CRT(YELLOW);
					ledMatrix->clear();
					
					EQ_ModeChanged = 0; 
				}
				mode_Equaliser3(4);
				break;
				case EQ_Mode_4:
				if(EQ_ModeChanged){
					//mode just changed, do the wipe
					ledMatrix->CRT(GREEN);
					ledMatrix->clear();
					
					EQ_ModeChanged = 0; 
				}
				//call with width = 2;
				mode_Equaliser1(4);
				break;


				case EQ_Mode_5:
				if(EQ_ModeChanged){
					//mode just changed, do the wipe
					ledMatrix->CRT(RED);
					ledMatrix->clear();
					
					EQ_ModeChanged = 0; 

				}
				//call with width = 4;
				mode_Equaliser1(2);
				break;

				case EQ_Mode_6:
				if(EQ_ModeChanged){
					//mode just changed, do the wipe
					ledMatrix->CRT(BLUE);
					ledMatrix->clear();
					
					EQ_ModeChanged = 0; 
				}
				mode_Equaliser2(2);
				break;

				case EQ_Mode_7:
				if(EQ_ModeChanged){
					//mode just changed, do the wipe
					ledMatrix->CRT(YELLOW);
					ledMatrix->clear();
					
					EQ_ModeChanged = 0; 
				}
				mode_Equaliser3(2);
				break;
				
			}
		break;

		case Mode_Clock:
			// Clock code
			if(ModeChanged)
			{
				#if defined DEBUG
					Serial.println("Mode just changed to Clock mode runing init...");
				#endif
				digitalWrite(SPEAKER_POWER,HIGH); //turn on the speaker.
				SJ.println("CLOCK.");
				delay(1000);
				digitalWrite(SPEAKER_POWER,LOW); //turn off the speaker.`
				mode_Clock_init(); 
				ModeChanged = 0;
			}
			#if defined DEBUG
				Serial.println("Now running the regular Clock routine");
			#endif			
			mode_Clock(); 
		break;
	}
	#if defined DEBUG
		Serial.print("Free memory equals: ");
		Serial.print(memoryFree());  // print the free memory
		Serial.println();
	#endif

}

#if defined DEBUG
	// variables created by the build process when compiling the sketch
	extern int __bss_end;
	extern void *__brkval;

	// function to return the amount of free RAM
	int memoryFree()
	{
	  int freeValue;

	  if((int)__brkval == 0)
	     freeValue = ((int)&freeValue) - ((int)&__bss_end);
	  else
	    freeValue = ((int)&freeValue) - ((int)__brkval);

	  return freeValue;
	}
#endif
