#include "UniversalRemoteHelper.h"


command_t PROGMEM cmd_off = {
	{ {NEC},{"telly"},{"off"}, 0xff, 1, 2, 3, 4, 5,6 } // telly off
	{ {NEC},{"AVR"},{"off"}, 0xff, 1, 2, 3, 4, 5,6 } // AVReceiver off
}

command_t PROGMEM cmd_on = {
	{ {NEC},{"telly"},{"on"}, 0xff, 1, 2, 3, 4, 5,6 } // telly on
	{ {NEC},{"AVR"},{"on"}, 0xff, 1, 2, 3, 4, 5,6 } // AVReceiver on
}

