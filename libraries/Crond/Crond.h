/*
 * Crond.h
 * Header file for the Equaliser mode of the LEDMachine
 */

#ifndef Crond_h
#define Crond_h
#endif
// DO NOT CHANGE THE BELOW  3 lines WITHOUT ALSO CHANGING THE main ino
#define SPEAKER_POWER	A1 //set high to turn on speaker, low to turn off....

#include <Arduino.h>

//no of cronjobs 
const int cronjobs = 3;

//edit function to set crontimes.
void runCron();
