#include <Arduino.h>
#include <avr/pgmspace.h>
#include <Wire.h>  
#include <RTClib.h>
#include <SFRGBLEDMatrix.h> // Fabio Ornella's excellent lib+ firmware - see https://github.com/fornellas/RGB_Backpack_4096_Colors
#include <SPI.h>
#include <LEDClock.h>

extern RTC_DS1307 RTC; //reference RTC object

extern SFRGBLEDMatrix *ledMatrix; //reference ledMatrix array
/*
	This function works out what color the clock should be based on the time.
	Variables passed:
		int h -- hour - must be from 0 to 23 inclusive
		int m -- minute - must be from 0 to 59 inclusive
		
	Returns:
		 an RGB triplet
*/

RGBT getTimeColor(int h, int m)
{
	// define 3 RGB triplets...
	RGBT lower_hour_color, upper_hour_color, interpolated_color;
	
	//lookup the lower and upper RGB triplets from the static table
	memcpy_P(&lower_hour_color, &timecolors[h],sizeof(lower_hour_color));
	if(h==23) //handle the 23h00-00h00 roll-over
	{
		memcpy_P(&upper_hour_color, &timecolors[0],sizeof(upper_hour_color));
	} 
	else 
	{
		memcpy_P(&upper_hour_color, &timecolors[h+1],sizeof(upper_hour_color));
	}
	
	//how we have the RGB triplets for h and h+1 o'click do a linear interpolation
	//of their RGB values using the minute passed.
	interpolated_color.r = map(m,0,59,lower_hour_color.r,upper_hour_color.r);
	interpolated_color.g = map(m,0,59,lower_hour_color.g,upper_hour_color.g);
	interpolated_color.b = map(m,0,59,lower_hour_color.b,upper_hour_color.b);
	
	return interpolated_color;
}

/*
	This function sets up the MSGEQ7 chip.
	It sets the analog reference, configures the pins and prepares the chip
	for comms.
*/
void mode_Clock_init()
{
	//all SPI setup should be done outside this method. 
	ledMatrix->clear(); //clear the screen ready for the mode to kick in.

}

/*
	This function gets the time from RTC, parses it out into single chars
	suitable for displaying.
	
	It determines the colour of the clock by virtue of the time of day.
	
	It also prints out a pixel for each second of the minute passed - a second "worm".
*/
void mode_Clock()
{
	//This first bit gets the time and loads up the tens and units variables
	//for display on the LED Matrix.
	
	//TODO - MAYBE move this into a global variable;
	DateTime now = RTC.now();
	char h1;
	char h2;
	char m1;
	char m2;
	char s1;
	char s2;
	char letter = '1';
	
	int ClockFaceColor; 
	
	//grab the time
	unsigned int hour = now.hour();
	unsigned int minute = now.minute();
	unsigned int second = now.second();
	unsigned int week = now.dayOfWeek();
	
	
	// parse out single ASCII characters for hour and minute
	//TODO: clean this up -- doesn't need to use chars, can just use ints - ineffecient!
	if(hour<=9)
	{
		h1 = '0';
		h2 = '0' + hour % 10;
	}  else 
	{
		h1 = '0' + hour / 10;
		h2 = '0' + hour % 10;
	}
	
	
	if(minute<=9)
	{
		m1 = '0';
		m2 = '0' + minute % 10;
	}  else 
	{
		m1 = '0' + minute / 10;
		m2 = '0' + minute % 10;
	}	

	if(second<=9)
	{
		s1 = '0';
		s2 = '0' + second % 10;
	}  else 
	{
		s1 = '0' + second / 10;
		s2 = '0' + second % 10;
	}	

	RGBT time_col;
	time_col = getTimeColor(hour,minute);
	
	ClockFaceColor = RGB(time_col.r,time_col.g,time_col.b);

	//clear the screen first....
	ledMatrix->clear();
	
	//write out the hours. minutes and seconds.	
	ledMatrix->print(ClockFaceColor,2,1,5,h1);  //7
	ledMatrix->print(ClockFaceColor,6,1,5,h2); //+4
	ledMatrix->print(ClockFaceColor,12,1,5,m1); //+6
	ledMatrix->print(ClockFaceColor,16,1,5,m2); //+4
	ledMatrix->print(ClockFaceColor,22,1,5,s1); //+6 
	ledMatrix->print(ClockFaceColor,26,1,5,s2); //+4

		
	//write a dot between the hour and minute numbers
	//make it flash according the seconds..

	
	if (second & 1) //if the second is odd...
	{
	  // blank out the dots
		ledMatrix->paintPixel(BLACK, 10, 2);
		ledMatrix->paintPixel(BLACK, 10, 4);
		ledMatrix->paintPixel(BLACK, 20, 2);
		ledMatrix->paintPixel(BLACK, 20, 4);
	} else
	{
		//write the dots
		ledMatrix->paintPixel(ClockFaceColor, 10, 2);
		ledMatrix->paintPixel(ClockFaceColor, 10, 4);
		ledMatrix->paintPixel(ClockFaceColor, 20, 2);
		ledMatrix->paintPixel(ClockFaceColor, 20, 4);
	}
	
		
	//print the week day
	//ledMatrix->printweek(ClockFaceColor, 21, 1, week);
	
	/*
	//write the second "worm"
	if(second <= 30) //less than 30 write only the top half of the minute
	{
		int tencount = 0;
		
		for(int i = 1; i <= second; i++)
		{
			RGBMatrix.fillPixel((30 - i + 1) / 8, 0, (30 - i + 1) % 8, second_color);
		}
	} else //more than thirty -- write the entire top half and count out the bottom half
	{
		for(int i = 1; i <= second-30; i++)
		{
			for(int j = 1; j <= 30; j++)
			{
				RGBMatrix.fillPixel((30 - j + 1) / 8, 0, (30 - j + 1) % 8, second_color);
			}	
			RGBMatrix.fillPixel((30 - i + 1) / 8, 7, (30 - i + 1) % 8, second_color);
		}
	}
	*/
	//display the lot
	ledMatrix->show();
}

