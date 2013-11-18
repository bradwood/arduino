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

/* FYI
#define NEC 1
#define SONY 2
#define RC5 3
#define RC6 4
#define DISH 5
#define SHARP 6
#define PANASONIC 7
#define JVC 8
#define SANYO 9
#define MITSUBISHI 10
#define UNKNOWN -1
*/

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


//NOTE!! For all toggle-bit based protocols (e.g. RC5), ensure that the data field has the toggle bit alternating 
//within a particular Cmd.

//format {PROTOCOL,devname, data, nbits,len, buf[],hz,addr,repeat}

ir_transmission_t cmdOnOff[3] = {   		  //Turns everything on / off -- not actually everything just TV, AVR & Tivo
	{ SONY,   TV,     0xA90,      12, 0, {}, 0, 0, 0 }, // telly on/off
	{ NEC,    AVR,    0x4B36D32C, 32, 0, {}, 0, 0, 0 }, // AVR on/off 
	{ RC5,    TIVO,   0x28C,      12, 0, {}, 0, 0, 0 }  // TIVO on/off Note, toggle bit is MSB ()
};

ir_transmission_t cmdTerrestrialTV[5] = {     //Turns on BBC1 -- assumes everything necessary is On.
	{ RC5,    TIVO,   0x281,      12, 0, {}, 0, 0, 0 }, // TIVO 1
	{ RC5,    TIVO,   0x280,      12, 0, {}, 0, 0, 0 }, // TIVO 0
	{ RC5,    TIVO,   0x281,      12, 0, {}, 0, 0, 0 }  // TIVO 1
};

ir_transmission_t cmdKidsTV[5] = {     //Turns on Cartoonito -- assumes everything necessary is On.
	{ RC5,    TIVO,   0x287,      12, 0, {}, 0, 0, 0 }, // TIVO 7
	{ RC5,    TIVO,   0x280,      12, 0, {}, 0, 0, 0 }, // TIVO 0
	{ RC5,    TIVO,   0x286,      12, 0, {}, 0, 0, 0 }  // TIVO 6
};

ir_transmission_t cmdMusicTV[5] = {     //Turns on MTV Channel -- assumes everything necessary is On.
	{ RC5,    TIVO,   0x283,      12, 0, {}, 0, 0, 0 }, // TIVO 3
	{ RC5,    TIVO,   0x280,      12, 0, {}, 0, 0, 0 }, // TIVO 0
	{ RC5,    TIVO,   0x281,      12, 0, {}, 0, 0, 0 }  // TIVO 1
};

ir_transmission_t cmdDocTV[5] = {     //Turns on Documentary Channel -- assumes everything necessary is On.
	{ RC5,    TIVO,   0x282,      12, 0, {}, 0, 0, 0 }, // TIVO 2
	{ RC5,    TIVO,   0x281,      12, 0, {}, 0, 0, 0 }, // TIVO 1
	{ RC5,    TIVO,   0x280,      12, 0, {}, 0, 0, 0 }  // TIVO 0
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
	{3,3,3,3,3},
	{0,0,0,0,0},
	{0,0,0,0,0},
	{0,0,0,0,0}
};

#endif