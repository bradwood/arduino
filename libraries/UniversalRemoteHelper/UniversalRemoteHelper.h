#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include <avr/pgmspace.h>  //will store all commands in PROGMEM

#include <IRremote.h> // will used datatypes from this file.

#ifndef _UNIVERSALREMOTEHELPER_H
#define _UNIVERSALREMOTEHELPER_H

// Values for dev_name
#define TV 1
#define VCR 2
#define AVRC 3
#define APPLETV 4
#define TIVO 5

typedef struct
{
	int dev_type;                       /* see IRremote.h for types NEC, SONY, SHARP, etc */
	int dev_name;          		        /* name of specific IR-controlled device */
	unsigned long data;                 /* data to be transmitted - used by IRsend obj */
	int nbits;                          /* no of bits - used by IRsend obj */
	int len;                            /* length - use for Raw sending - used by IRsend obj */
	int buf[1];      //TODO: ?????      /* buffer - use for Raw sending - used by IRsend obj */
	int hz;                             /* freq - use for Raw sending - used by IRsend obj */
	int address;                        /* for Panasonic only -  used by IRsend obj */
	int repeat;                         /* for JVC only - used by IRsend obj */
} ir_transmission_t;


//format {PROTOCOL,devname, data, nbits,len, buf[],hz,addr,repeat}
ir_transmission_t cmdOnOff[3] = {   		  //Turns everything on / off -- not actually everything just TV, AVR & Tivo
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // telly on/off
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // AVR on/off TODO:
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }  // TIVO on/off TODO:
};

ir_transmission_t cmdTerrestrialTV[5] = {     //Turns on BBC1 -- assumes everything necessary is On.
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // set AVR to CBL/SAT
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 1 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 0 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 1 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }  // Type ENTER  on TIVO
};

ir_transmission_t cmdKidsTV[5] = {     //Turns on Cartoonito -- assumes everything necessary is On.
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // set AVR to CBL/SAT
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 7 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 0 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 2 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }  // Type ENTER  on TIVO
};

ir_transmission_t cmdMusicTV[5] = {     //Turns on MTV Channel -- assumes everything necessary is On.
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // set AVR to CBL/SAT
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 7 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 0 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 2 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }  // Type ENTER  on TIVO
};

ir_transmission_t cmdDocTV[5] = {     //Turns on Documentary Channel -- assumes everything necessary is On.
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // set AVR to CBL/SAT
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 7 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 0 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }, // Type 2 on TIVO
	{ SONY, TV, 0xA90, 12, 0, {}, 0, 0, 0 }  // Type ENTER  on TIVO
};

ir_transmission_t nop[1] = { //Do Nothing
	{ SONY, TV, 0x000, 00, 0, {}, 0, 0, 0 } // do nothing TODO: check this. 
};

//dimensions of keypad
const byte numRows = 4;
const byte numCols = 5;

//this 2 d arrays maps each ir_transmission to the 2D button grid
ir_transmission_t *keyMap[numRows][numCols] = 
{
	{cmdOnOff, 			cmdTerrestrialTV, 	cmdKidsTV, 		cmdMusicTV,		cmdDocTV},
	{nop,				nop,				nop, 		    nop, 		    nop},
	{nop,				nop,				nop,			nop,			nop},
	{nop,				nop,				nop,			nop,			nop}
};

//this 2D array holds the number of IR transmissions per command 
//TODO: There must be a better way to do this, but too much messing with sizeof yet no joy...
// vector type not supported in arduino :( 
int keyMapSizes[numRows][numCols] = 
{
	{3,5,5,5,5},
	{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0}
};



#endif