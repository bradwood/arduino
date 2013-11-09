/*
 * Universal Remote
 * By Bradley Wood
 * Q4, 2013
 */


#include <IRremote.h>
#include <UniversalRemoteHelper.h>

IRsend irsend;

void setup()
{
  Serial.begin(9600);
}

void loop() {
  if (Serial.read() != -1) {
    for (int i = 0; i < 3; i++) {
      irsend.sendSony(cmdOnOff[0].data, cmdOnOff[0].nbits); // Sony TV power code
      delay(40);
    }
  }
}
