/*
 * Universal Remote
 * By Bradley Wood
 * Q4, 2013
 */


#include <IRremote.h>
#include <UniversalRemoteHelper.h> // see this file for the button command IR definitions

#include <Wire.h>  // Wire.h library is required to use SX1509 lib
#include <sx1509_library.h>  // Include the SX1509 library

const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address (00)

// SX1509 pin definitions
const byte sx1509resetPin = 9;
const byte sx1509interruptPin = 2;

byte sx1509sleepTime = 7;
byte sx1509scanTime = 2;  // Scan time per row
byte sx1509debounceTime = 1;   // The debounce config value


// Create a new sx1509Class object
sx1509Class sx1509(SX1509_ADDRESS, sx1509resetPin, sx1509interruptPin);

//instantiate IR transmission infrastructure
IRsend irsend;

void setup()
{
	Serial.begin(9600);

	// Must first initialize the sx1509:
	sx1509.init();
	// In order to use the keypad, the clock must first be
	// configured. We can call configureClock() with the default
	// parameters (2MHz internal oscillator, no clock in/out).
	sx1509.configClock();
	// Next call the keypad function with the number of rows
	//  and columns.
	sx1509.keypad(numRows, numCols, sx1509sleepTime, sx1509scanTime);  // Advanced keypad init
	sx1509.debounceConfig(sx1509debounceTime);

}




void loop() {
	if (Serial.read() != -1) {
		for (int i = 0; i < 3; i++) {
			irsend.sendSony(keyMap[0][0][0].data,keyMap[0][0][0].nbits );
			delay(40);
		}
	}
}
