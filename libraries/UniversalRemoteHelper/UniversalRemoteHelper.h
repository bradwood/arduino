#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif
#include <avr/pgmspace.h>  //will store all commands in PROGMEM

#include <IRremote.h> // will used datatypes from this file.

#ifndef _UNIVERSALREMOTEHELPER_H
#define _UNIVERSALREMOTEHELPER_H

typedef struct {
  int dev_type;                       /* see IRremote.h for types NEC, SONY, SHARP, etc */
  char device_name[4];                /* name of specific IR-controlled device */
  char dev_button[4];                 /* name of device's remote button */
  unsigned long data;                 /* data to be transmitted - used by IRsend obj */
  int nbits;                          /* no of bits - used by IRsend obj */
  int len;                            /* length - use for Raw sending - used by IRsend obj */
  int buf[];                          /* buffer - use for Raw sending - used by IRsend obj */
  int hz;                             /* freq - use for Raw sending - used by IRsend obj */
  int address;                         /* for Panasonic only -  used by IRsend obj */
  int repeat;                         /* for JVC only - used by IRsend obj */
} ir_transmission_t;

typedef struct {
    ir_transmission_t ir_transmissions[]; // array of transmissions that make up the command.
} command_t;

#endif