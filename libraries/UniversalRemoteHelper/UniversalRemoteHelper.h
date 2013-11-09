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

typedef struct {
  int dev_type;                       /* see IRremote.h for types NEC, SONY, SHARP, etc */
  int dev_name;                /* name of specific IR-controlled device */
  unsigned long data;                 /* data to be transmitted - used by IRsend obj */
  int nbits;                          /* no of bits - used by IRsend obj */
  int len;                            /* length - use for Raw sending - used by IRsend obj */
  int buf[1];                          /* buffer - use for Raw sending - used by IRsend obj */
  int hz;                             /* freq - use for Raw sending - used by IRsend obj */
  int address;                         /* for Panasonic only -  used by IRsend obj */
  int repeat;                         /* for JVC only - used by IRsend obj */
} ir_transmission_t;


//format {{PROTOCOL},{"devname"},{"buttname"}, data, nbits,len, buf[],hz,addr,repeat}
ir_transmission_t cmdOnOff[1] = { 
  { SONY, TV, 0xA90, 12, 0, {0}, 0, 0, 0 } 
};


#endif