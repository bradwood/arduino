#include <Arduino.h>
#include <Accelerometer.h>
#include <i2c.h>  // not the wire library, can't use pull-ups //TODO: Don't follow this...

#define DEBUG

/* Initialize the MMA8452 registers 
   See the many application notes for more info on setting 
   all of these registers:
   http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=MMA8452Q
   
   Feel free to modify any values, these are settings that work well for me.
*/

void initMMA8452(byte fsr, byte dataRate)
{
	MMA8452Standby();  // Must be in standby to change registers

/* Set up the full scale range to 2, 4, or 8g. */
	if ((fsr==2)||(fsr==4)||(fsr==8))
		writeRegister(XYZ_DATA_CFG_REG, fsr >> 2);  
	else
		writeRegister(XYZ_DATA_CFG_REG, 0);
/* Setup the 3 data rate bits, from 0 to 7 */
	writeRegister(CTRL_REG1, readRegister(CTRL_REG1) & ~(0x38));
	if (dataRate <= 7)
		writeRegister(CTRL_REG1, readRegister(CTRL_REG1) | (dataRate << 3));  
/* Set up portrait/landscap registers */
	writeRegister(PL_CFG_REG, 0x40);  // Enable P/L
	writeRegister(PL_BF_ZCOMP_REG, 0x14);  // 29deg z-lock, 
	writeRegister(PL_P_L_THS_REG, 0x84);  // 45deg thresh, 14deg hyst
	writeRegister(PL_COUNT_REG, 0x05);  // debounce counter at 100ms 
/* Set up single and double tap */
	//writeRegister(PULSE_CFG_REG, 0x7F);  // enable single/double taps on all axes
	writeRegister(PULSE_CFG_REG, 0x60);  // enable latch and double taps on z axis only
	//writeRegister(PULSE_THSX_REG, 0x20);  // x thresh at 2g
	//writeRegister(PULSE_THSY_REG, 0x20);  // y thresh at 2g
	//writeRegister(PULSE_THSZ_REG, 0x8);  // z thresh at .5g
	writeRegister(PULSE_THSZ_REG, 0x20);  // z thresh at 2g
	writeRegister(PULSE_TMLT_REG, 0x30);  // 60ms time limit, the min/max here is very dependent on output data rate
	writeRegister(PULSE_LTCY_REG, 0x28);  // 200ms between taps min
	writeRegister(PULSE_WIND_REG, 0xFF);  // 1.275s (max value) between taps max
/* Set up interrupt 1 and 2 */
	writeRegister(CTRL_REG3, 0x02);  // Active high, push-pull
	writeRegister(CTRL_REG4, 0x19);  // DRDY int enabled, P/L enabled
	writeRegister(CTRL_REG5, 0x01);  // DRDY on INT1, everything else on INT2

/* Set the wake oversampling mode to Low Noise, Low Power*/	
	//clear the mode bits in CTRL_REG2
	writeRegister(CTRL_REG2, (readRegister(CTRL_REG2) & ~MODS_MASK));
//set the MODS bnits to 01 for LNLP. 
	writeRegister(CTRL_REG2, (readRegister(CTRL_REG2) | MODS0_MASK));

///-----motion detection config--------

//set configuration register for motion detection.
//this turns on latching and motion, on the x, y and z axes
	writeRegister(FF_MT_CFG_1_REG, (ELE_MASK & OAE_MASK & XEFE_MASK & YEFE_MASK & ZEFE_MASK));
	
//set theshold acceleration value for motion detection.
//7 bit number * 0.063g
	writeRegister(FT_MT_THS_1_REG, 0x04); // = 0.063g * 4

//set debounce counter for motion detection.
// depends on ODR setting
	writeRegister(FF_MT_COUNT_1_REG, 0x02); // 2 counts = 160ms while AWAKE, 320ms while aSLEEP

///-----autowake/sleep config--------

/* set Autowake/Autosleep bit (bit 2 in register 0x2B)	*/
	writeRegister(CTRL_REG2, (readRegister(CTRL_REG2) | SLPE_MASK));

/* set sleep sample rate - see page 8 of AN4074	
   this forces the lowest possible sleep sample rate, namely 1.56Hz*/	
	writeRegister(CTRL_REG1, (readRegister(CTRL_REG1) | B11000000)); //0xC0 ASLP_RATE_MASK

/* set the sleep oversampling mode. 
   this sets it to Low Power Mode*/
	writeRegister(CTRL_REG2, (readRegister(CTRL_REG2) | B00011000));

/* set the event types that will trigger a wake event
   this enables all functions to trigger a wake event
*/
	//writeRegister(CTRL_REG4, (readRegister(CTRL_REG4) | B11111100));
	writeRegister(CTRL_REG4, (readRegister(CTRL_REG4) | B11111100));

/* set the event types that will trigger a wake interrupt
   this enables all functions to trigger a wake interrupt
*/
	writeRegister(CTRL_REG3, (readRegister(CTRL_REG3) | B01111000));

/*  set sleep counter timeout*/
	writeRegister(ASLP_COUNT_REG, 600); //roughly 200 seconds at 6.25Hz

	MMA8452Active();  // Set to active to start reading
}

/* calibrates the accelerometer for tilt/linear acceleration detection by
removing the effect of gravity from the readings.

See Freescale note AN4069

Does not do any noise filtering.

*/

void CalibrateAccel()
{
	//NOTE: Only works for 2g mode!!!
	
	//clear calibration registers
	//put into standby mode to load the offsets into the offset registers
	MMA8452Standby();
	delay(400);
	writeRegister(OFF_X_REG,0x00);
	writeRegister(OFF_Y_REG,0x00);
	writeRegister(OFF_Z_REG,0x00);
	MMA8452Active();
	delay(400);	
	
	byte regdata[6];  // x/y/z accel register data store here
	int signeddata[3];  // Stores the 12-bit signed value
	signed int offx, offy, offz = 0;
	/* If int1 goes high, all data registers have new data */
	if (digitalRead(AccelInt1Pin))  // Interrupt pin, should probably attach to interrupt function
	{
		readRegisters(0x01, 6, &regdata[0]);  // Read the six data registers into data array

	/* For loop to calculate 12-bit 2's compliment values for each axis */
		for (int i=0; i<6; i+=2)
		{
			signeddata[i/2] = ((regdata[i] << 8) | regdata[i+1]) >> 4;  // Turn the MSB and LSB into a 12-bit value
			if (regdata[i] > 0x7F)
			{  // If the number is negative, we have to make it so manually (no 12-bit data type)
				signeddata[i/2] = ~signeddata[i/2] + 1;
				signeddata[i/2] *= -1;  // Transform into negative 2's complement #
			}
		}		
	}
	
	#if defined DEBUG
	Serial.println("Before calibration");
		for (int i=0; i<3; i++)
		{
			Serial.print(signeddata[i], DEC);  // Print adc count values, feel free to uncomment this line
			Serial.print("\t");
		}
		Serial.println();
	#endif
	
	// now we calculate the offset values for each axis.
	// take each value, divide by 2 (for 2g mode) and calculate inverse.
	offx = signeddata[0]/2 * -1;
	offy = signeddata[1]/2 * -1;
	offz = (1024 - signeddata[2]/2) * -1;	

	#if defined DEBUG
	Serial.println("Offsets");
	
	Serial.print(offx, DEC);
	Serial.print("\t");
	Serial.print(offy, DEC);
	Serial.print("\t");
	Serial.print(offz, DEC);
	Serial.println("\t");
	
	Serial.println("2* Offsets + Pre-calib");
	
	Serial.print(2*offx + signeddata[0], DEC);
	Serial.print("\t");
	Serial.print(2*offy + signeddata[1], DEC);
	Serial.print("\t");
	Serial.print(2*offz + signeddata[2], DEC);
	Serial.println("\t");
	
	#endif

	//put into standby mode to load the offsets into the offset registers
	MMA8452Standby();
	delay(400);
	
	writeRegister(OFF_X_REG,(byte)offx);
	writeRegister(OFF_Y_REG,(byte)offy);
	writeRegister(OFF_Z_REG,(byte)offz);
	MMA8452Active();	
	delay(400);

	#if defined DEBUG	
		if (digitalRead(AccelInt1Pin))  // Interrupt pin, should probably attach to interrupt function
		{
			readRegisters(0x01, 6, &regdata[0]);  // Read the six data registers into data array

		/* For loop to calculate 12-bit 2's compliment values for each axis */
			for (int i=0; i<6; i+=2)
			{
				signeddata[i/2] = ((regdata[i] << 8) | regdata[i+1]) >> 4;  // Turn the MSB and LSB into a 12-bit value
				if (regdata[i] > 0x7F)
				{  // If the number is negative, we have to make it so manually (no 12-bit data type)
					signeddata[i/2] = ~signeddata[i/2] + 1;
					signeddata[i/2] *= -1;  // Transform into negative 2's complement #
				}
			}		
		}
	
		Serial.println("After calibration");
		for (int i=0; i<3; i++)
		{
			Serial.print(signeddata[i], DEC);  // Print adc count values, feel free to uncomment this line
			Serial.print("\t");
		}
		Serial.println();
	
	#endif
}


/* Sets the MMA8452 to standby mode.
   It must be in standby to change most register settings */
void MMA8452Standby()
{
	byte c = readRegister(CTRL_REG1);
	writeRegister(CTRL_REG1, c & ~(0x01));
}

/* Sets the MMA8452 to active mode.
   Needs to be in this mode to output data */
void MMA8452Active()
{
	byte c = readRegister(CTRL_REG1);
	writeRegister(CTRL_REG1, c | 0x01);
}

/* Read in registers sequentially, starting at address 
   into the dest byte arra */
void readRegisters(byte address, int i, byte * dest)
{
	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte((MMA8452_ADDRESS<<1));	// write 0xB4
	i2cWaitForComplete();

	i2cSendByte(address);	// write register address
	i2cWaitForComplete();

	i2cSendStart();
	i2cSendByte((MMA8452_ADDRESS<<1)|0x01);	// write 0xB5
	i2cWaitForComplete();
	for (int j=0; j<i; j++)
	{
		i2cReceiveByte(TRUE);
		i2cWaitForComplete();
		dest[j] = i2cGetReceivedByte();	// Get MSB result
	}
	i2cWaitForComplete();
	i2cSendStop();

	cbi(TWCR, TWEN);	// Disable TWI
	sbi(TWCR, TWEN);	// Enable TWI
}

/* read a single byte from address and return it as a byte */
byte readRegister(uint8_t address)
{
	byte data;

	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte((MMA8452_ADDRESS<<1));	// write 0xB4
	i2cWaitForComplete();

	i2cSendByte(address);	// write register address
	i2cWaitForComplete();

	i2cSendStart();

	i2cSendByte((MMA8452_ADDRESS<<1)|0x01);	// write 0xB5
	i2cWaitForComplete();
	i2cReceiveByte(TRUE);
	i2cWaitForComplete();

	data = i2cGetReceivedByte();	// Get MSB result
	i2cWaitForComplete();
	i2cSendStop();

	cbi(TWCR, TWEN);	// Disable TWI
	sbi(TWCR, TWEN);	// Enable TWI

	return data;
}

/* Writes a single byte (data) into address */
void writeRegister(unsigned char address, unsigned char data)
{
	i2cSendStart();
	i2cWaitForComplete();

	i2cSendByte((MMA8452_ADDRESS<<1));// write 0xB4
	i2cWaitForComplete();

	i2cSendByte(address);	// write register address
	i2cWaitForComplete();

	i2cSendByte(data);
	i2cWaitForComplete();

	i2cSendStop();
}