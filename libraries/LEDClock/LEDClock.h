/*
 * LEDClock.h
 * Header file for the Equaliser mode of the LEDMachine
 */

#ifndef LEDClock_h
#define LEDClock_h
#endif

#include <Arduino.h>
#include <avr/pgmspace.h>


//type definition for defining RGB color codes for each hour of time.
typedef struct RGBT {
	int r;
	int g;
	int b;
};

//colors for the clock for each hour: 0-23....
PROGMEM static RGBT timecolors[] = 
{
	{15,0,0}, //0  red
	{13,1,0}, //1  
	{11,3,0}, //2  
	{9,5,0}, //3  
	{7,7,0}, //4  dark yellow
	{5,9,0}, //5  
	{3,11,0}, //6  
	{1,13,0}, //7  
	{0,15,0}, //8  green
	{0,13,1}, //9  
	{0,11,3}, //10  
	{0,9,5}, //11  
	{0,7,7}, //12  turquoise
	{0,5,9}, //13  
	{0,3,11}, //14  
	{0,1,13}, //15  
	{0,0,15}, //16  blue
	{1,0,13}, //17  
	{3,0,11}, //18  
	{5,0,9}, //19  
	{7,0,7}, //20  purple
	{9,0,5}, //21  
	{11,0,3}, //22  
	{13,0,1}, //23
};


/*
	This function returns an RGB triplet given a time.*/
RGBT getTimeColor(int h, int m);

/*
	This function sets up the necessary stuff for the LED clock
*/
void mode_Clock_init(); //initialisation routine - not constructor

/*
	This function displays the Clock
	Returns: nothing
*/
void mode_Clock(); //function prototype

