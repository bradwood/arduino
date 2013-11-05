/*
 * Universal Remote
 * By Bradley Wood
 * Q4, 2013
 */


#include <IRremote.h>
#include <UniversalRemoteHelper.h>


//format {{PROTOCOL},{"devname"},{"buttname"}, data, nbits,len, buf[],hz,addr,repeat}
ir_transmission_t cmd_off[1] = { 
	{ SONY, TV, 0xA90, 12, 0, {0}, 0, 0, 0 } 
};

IRsend irsend;

void setup()
{
  Serial.begin(9600);
}

void loop() {
  if (Serial.read() != -1) {
    for (int i = 0; i < 3; i++) {
      irsend.sendSony(cmd_off[0].data, cmd_off[0].nbits); // Sony TV power code
      delay(40);
    }
  }
}
