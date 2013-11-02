#include <Arduino.h>
#include <LEDEqualiser.h>
#include <SFRGBLEDMatrix.h> // Fabio Ornella's excellent lib+ firmware - see https://github.com/fornellas/RGB_Backpack_4096_Colors
#include <SPI.h>

//TODO: Optimise screen refresh rate on this -- bitwise operations through out, and less loops, etc...
//TODO: get rid of switch statements -- replace with loops...
//uncomment this to get debug text over Serial.
//#define DEBUG

extern SFRGBLEDMatrix *ledMatrix;
extern int ClockFaceColor;

/*
	This function sets up the MSGEQ7 chip.
	It sets the analog reference, configures the pins and prepares the chip
	for comms.
	It is not called from setup(), but only when this mode is entered...
*/
void mode_Equaliser_init()
{
	#if defined DEBUG
		Serial.println("inside mode_Equaliser_init");
	#endif
	//set up MSGEQ7 pins
	pinMode(EQAnalogPin, INPUT);
	pinMode(EQStrobePin, OUTPUT);
	pinMode(EQResetPin, OUTPUT);
	analogReference(DEFAULT); //TODO consider moving this call to setup()

	//turn off comms to MSGEQ7 to start with
	digitalWrite(EQResetPin, LOW);
	digitalWrite(EQStrobePin, HIGH);
	//prep the MSGEQ7 to read in the frequency bands
	digitalWrite(EQResetPin, HIGH);
	digitalWrite(EQResetPin, LOW);	
	//Note: all SPI setup done outside this lib...
	ledMatrix->clear(); //clear the screen ready for the mode to kick in.
}

/*
	mode_Equaliser2()
	
	This function displays another equaliser mode
	width is either 2 or 4 and makes the bar that wide as a result.
	Only 2 and 4 width's implemented.
	Returns: nothing
*/
void mode_Equaliser1(int width)
{	
	
	
	#if defined DEBUG
		Serial.println("inside mode_Equaliser1");
	#endif

	//read in the bands
	for (int i = 0; i < 7; i++)
	{
		digitalWrite(EQStrobePin, LOW);
		EQSpectrumValue[i] = analogRead(EQAnalogPin);
		EQHeight[i] = map(EQSpectrumValue[i],EQLowerMapBound,EQUpperMapBound,0,7);
		digitalWrite(EQStrobePin, HIGH);
	}

	//now write out the LED array
	for (int i = 0; i < 7; i++)	
	{
		switch(i)
		{
			case 0:	
			//write the low frequency band
			//TODO: make this cooler, prettier...
			//TODO: use lower level routine... is line() expensive??!? update: it appears not
			//TODO: make this faster... how?
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 3,0,7);
			ledMatrix->line_vert(BLACK, 4,0,7);
			ledMatrix->line_vert(BLACK, 5,0,7);
			ledMatrix->line_vert(BLACK, 6,0,7);
			
			
			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(RED, 3,7-EQHeight[i],7);
					ledMatrix->line_vert(RED, 6,7-EQHeight[i],7);				
				}
				ledMatrix->line_vert(RED, 4,7-EQHeight[i],7);
				ledMatrix->line_vert(RED, 5,7-EQHeight[i],7);				
			}
			break;
			
			case 1:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 7,0,7);
			ledMatrix->line_vert(BLACK, 8,0,7);
			ledMatrix->line_vert(BLACK, 9,0,7);
			ledMatrix->line_vert(BLACK, 10,0,7);
			
			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(ORANGE, 7,7-EQHeight[i],7);
					ledMatrix->line_vert(ORANGE, 10,7-EQHeight[i],7);				
					
				}				
				ledMatrix->line_vert(ORANGE, 9,7-EQHeight[i],7);
				ledMatrix->line_vert(ORANGE, 8,7-EQHeight[i],7);
			}
			break;
		
			case 2:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 11,0,7);
			ledMatrix->line_vert(BLACK, 12,0,7);
			ledMatrix->line_vert(BLACK, 13,0,7);
			ledMatrix->line_vert(BLACK, 14,0,7);
			
			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(BLUE, 11,7-EQHeight[i],7);
					ledMatrix->line_vert(BLUE, 14,7-EQHeight[i],7);				
				}
				ledMatrix->line_vert(BLUE, 12,7-EQHeight[i],7);				
				ledMatrix->line_vert(BLUE, 13,7-EQHeight[i],7);
			}
			break;
		
			case 3:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 15,0,7);
			ledMatrix->line_vert(BLACK, 16,0,7);
			ledMatrix->line_vert(BLACK, 17,0,7);
			ledMatrix->line_vert(BLACK, 18,0,7);
			
			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(GREEN, 15,7-EQHeight[i],7);
					ledMatrix->line_vert(GREEN, 18,7-EQHeight[i],7);									
				}
				
				ledMatrix->line_vert(GREEN, 16,7-EQHeight[i],7);				
				ledMatrix->line_vert(GREEN, 17,7-EQHeight[i],7);
			}
			break;
		
			case 4:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 19,0,7);
			ledMatrix->line_vert(BLACK, 20,0,7);
			ledMatrix->line_vert(BLACK, 21,0,7);
			ledMatrix->line_vert(BLACK, 22,0,7);
			
			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(PINK, 19,7-EQHeight[i],7);
					ledMatrix->line_vert(PINK, 22,7-EQHeight[i],7);				
				}
				ledMatrix->line_vert(PINK, 20,7-EQHeight[i],7);				
				ledMatrix->line_vert(PINK, 21,7-EQHeight[i],7);
			}
			break;
		
			case 5:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 23,0,7);
			ledMatrix->line_vert(BLACK, 24,0,7);
			ledMatrix->line_vert(BLACK, 25,0,7);
			ledMatrix->line_vert(BLACK, 26,0,7);
			
			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(CYAN, 23,7-EQHeight[i],7);
					ledMatrix->line_vert(CYAN, 26,7-EQHeight[i],7);
				}
				ledMatrix->line_vert(CYAN, 24,7-EQHeight[i],7);
				ledMatrix->line_vert(CYAN, 25,7-EQHeight[i],7);
			}
			break;

			case 6:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 27,0,7);
			ledMatrix->line_vert(BLACK, 28,0,7);
			ledMatrix->line_vert(BLACK, 29,0,7);
			ledMatrix->line_vert(BLACK, 30,0,7);
			
			if(EQHeight[i]>0){
				if(width == 4)
				{	
					ledMatrix->line_vert(WHITE, 27,7-EQHeight[i],7);
					ledMatrix->line_vert(WHITE, 30,7-EQHeight[i],7);				
				
				}
				ledMatrix->line_vert(WHITE, 28,7-EQHeight[i],7);				
				ledMatrix->line_vert(WHITE, 29,7-EQHeight[i],7);
			}
			break;
		}
	}
	//now display the LED buffer
	ledMatrix->show();
}

/*
	mode_Equaliser2()
	
	This function displays another equaliser mode
	No variables passed but does manipulate all EQ* variables
	Returns: nothing
*/
void mode_Equaliser2(int width)
{	


	#if defined DEBUG
		Serial.println("inside mode_Equaliser1");
	#endif

	//read in the bands
	for (int i = 0; i < 7; i++)
	{
		digitalWrite(EQStrobePin, LOW);
		EQSpectrumValue[i] = analogRead(EQAnalogPin);
		EQHeight[i] = map(EQSpectrumValue[i],EQLowerMapBound,EQUpperMapBound,0,7);
		digitalWrite(EQStrobePin, HIGH);
	}

	//now write out the LED array
	for (int i = 0; i < 7; i++)	
	{
		switch(i)
		{
			case 0:	
			//write the low frequency band
			//TODO: make this cooler, prettier...
			//TODO: use lower level routine... is line() expensive??!? update: it appears not
			//TODO: make this faster... how?
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 3,0,7);
			ledMatrix->line_vert(BLACK, 4,0,7);
			ledMatrix->line_vert(BLACK, 5,0,7);
			ledMatrix->line_vert(BLACK, 6,0,7);


			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(ClockFaceColor, 3,7-EQHeight[i],7);
					ledMatrix->line_vert(ClockFaceColor, 6,7-EQHeight[i],7);				
				}
				ledMatrix->line_vert(ClockFaceColor, 4,7-EQHeight[i],7);
				ledMatrix->line_vert(ClockFaceColor, 5,7-EQHeight[i],7);				
			}
			break;

			case 1:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 7,0,7);
			ledMatrix->line_vert(BLACK, 8,0,7);
			ledMatrix->line_vert(BLACK, 9,0,7);
			ledMatrix->line_vert(BLACK, 10,0,7);

			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(ClockFaceColor, 7,7-EQHeight[i],7);
					ledMatrix->line_vert(ClockFaceColor, 10,7-EQHeight[i],7);				

				}				
				ledMatrix->line_vert(ClockFaceColor, 9,7-EQHeight[i],7);
				ledMatrix->line_vert(ClockFaceColor, 8,7-EQHeight[i],7);
			}
			break;

			case 2:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 11,0,7);
			ledMatrix->line_vert(BLACK, 12,0,7);
			ledMatrix->line_vert(BLACK, 13,0,7);
			ledMatrix->line_vert(BLACK, 14,0,7);

			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(ClockFaceColor, 11,7-EQHeight[i],7);
					ledMatrix->line_vert(ClockFaceColor, 14,7-EQHeight[i],7);				
				}
				ledMatrix->line_vert(ClockFaceColor, 12,7-EQHeight[i],7);				
				ledMatrix->line_vert(ClockFaceColor, 13,7-EQHeight[i],7);
			}
			break;

			case 3:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 15,0,7);
			ledMatrix->line_vert(BLACK, 16,0,7);
			ledMatrix->line_vert(BLACK, 17,0,7);
			ledMatrix->line_vert(BLACK, 18,0,7);

			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(ClockFaceColor, 15,7-EQHeight[i],7);
					ledMatrix->line_vert(ClockFaceColor, 18,7-EQHeight[i],7);									
				}

				ledMatrix->line_vert(ClockFaceColor, 16,7-EQHeight[i],7);				
				ledMatrix->line_vert(ClockFaceColor, 17,7-EQHeight[i],7);
			}
			break;

			case 4:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 19,0,7);
			ledMatrix->line_vert(BLACK, 20,0,7);
			ledMatrix->line_vert(BLACK, 21,0,7);
			ledMatrix->line_vert(BLACK, 22,0,7);

			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(ClockFaceColor, 19,7-EQHeight[i],7);
					ledMatrix->line_vert(ClockFaceColor, 22,7-EQHeight[i],7);				
				}
				ledMatrix->line_vert(ClockFaceColor, 20,7-EQHeight[i],7);				
				ledMatrix->line_vert(ClockFaceColor, 21,7-EQHeight[i],7);
			}
			break;

			case 5:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 23,0,7);
			ledMatrix->line_vert(BLACK, 24,0,7);
			ledMatrix->line_vert(BLACK, 25,0,7);
			ledMatrix->line_vert(BLACK, 26,0,7);

			if(EQHeight[i]>0){
				if(width == 4)
				{
					ledMatrix->line_vert(ClockFaceColor, 23,7-EQHeight[i],7);
					ledMatrix->line_vert(ClockFaceColor, 26,7-EQHeight[i],7);
				}
				ledMatrix->line_vert(ClockFaceColor, 24,7-EQHeight[i],7);
				ledMatrix->line_vert(ClockFaceColor, 25,7-EQHeight[i],7);
			}
			break;

			case 6:
			//write the next frequency band
			//write over last loops data -- faster than clear();
			ledMatrix->line_vert(BLACK, 27,0,7);
			ledMatrix->line_vert(BLACK, 28,0,7);
			ledMatrix->line_vert(BLACK, 29,0,7);
			ledMatrix->line_vert(BLACK, 30,0,7);

			if(EQHeight[i]>0){
				if(width == 4)
				{	
					ledMatrix->line_vert(ClockFaceColor, 27,7-EQHeight[i],7);
					ledMatrix->line_vert(ClockFaceColor, 30,7-EQHeight[i],7);				

				}
				ledMatrix->line_vert(ClockFaceColor, 28,7-EQHeight[i],7);				
				ledMatrix->line_vert(ClockFaceColor, 29,7-EQHeight[i],7);
			}
			break;
		}
	}
	//now display the LED buffer
	ledMatrix->show();
}

/*
	This function writes an EQ band onto the displace
	Variables passed:
		height of the band (as driven by the amplitude)
		band number (0-7, as provided by the MSGEQ7)
		5 colors, for various intensities.... first colour is low intense, last is high.
		TODO: make this smaller, consider doing 0,2,4,8 in a loop... 

*/
void writeEQBand3(int height, int width, int band, Color color1,Color color2,Color color3,Color color4,Color color5)
{	
	//TODO: use width variable!! currently unused.
	
	int bandstartcol = band*4+2; //TODO optimise with bitwise x4
	// clear the band
	//TODO write a black box instead -- may be faster...
	
	ledMatrix->line_vert(BLACK, bandstartcol,0,7);
	ledMatrix->line_vert(BLACK, bandstartcol+1,0,7);
	ledMatrix->line_vert(BLACK, bandstartcol+2,0,7);
	ledMatrix->line_vert(BLACK, bandstartcol+3,0,7);
	
	switch(height)
	{
		case 0:
		//do nothing;
		break;
		
		case 1:
		//write 1 line in the middle
		ledMatrix->paintPixel(color1, bandstartcol, 4);
		ledMatrix->paintPixel(color1, bandstartcol+1, 4);
		ledMatrix->paintPixel(color1, bandstartcol+2, 4);
		ledMatrix->paintPixel(color1, bandstartcol+3, 4);
		break;
		
		case 2:
		//write 2 line in the middle
		ledMatrix->line_vert(color1, bandstartcol,3,4);
		ledMatrix->line_vert(color1, bandstartcol+1,3,4);
		ledMatrix->line_vert(color1, bandstartcol+2,3,4);
		ledMatrix->line_vert(color1, bandstartcol+3,3,4);

		ledMatrix->line_vert(color2, bandstartcol,3,3);
		ledMatrix->line_vert(color2, bandstartcol+1,3,3);
		ledMatrix->line_vert(color2, bandstartcol+2,3,3);
		ledMatrix->line_vert(color2, bandstartcol+3,3,3);		
		break;
		
		case 3:
		//write 4 line in the middle
		ledMatrix->line_vert(color1, bandstartcol,2,5);
		ledMatrix->line_vert(color1, bandstartcol+1,2,5);
		ledMatrix->line_vert(color1, bandstartcol+2,2,5);
		ledMatrix->line_vert(color1, bandstartcol+3,2,5);
		
		ledMatrix->line_vert(color2, bandstartcol,3,4);
		ledMatrix->line_vert(color2, bandstartcol+1,3,4);
		ledMatrix->line_vert(color2, bandstartcol+2,3,4);
		ledMatrix->line_vert(color2, bandstartcol+3,3,4);
		
		ledMatrix->line_vert(color3, bandstartcol,3,3);
		ledMatrix->line_vert(color3, bandstartcol+1,3,3);
		ledMatrix->line_vert(color3, bandstartcol+2,3,3);
		ledMatrix->line_vert(color3, bandstartcol+3,3,3);
		break;
		
		case 4:
		//write 6 line in the middle
		ledMatrix->line_vert(color1, bandstartcol,1,6);
		ledMatrix->line_vert(color1, bandstartcol+1,1,6);
		ledMatrix->line_vert(color1, bandstartcol+2,1,6);
		ledMatrix->line_vert(color1, bandstartcol+3,1,6);
		
		ledMatrix->line_vert(color2, bandstartcol,2,5);
		ledMatrix->line_vert(color2, bandstartcol+1,2,5);
		ledMatrix->line_vert(color2, bandstartcol+2,2,5);
		ledMatrix->line_vert(color2, bandstartcol+3,2,5);
		
		ledMatrix->line_vert(color3, bandstartcol,3,4);
		ledMatrix->line_vert(color3, bandstartcol+1,3,4);
		ledMatrix->line_vert(color3, bandstartcol+2,3,4);
		ledMatrix->line_vert(color3, bandstartcol+3,3,4);

		ledMatrix->line_vert(color4, bandstartcol,3,3);
		ledMatrix->line_vert(color4, bandstartcol+1,3,3);
		ledMatrix->line_vert(color4, bandstartcol+2,3,3);
		ledMatrix->line_vert(color4, bandstartcol+3,3,3);
		
		break;
		
		case 5:
		//write 8 line in the middle
		ledMatrix->line_vert(color1, bandstartcol,0,7);
		ledMatrix->line_vert(color1, bandstartcol+1,0,7);
		ledMatrix->line_vert(color1, bandstartcol+2,0,7);
		ledMatrix->line_vert(color1, bandstartcol+3,0,7);

		ledMatrix->line_vert(color2, bandstartcol,1,6);
		ledMatrix->line_vert(color2, bandstartcol+1,1,6);
		ledMatrix->line_vert(color2, bandstartcol+2,1,6);
		ledMatrix->line_vert(color2, bandstartcol+3,1,6);
		
		ledMatrix->line_vert(color3, bandstartcol,2,5);
		ledMatrix->line_vert(color3, bandstartcol+1,2,5);
		ledMatrix->line_vert(color3, bandstartcol+2,2,5);
		ledMatrix->line_vert(color3, bandstartcol+3,2,5);
		
		ledMatrix->line_vert(color4, bandstartcol,3,4);
		ledMatrix->line_vert(color4, bandstartcol+1,3,4);
		ledMatrix->line_vert(color4, bandstartcol+2,3,4);
		ledMatrix->line_vert(color4, bandstartcol+3,3,4);

		ledMatrix->line_vert(color5, bandstartcol,3,3);
		ledMatrix->line_vert(color5, bandstartcol+1,3,3);
		ledMatrix->line_vert(color5, bandstartcol+2,3,3);
		ledMatrix->line_vert(color5, bandstartcol+3,3,3);
		
		break;
	}
}



/*
	mode_Equaliser3()
	
	This function displays another equaliser mode
	No variables passed but does manipulate all EQ* variables
	Returns: nothing
*/
void mode_Equaliser3(int width)
{	
	#if defined DEBUG
		Serial.println("inside mode_Equaliser3");
	#endif

	//read in the bands
	for (int i = 0; i < 7; i++)
	{
		digitalWrite(EQStrobePin, LOW);
		EQSpectrumValue[i] = analogRead(EQAnalogPin);
		EQHeight[i] = map(EQSpectrumValue[i],EQLowerMapBound,EQUpperMapBound,0,5);
		digitalWrite(EQStrobePin, HIGH);
	}

	//now write out the LED array
	for (int i = 0; i < 7; i++)	
	{
		//5 colors, for various intensities.... first colour is low intense, last is high.
		writeEQBand3(EQHeight[i],width,i,RGB(0,0,7),RGB(0,0,15),RGB(12,7,6),RGB(15,15,7),RGB(15,15,15));
	}
	//now display the LED buffer
	ledMatrix->show();
}
