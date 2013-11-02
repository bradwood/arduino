/*
 * LEDEqualiser.h
 * Header file for the Equaliser mode of the LEDMachine
 */

#ifndef LEDEqualiser_h
#define LEDEqualiser_h
#endif

#include <Arduino.h>

//Reference constants for MSGEQ7 input
extern const int EQAnalogPin; // read from multiplexer using analog input 0
extern const int EQStrobePin; // strobe is attached to digital pin 2
extern const int EQResetPin; // reset is attached to digital pin 3

// Reference the lower threshold for map() to reduce noice when "silent" 
extern const int EQLowerMapBound; 
extern const int EQUpperMapBound; 

// to hold a2d values from MSGEQ7 -- defined in the main ino file.
extern int EQSpectrumValue[7]; 
// and an array used to map() the above.
extern int EQHeight[7]; 

/*
	This function sets up the MSGEQ7 chip.
	It sets the analog reference, configures the pins and prepares the chip
	for comms.
*/
void mode_Equaliser_init(); //initialisation routine - not constructor
/*
	Thess functions display various equaliser modes
	No variables passed but does manipulate all EQ* variables
	Returns: nothing
*/

void mode_Equaliser1(int width); 
void mode_Equaliser2(int width); 
void mode_Equaliser3(int width); 

