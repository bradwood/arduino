/*
 * Universal Remote
 * By Bradley Wood
 * Q4, 2013
 */


#include <IRremote.h>
#include <UniversalRemoteHelper.h> // see this file for the button command IR definitions

#include <Wire.h>  // Wire.h library is required to use SX1509 lib
#include <sx1509_library.h>  // Include the SX1509 library

const byte LEDPin = 13;

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

//used for RC5 protocols which much toggle a bit on each new button pressed.
boolean togglestate = false;

void setup()
{
	Serial.begin(9600);

	pinMode(LEDPin,OUTPUT);

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
		digitalWrite(LEDPin,HIGH); //button pressed, so turn on LED


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

	digitalWrite(LEDPin,LOW); // turn off LED

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


//executes the command array corresponding the the command at row and col in the keyMap
void executeCommand(int row,int col) 
{
		
	for (int i = 0; i < keyMapSizes[row][col]; i++) //for each IR transmission in the the command
	{

		Serial.print("Command");
		Serial.print(i);
		Serial.print(" of ");
		Serial.println(keyMapSizes[row][col]);

		Serial.print("devtype =");
		Serial.println(keyMap[row][col][i].dev_type);

		Serial.print("data =");
		Serial.println(keyMap[row][col][i].data,HEX);

		Serial.print("nbits =");
		Serial.println(keyMap[row][col][i].nbits);	
		Serial.println(); //newline

		switch (keyMap[row][col][i].dev_type)
		{
		case NEC:
			irsend.sendNEC(keyMap[row][col][i].data, keyMap[row][col][i].nbits);
			delay(100);
		break;

		case SONY:
			for (int j = 0; j < 3; j++) //Sony needs 2 or 3 blasts.
			{
				irsend.sendSony(keyMap[row][col][i].data, keyMap[row][col][i].nbits);
				delay(40);
			}
		break;

		case RC5:
			togglestate = !togglestate; //flip whatever the last toggled state was.

			if (togglestate){  //if the resulting state is 1 then flip the 0 data toggle bit to 1
							   //requires that all RC5 data fields have the MSB=0
				unsigned long data = keyMap[row][col][i].data;
				data ^= 1 << 11;  //toggle the 12th bit ON-- LSB is bit 0
				
				for (int j = 0; j < 3; j++)  //RC5 needs 2 or 3 blasts.
				{
				 	irsend.sendRC5(data, keyMap[row][col][i].nbits); //and send
				 	delay(40);
				} 
			} 
			else
			{
				for (int j = 0; j < 3; j++) //RC5 needs 2 or 3 blasts.
				{
					irsend.sendRC5(keyMap[row][col][i].data, keyMap[row][col][i].nbits); //otherwise, send with togglebit = 0
					delay(40);
				}
			}
		break;

		case RC6:
			//not implemented
		break;

		case DISH:
			//not implemented
		break;

		case SHARP:
			//not implemented
		break;

		case JVC:
			//not implemented
		break;
	    
		case SANYO:
			//not implemented
		break;

		case MITSUBISHI:
			//not implemented
		break;

		case UNKNOWN:
			//digitalWrite(5, HIGH);
		break;

		//default:
		//not implemented
		} //end switch  
	
	delay(350);
	}

}
