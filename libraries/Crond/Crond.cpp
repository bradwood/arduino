/*
  Crond
  Created 2013-03-26 by Bradley Wood
*/
#include <Arduino.h>
#include <SoftwareSerial.h> //needed by the speakjet
#include <RTCLib.h>
#include <Crond.h>

//DEBUG code -- remove this compiler directive to turn of Serial debug msgs
#define DEBUG


//Reference external RTC
extern RTC_DS1307 RTC;

extern SoftwareSerial SJ;
extern boolean ranCronCommand;
extern unsigned int hour, minute, second, week; 

void sayAM(){
	delay(500);
	SJ.println("AY EM");
}

void sayPM()
{
	delay(500);
	SJ.println("PEEH EM");
}

void sayTime()
{
	digitalWrite(SPEAKER_POWER,HIGH); //turn on the speaker.
	switch(hour)
	{
		case 0:
			SJ.println("twelve");
			sayAM();
		break;

		case 1:
			SJ.println("one");
			sayAM();
		break;

		case 2:
			SJ.println("too");
			sayAM();
		break;

		case 3:
			SJ.println("three");
			sayAM();
		break;

		case 4:
			SJ.println("for");
			sayAM();
		break;

		case 5:
			SJ.println("five");
			sayAM();
		break;

		case 6:
			SJ.println("siks");
			sayAM();
		break;

		case 7:
			SJ.println("seh ven");
			sayAM();
		break;

		case 8:
			SJ.println("ayt");
			sayAM();
		break;

		case 9:
			SJ.println("nine");
			sayAM();
		break;

		case 10:
			SJ.println("ten");
			sayAM();
		break;
		
		case 11:
			SJ.println("eleven");
			delay(150);
			sayAM();
		break;
		
		case 12:
			SJ.println("twelve");
			sayPM();
		break;
		
		case 13:
			SJ.println("one");
			sayPM();
		break;
		
		case 14:
			SJ.println("too");
			sayPM();
		break;
		
		case 15:
			SJ.println("three");
			sayPM();
		break;
		
		case 16:
			SJ.println("for");
			sayPM();
		break;
		
		case 17:
			SJ.println("five");
			sayPM();
		break;
		
		case 18:
			SJ.println("siks");
			sayPM();
		break;
		
		case 19:
			SJ.println("seh ven");
			sayPM();
		break;
		
		case 20:
			SJ.println("ayt");
			sayPM();
		break;
		
		case 21:
			SJ.println("nine");
			sayPM();
		break;
		
		case 22:
			SJ.println("ten");
			sayPM();
		break;
		
		case 23:
			SJ.println("eleven");
			delay(150);
			sayPM();
		break;
	}
	delay(500);
	digitalWrite(SPEAKER_POWER,LOW); //turn off the speaker.
	
}
void sayBedTime(int childnum)
{
	digitalWrite(SPEAKER_POWER,HIGH); //turn on the speaker.
	SJ.println("BED time");
	delay(500);
	if (childnum == 1)
	{
		SJ.println("EETH EN");
		delay(800);
	} else
	{
		SJ.println("Joo Lee ET");
		delay(1000);
	}
	digitalWrite(SPEAKER_POWER,LOW); //turn off the speaker.
}

void matchCron(int cronjob)
//WARNING -- make sure cronds don't cash it time!!!
{
	switch(cronjob)
	{
		case 0: //job 0 -- say time on the hour
			if(minute == 0)
				sayTime();
		break;
		
		case 1: //job 1 -- goodnight juliet
			if((hour == 19) && (minute >= 30) && (minute < 45))
				sayBedTime(2);
		break;
		
		case 2: //job 2 -- goodnight ethan
			if((week == 4) && (hour == 20) && (minute >= 30) && (minute < 35))
				sayBedTime(1);
		break;
	}
}


void runCron()
{
	#if defined DEBUG
		Serial.println("in crond");
	#endif
	for (int j = 0; j < cronjobs ; j ++)
	{
		matchCron(j);	  
    }
}
