/*
 * LEDLightFamilyHelper.cpp
 * Header file for the misc functions used by both Parent and Child sketches
 * from the LED Light family project.
 */

#include <Arduino.h>
#include <Accelerometer.h>
#include <math.h>
#include <i2c.h>
#include <LEDLightFamilyHelper.h>
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>


extern byte accelData[6];  // x/y/z accel register data store here
extern int accelCount[3];  // Stores the 12-bit signed value
extern const byte AccelScale;
extern const byte AccelDataRate;

#define DEBUG

#define OFFPin		4  //set high to turn the whole device off

//so that each device knows what it is.
#define PARENT 1
#define CHILD 2

//OPERATING MODES

#define MODE_FREE_COLOR		0 // free hue & sat change by tilt
#define MODE_FULL_LOCK		1 // hue, sat and value (intensity) locked
#define MODE_COLORCYCLE		2 // gently cycle through the colour wheel (mood-light mode)
#define MODE_AUDIO			3 // audio response mode


//WAKE SLEEP states
#define STATE_WAKE	0
#define STATE_SLEEP	1



// reads the accelerometer and returns an array of 3 floating point readings.
void readRAWAccel(float (&data)[3])
{
	readRegisters(0x01, 6, &accelData[0]);  // Read the six data registers into data array

	/* For loop to calculate 12-bit ADC and g value for each axis */
	for (int i=0; i<6; i+=2)
	{
		accelCount[i/2] = ((accelData[i] << 8) | accelData[i+1]) >> 4;  // Turn the MSB and LSB into a 12-bit value
		if (accelData[i] > 0x7F)
		{  // If the number is negative, we have to make it so manually (no 12-bit data type)
			accelCount[i/2] = ~accelCount[i/2] + 1;
			accelCount[i/2] *= -1;  // Transform into negative 2's complement #
		}
		data[i/2] = (float) accelCount[i/2]/((1<<12)/(2*AccelScale));  // get actual g value, this depends on scale being set
	}
	/* For loop to print out values */

	#if defined DEBUG
	for (int i=0; i<3; i++)
	{
		Serial.print(accelValue[i], 4);  // Print g values
		Serial.print("\t");
		Serial.print(accelCount[i], DEC);  // Print adc count values, feel free to uncomment this line
		Serial.print("\t\t");
	}		
	Serial.println();
	#endif
}

//takes in an array of 3 raw readings and returns 3 smoothed readings (LP filter/moving average)
void SmoothAccel(float RC, float rawdata[3], float (&smoothdata)[3])
{
	//apply moving average to smoothe out accel readings
	for (int i=0; i<3; i++)
	{	
		smoothdata[i] = smoothdata[i] * RC + rawdata[i] * (1-RC);
		#if defined DEBUG
		Serial.print(smoothdata[i], 4);  // Print g values
		Serial.print("S\t");
		#endif
	}	
}


//takes in 7 byte RGB+cmd buffer and smoothes them out using LP filter/moving average if cmd bit is set
// depending on the child ID, will return etther mids (0,1,2) or treb (3,4,5) rgb bytes
void SmoothRGB(int child_id, float RC, byte (&buf)[7], byte &out_r, byte &out_g, byte &out_b)
{
	/*									 76543210  */
	const byte CMD_OFF_MASK = 			B10000000;
	const byte CMD_LPFON_MASK = 		B01000000;
	const byte CMD_MODE_CHANGE_MASK = 	B00100000;
	const byte CMD_MODE_FREECOLOR = 	B00000000;
	const byte CMD_MODE_COLORLOCK = 	B00001000;
	const byte CMD_MODE_COLORCYCLE = 	B00010000;
	const byte CMD_MODE_AUDIO = 		B00011000;

	#if defined DEBUG
		Serial.println("Inside SmoothRGB");
		Serial.print("Cmd byte received:-- ");
		Serial.println(buf[6], BIN);
	#endif
	if((buf[6] & CMD_LPFON_MASK) == CMD_LPFON_MASK)  //LPF is on so apply moving average
	{
		// note, shouldn't need to check child-id here, as if this mode is on, then we shouldn't
		// be in AUDIO mode anyway, so just return the bass RGBs
		#if defined DEBUG
			Serial.println("Fading to parent colour...");
		#endif
		out_r = int(out_r * RC) + int(buf[0] * (1-RC));
		out_g = int(out_g * RC) + int(buf[1] * (1-RC));
		out_b = int(out_b * RC) + int(buf[2] * (1-RC));
		
	} else //LPF bit is off, so just set the RGB to be exactly what was received.
	{
		#if defined DEBUG
			Serial.println("Fast-switching to parent colour...");
		#endif
		if (child_id == 0) //mids
		{
			out_r = buf[0];
			out_g = buf[1];
			out_b = buf[2];

		} else //treble
		{
			out_r = buf[3];
			out_g = buf[4];
			out_b = buf[5];	
		}

	}
}

//takes in 1 analog audio reading values and smoothes them out (LP filter/moving average)

/**
void SmoothAudio(float RC, int in_audio, int &out_audio)
{
	out_audio = int(out_audio * RC) + int(in_audio * (1-RC));
}
**/

//initialise MSGEQ7 chip
void initMSGEQ7(int analog, int strobe, int reset)
{
	#if defined DEBUG
		Serial.println("MSGEQ init");

		Serial.print("Analog Pin");
		Serial.println(analog);

		Serial.print("Strobe Pin");
		Serial.println(strobe);

		Serial.print("Reset Pin");
		Serial.println(reset);
		
	#endif
	//set up MSGEQ7 pins
	pinMode(analog, INPUT);
	pinMode(strobe, OUTPUT);
	pinMode(reset, OUTPUT);
	analogReference(DEFAULT);

	//turn off comms to MSGEQ7 to start with
	digitalWrite(reset, LOW);
	digitalWrite(strobe, HIGH);
	//prep the MSGEQ7 to read in the frequency bands
	digitalWrite(reset, HIGH);
	digitalWrite(reset, LOW);	
}



//read the EQ
void readEQ(int reset, int strobe, int analog, int (&bands)[7])
{
	#if defined DEBUG
		Serial.print("RAW EQ\t\t");
	#endif
	
	digitalWrite(reset, HIGH);// reset the MSGEQ7's counter
	//delay(5);
	digitalWrite(reset, LOW);
	
	//read in the bands
	for (int i = 0; i < 7; i++)
	{
		digitalWrite(strobe, LOW);
		//delayMicroseconds(35); // to allow the output to settle
		bands[i] = analogRead(analog);
		//delay(5);
		#if defined DEBUG
			Serial.print(bands[i]);
			Serial.print("\t");
		#endif
		digitalWrite(strobe, HIGH);
	}
	#if defined DEBUG
		Serial.println();
	#endif
}

//convert the 7 bands from the EQ into an RGB code. This routine, which runs on the parent, calculates RGBs for perent (bass)
//as well as the two children (mids and treble). These values then get send to the children via RF. 
void EQ_to_RGB(int bands[7], int lower, int upper, \
				byte &bass_r, byte &bass_g, byte &bass_b, \
				byte &mids_r, byte &mids_g, byte &mids_b, \
				byte &treb_r, byte &treb_g, byte &treb_b  )
{
	int mappedbands[7];
	
	byte tempmax;
	#if defined DEBUG
		Serial.print("MAPPED EQ\t");
	#endif
	
	//trim the readings -- makes sure that slient = off
	for(int i = 0; i < 7; i++)
	{
		mappedbands[i]=constrain(map(bands[i],lower,upper,0,255),0,255);
		#if defined DEBUG
			Serial.print(mappedbands[i]);
			Serial.print("\t");
		#endif	
	}

	#if defined DEBUG
		Serial.println();
	#endif
	
	//set BASS (PARENT) --- BLUE
	bass_r = 0;
	bass_g = 0;
	bass_b = max(mappedbands[0],mappedbands[1]);

	//set MIDS (CHILD 1) --- GREEN
	mids_r = 0; 
	//WARNING -- do not nest called to max();
	mids_g = max(mappedbands[2],mappedbands[3]);	
	//mids_g = max(tempmax,mappedbands[4]);	
	mids_b = 0;

	//set TREBLE (CHILD 2) -- RED
	treb_r = mappedbands[4];
	//treb_r = mids_g;
	//treb_r = map(max(mappedbands[5],mappedbands[6]),0,255,0,200); //max is too red, so take avg
	treb_g = 0;
	treb_b = 0;

}

//given x, y and sensitivity parameters, returns a saturation and hue.
void CalcSat_n_Hue(int sens, float x, float y, float &sat, float &hue)
{
	sat = sqrt(pow(x,2) + pow(y,2)) * sens; //multiplier calibrates title angle sensitivity
	
	hue = atan(x/y) *57296/1000; //convert from rad to deg.

	//need to correct the hue as arctan only returns the first quadrant's data.
	if (x < 0 && y > 0) 
	{
		hue = hue + 360.0;
	}
	
	if ((x > 0 && y < 0)||(x < 0 && y < 0)) 
	{
		hue = hue + 180.0;
	}
}


//slowly cycles through all the hues (colors)
void cycleHue(float &hue, int speed)
{
	hue = hue + 1.0;
	if (hue>359) hue = 0.0;
	delay(speed);
}


//given x and y parameters, returns a brightness
void CalcBrightness(int sens, float x, float y, float &brightness)
{
	brightness = sqrt(pow(x,2) + pow(y,2)) * sens; //multiplier calibrates title angle sensitivity	
}
/*
	This function ....
	Variables passed:
		h --- 0-360 degrees
		s --- 0-100
		v --- 0-100
	Returns:
		r --- red byte
		g --- green byte
		b --- blue byte
*/
void HSV_to_RGB(float h, float s, float v, byte &r, byte &g, byte &b)
{
	int i;
	float f,p,q,t;

	h = constrain(h, 0.0, 360.0);
	s = constrain(s, 0.0, 100.0);
	v = constrain(v, 0.0, 100.0);

	s /= 100;
	v /= 100;

	if (s == 0) {
	  // Achromatic (grey)
	  r = g = b = round(v*255);
	  return;
	}

	h /= 60.0; 
	i = floor(h); // sector 0 to 5
	f = h - (float)i; // factorial part of h
	p = v * (1.0 - s);
	q = v * (1.0 - s * f);
	t = v * (1.0 - s * (1 - f));
	switch(i) {
	case 0:
		r = round(255*v);
		g = round(255*t);
		b = round(255*p);
		break;
	case 1:
		r = round(255*q);
		g = round(255*v);
		b = round(255*p);
		break;
	case 2:
		r = round(255*p);
		g = round(255*v);
		b = round(255*t);
		break;
	case 3:
		r = round(255*p);
		g = round(255*q);
		b = round(255*v);
		break;
	case 4:
		r = round(255*t);
		g = round(255*p);
		b = round(255*v);
		break;
	default: // case 5:
		r = round(255*v);
		g = round(255*p);
		b = round(255*q);
	}
}

//Gamma corrects an R, G, or B value passed in
byte gamma_correction(byte input) {
	unsigned int multiplied = input * input;
	return multiplied / 256;
}

//turns TX hardware on or off
void setTX(int state)
{
	if(state == TX_ON)
	{
		//turn on transmitter
		Mirf.powerUpTx();		
	} 
	else 
	{
		if(state == TX_OFF)
		{
			//turn off transmitter
			Mirf.powerDown();
		}
	}
}
//transmits an array buffer over TX pin
//sends only infrequently when in SLEEP state
//frequency set with TX_SLEEP_DELAY_MILLI
void TXAccel(byte (&buf)[7], int state)
{
	static unsigned long lastTXtime; 
	
	if(state == STATE_WAKE)
	{
		if(millis() - lastTXtime > TX_WAKE_DELAY_MILLI)  //elapsed time has passed, so transmit.
		{
			#if defined DEBUG
				Serial.println("Fast Transmitting...");
			#endif
			Mirf.payload = sizeof(buf);
			Mirf.config();
			Mirf.send(buf);
			//while(Mirf.isSending())
			//{
				//do nothing.
			//}
			lastTXtime = millis();
			#if defined DEBUG
			Serial.println("SENT!");			
			#endif
		}
	} else  //STATE_SLEEP
	{
		if(millis() - lastTXtime > TX_SLEEP_DELAY_MILLI)  //elapsed time has passed, so transmit.
		{
			#if defined DEBUG
				Serial.println("Slow Transmitting...");
			#endif
			Mirf.payload = sizeof(buf);
			Mirf.config();
			Mirf.send(buf);
			//while(Mirf.isSending())
			//{
				//do nothing.
			//}
			lastTXtime = millis();
			#if defined DEBUG
			Serial.println("SENT!");			
			#endif
		}
	}
}

//same as above but sends without any delay at all!

void TXAccelAudioMode(byte (&buf)[7])
{
	#if defined DEBUG
		Serial.println("SUPER-Fast Transmitting...");
	#endif
	Mirf.payload = sizeof(buf);
	Mirf.config();
	Mirf.send(buf);
	//while(Mirf.isSending())
	//{
		//do nothing.
	//}
	#if defined DEBUG
	Serial.println("SENT!");			
	#endif

}




//turns RX hardware on or off

void setRX(int state)
{
	if(state == RX_ON)
	{
		//turn on transmitter
		Mirf.powerUpRx();			} 
	else 
	{
		if(state == RX_OFF)
		{
			//turn off transmitter
			Mirf.powerDown();
		}
	}
	
}
//receives an array of data from RX pin. 
//returns true if data was received, false otherwise (this implies its non-blocking)
boolean RXAccel(byte (&buf)[7])
{
	#if defined DEBUG
		Serial.println("Checking for RX'ed bytes...");
	#endif
    if (!Mirf.isSending() && Mirf.dataReady()) // Non-blocking
	{
		Mirf.getData(buf);  
		#if defined DEBUG
			// Message with a good checksum received
			Serial.print("------------ RECEIVED RF!!!------------------");
		#endif
		return true;
	} else 
	{
		return false;
	}
}

//flash lights quickly -- used to confirm tap
void FlashLights(int count, int RedLED, int GreenLED, int BlueLED, byte r, byte g, byte b)
{
	for(int i = 0; i < count; ++i)
	{
		analogWrite(RedLED, 0);
		analogWrite(GreenLED, 0);
		analogWrite(BlueLED, 0);
		delay(100);
		analogWrite(RedLED, r);
		analogWrite(GreenLED, g);
		analogWrite(BlueLED, b);
		delay(100);
	}
}

//debounces
boolean ModeButtonDebounce(int pin, boolean last, int btn_delay)
{
	boolean current = digitalRead(pin);
	if (last != current)
	{
		delay(btn_delay);
		current = digitalRead(pin);
	}
	return current;
}

//checks for button press
boolean ModeButtonPressed(boolean &last, boolean &current,int pin, int btn_delay)
{
	current = ModeButtonDebounce(pin, last, btn_delay);
	if (last == HIGH && current == LOW)
	{
		last = current;
		return 1;
	} else
	{
		last = current;
		return 0;
	}
}

//write PWN values out for RGB LEDs
void ShowLight(int RedLED, int GreenLED, int BlueLED, byte r, byte g, byte b)
{
	analogWrite(RedLED, r);
	analogWrite(GreenLED, g);
	analogWrite(BlueLED, b);
}

void changeMode(int &mode, int no_modes)
{
	mode++;
	if (mode > no_modes -1 ) mode = 0;
	
	#if defined DEBUG
		Serial.print("MODE =");
		Serial.println(mode);
	#endif
	
}

/**
 * processes the command by and sets the mode accordingly
 * @param cmd  command byte
 * @param mode resulting mode.
 * 
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

void ProcessCommand(byte cmd, int &mode)
{
	/*									 76543210  */
	const byte CMD_OFF_MASK = 			B10000000;
	const byte CMD_LPFON_MASK = 		B01000000;
	const byte CMD_MODE_CHANGE_MASK = 	B00100000;
	const byte CMD_MODE_FREECOLOR = 	B00000000;
	const byte CMD_MODE_COLORLOCK = 	B00001000;
	const byte CMD_MODE_COLORCYCLE = 	B00010000;
	const byte CMD_MODE_AUDIO = 		B00011000;

	#if defined DEBUG
		Serial.print("ProcessCommand: pre-mode: ");
		Serial.println(mode);
		Serial.print("ProcessCommand: pre-cmd: ");
		Serial.println(cmd, BIN);

	#endif


	if ((cmd & CMD_MODE_CHANGE_MASK) == CMD_MODE_CHANGE_MASK) //there is a command to change mode		
	{	
		#if defined DEBUG
			Serial.println("MODE CHANGE BIT SET");
		#endif	

		mode = int(cmd >> 3 & B00000011); //convert to correct mode.
	}

	
	#if defined DEBUG
		Serial.print("ProcessCommand: post-mode: ");
		Serial.println(mode);
		Serial.print("ProcessCommand: post-cmd: ");
		Serial.println(cmd, BIN);

	#endif

	if ((cmd & CMD_OFF_MASK) == CMD_OFF_MASK) //received off command!!
	{
		//turn off device!
		// commented out for now -- need to get this working on the parent
		//digitalWrite(OFFPin,HIGH);	
		
	}

}


/* This function will read the status of the tap source register.
   And print if there's been a single or double tap, and on what
   axis. */

void tapHandler(int whoami, int state, int &mode, byte &cmd)
{
	byte source = readRegister(0x22);//read the register to clear the int register
	const byte CMD_OFF_MASK = B10000000;
	
	if ((source & 0x08)==0x08)  // If DPE (double puls) bit is set
	{
		//double tap sensed
		//set cmd but to turn-off!
		cmd = (cmd | CMD_OFF_MASK);
	}
	
	//do nothing else for now...
	
	//mode++;
	//if (mode > no_modes -1 ) mode = 0;
	
	/*
	
	//Don't need all this, but left here just in case.
	
	byte source = readRegister(0x22);  // Reads the PULSE_SRC register

	if ((source & 0x10)==0x10)  // If AxX bit is set
	{
		if ((source & 0x08)==0x08)  // If DPE (double puls) bit is set
			Serial.print("    2 X");
		else
			Serial.print("1 X");

		if ((source & 0x01)==0x01)  // If PoIX is set
			Serial.println(" +");
		else
			Serial.println(" -");
	}
	if ((source & 0x20)==0x20)  // If AxY bit is set
	{
		if ((source & 0x08)==0x08)  // If DPE (double puls) bit is set
			Serial.print("    2 Y");
		else
			Serial.print("1 Y");

		if ((source & 0x02)==0x02)  // If PoIY is set
			Serial.println(" +");
		else
			Serial.println(" -");
	}
	if ((source & 0x40)==0x40)  // If AxZ bit is set
	{
		if ((source & 0x08)==0x08)  // If DPE (double puls) bit is set
			Serial.print("    2 Z");
		else
			Serial.print("1 Z");
		if ((source & 0x04)==0x04)  // If PoIZ is set
			Serial.println(" +");
		else
			Serial.println(" -");
	}

	*/
}
 
/* This function will read the p/l source register and
   print what direction the sensor is now facing.

   It does NOTHING else yet */
void portraitLandscapeHandler(int whoami, int state, int &mode)
{
	byte pl = readRegister(PL_STATUS_REG);  // Reads the PL_STATUS register
	switch((pl&0x06)>>1)  // Check on the LAPO[1:0] bits
	{
		case 0:
			Serial.print("Portrait up, ");
		break;
		case 1:
			Serial.print("Portrait Down, ");
		break;
		case 2:
			Serial.print("Landscape Right, ");
		break;
		case 3:
			Serial.print("Landscape Left, ");
		break;
	}
	if (pl&0x01)  // Check the BAFRO bit
		Serial.print("Back");
	else
		Serial.print("Front");
	
	if (pl&0x40)  // Check the LO bit
		Serial.print(", Z-tilt!");
	Serial.println();

	if ((whoami == CHILD) && (state == STATE_SLEEP))
		mode = MODE_FREE_COLOR;

}

/* This function will read the wake/sleep source register to 
   see whether sleep or mode is active, and update the boolean variable. */
void wakesleepHandler(int &state)
{
	byte ws = readRegister(SYSMOD_REG);  // Reads the SYSMOD register
	
	if((ws & 0x02) == 0x02)
	{
		state = STATE_SLEEP; //sleep mode
		#if defined DEBUG
			Serial.println("Entered SLEEP mode");
		#endif
	}
		
	if((ws & 0x01) == 0x01)
	{
		state = STATE_WAKE; //wake mode
		#if defined DEBUG
			Serial.println("Entered WAKE mode");
		#endif
	}
}

/* This function will read the FF_MT source register to 
   clear the latch on the accelerometer so that the wake interrupt fires.
	If this is being called by a CHILD, it will set the MODE to FREE COLOUR.
. */

void motionHandler(int whoami, int state, int &mode)
{
	byte motionreg = readRegister(FF_MT_SRC_1_REG);
	if ((whoami == CHILD) && (state == STATE_SLEEP))
		mode = MODE_FREE_COLOR;
}



