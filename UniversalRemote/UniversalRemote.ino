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

//////////////////////////////////
//// Loop Variables  /////////////
//////////////////////////////////
unsigned int keyData;  // The raw data from the key press register
unsigned int previousKeyData = 0;  // previously read raw data
byte activeRow;  // The row of the button being pressed
byte activeColumn;  // The column of the button being pressed
// These variables are used to emulate a key-hold. While the key
//  is held down, there's a long delay before the second character
//  is printed. Then a shorter delay between the remaining key presses
unsigned int holdCount = 0;
// This behavior is highly dependent on scanTime and debounceConfig
//  which are set in the setup.
const byte holdCountMax = 25;

// These releaseCount variables 
//  The keypad engin on the SX1509 doesn't generate an interrupt
//  when a key is relased. So we'll use this counter to generate
//  releases.
unsigned int releaseCount = 0;  // Our counter
unsigned int releaseCountMax = 100;  // Top, in about milliseconds

// The loop will poll the interrupt pin. If the pin
//  is pulled low by the SX1509, we'll read the keypad data and
//  sort it into row and column, and send the corresponding key
//  press out to the computer.

void loop()
{
	// The interrupt is active low, and pulled-up otherwise.
	// The interrupt will be activated whenever a key press is read
	if (!digitalRead(sx1509interruptPin))
	{
		// readKeyData() returns a 16-bit word of data. The lower 8-bits 
		//  represent each of the up-to 8 rows. The upper 8-bits
		//  correspond to the columns. A 1 in a bit position means
		//  that a button in that row or column is being pressed.
		keyData = sx1509.readKeyData();

		// Next, we'll sort out which row and column are being pressed.
		activeRow = keyData & 0xFF;  // The row is the lower 8-bits
		activeColumn = keyData >> 8;  // column is the upper 8-bits
		// The getBitPosition function will return which bit is our 1
		activeRow = getBitPosition(activeRow);
		activeColumn = getBitPosition(activeColumn);

		// If it's a new button press spit it out, reset hold delay
		if (keyData != previousKeyData)
		{
			holdCount = 0;

			Serial.print(activeRow);
			Serial.print(" ");
			Serial.println(activeColumn);
			executeCommand(activeRow,activeColumn);

		}
		else
		{
			holdCount++;  // Increment holdCount
			// This works as something of a key-press delay. Hold
			//  down a key on your computer to see what I'm talking
			//  about. After the initial delay, all characters following
			//  will stream out quickly.
			if (holdCount > holdCountMax)
			{
				Serial.print(activeRow);
				Serial.print(" ");
				Serial.println(activeColumn);
				executeCommand(activeRow,activeColumn);
			}
		}
		// Reset release count since there's been a key-press
		releaseCount = 0;
		// Set keyData as previousKeyData
		previousKeyData = keyData;
	}

	// If no keys have been pressed we'll continuously increment
	//  releaseCount. Eventually creating a release, once the count
	//  hits the max.
	
	releaseCount++;
	if (releaseCount == releaseCountMax)
	{
		releaseCount = 0;
		previousKeyData = 0;
	}
	delay(1);  // This gives releaseCountMax a more intuitive unit

	/* original IR transmission loop -- remember SONY requires 3 sends...
	if (Serial.read() != -1) {
		for (int i = 0; i < 3; i++) {
			irsend.sendSony(keyMap[0][0][0].data,keyMap[0][0][0].nbits );
			delay(40);
		}
	}
	*/
}  // end loop

// This function scours a byte and returns the position of the 
//  first byte it sees a 1. Great if our data bytes only have
//  a single 1 in them! Should return 0-7 if it sees a 1, 255 otherwise
byte getBitPosition(byte dataByte)
{
	for (int i=0; i<8; i++)
	{
		if (dataByte & (1<<i))
		{
			return i;
		}
	}
	return 255;  // Otherwise return an error
}



void executeCommand(int row,int col) //executes the command array corresponding the the command at row and col in the keyMap
{
	unsigned int arraySize = sizeof( *keyMap[row][col] ) / sizeof( keyMap[row][col][0] ); //get no of items in array of cmds
	
	Serial.print("No of items in array ");
	Serial.println(arraySize);
	
	Serial.print("Size of cmd array");
	Serial.println(sizeof( *keyMap[row][col] ));

	Serial.print("Size of 1st item in cmd array");
	Serial.println(sizeof( keyMap[row][col][0] ));

	Serial.print("1st item in cmd array devname");
	Serial.println(keyMap[row][col][2].dev_name );
	Serial.print("2nd item in cmd array devname");
	Serial.println(keyMap[row][col][3].dev_name );


	switch (row)
	{
	case 'a':    
		//digitalWrite(2, HIGH);
	break;
	case 'b':    
		//digitalWrite(3, HIGH);
	break;

	case 'c':    
		//digitalWrite(4, HIGH);
	break;

	case 'd':    
		//digitalWrite(5, HIGH);
	break;

	case 'e':    
		//digitalWrite(6, HIGH);
	break;
    
	//default:
	// turn all the LEDs off:
	} //end switch  

}
