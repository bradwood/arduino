#include "optiLoader.h"

image_t PROGMEM image_328 = {
    {"3v3_8MHz_atmega328.hex"},
    {"atmega328"},
    0x950F,				/* Signature bytes for 328P */
//    0x9514,				/* Signature bytes for 328P PU */
    {0x3F, 0xFF, 0xDA, 0x05},            // pre program fuses (prot/lock, low, high, ext)
    {0x0F, 0x0, 0x0, 0x0},            // post program fuses -- the 0x0's tell the script not to load, rather than to load 0's
    //{0x0F, 0xFF, 0xDA, 0x05},            // post program fuses
    {0x3F, 0xFF, 0xFF, 0x07},           // fuse mask
    32768,     // size of chip flash in bytes
    //8192,     // hacked chipsize -- see http://forums.adafruit.com/viewtopic.php?f=19&t=25966
    128,   // size in bytes of flash page
    {
   	":103E0000112484B714BE81FFFCD085E0809381002B\n"
	":103E100082E08093C00088E18093C10086E08093B7\n"
	":103E2000C20088E08093C4008EE0D5D0259A26E0B9\n"
	":103E300088E19EEF31E0909385008093840036BB4B\n"
	":103E4000B09BFECF1D9AA8952150A9F700E010E085\n"
	":103E5000EE24E394F5E0DF2EA1E1CA2EB3E0FB2EC1\n"
	":103E6000AED0813489F4ABD08983BBD089818238CC\n"
	":103E700019F484E09DD099C0813819F484E098D079\n"
	":103E800094C083E095D091C0823419F484E1B1D01C\n"
	":103E90008CC0853419F485E0ACD087C0853571F4C9\n"
	":103EA0008ED0A82EBB248BD0082F10E0102F002717\n"
	":103EB0000A291B29000F111F94D077C0863529F4D9\n"
	":103EC00084E097D080E074D070C0843609F043C09D\n"
	":103ED00076D075D0B82E73D088E30030180738F448\n"
	":103EE000F801F7BEE895812C61E0962E03C0812C85\n"
	":103EF00051E0952E64D0F40181934F01BE16D1F7A5\n"
	":103F0000F8E300301F0718F0F801F7BEE89569D014\n"
	":103F100007B600FCFDCFF801A0E0B1E02C9130E045\n"
	":103F200011968C91119790E0982F8827822B932BD4\n"
	":103F300012960C01E7BEE8951124329681E0A03874\n"
	":103F4000B80761F7F801D7BEE89507B600FCFDCFCA\n"
	":103F5000C7BEE8952AC08437B9F431D030D0A82E36\n"
	":103F60002ED03FD0BA2CF80101C0F4014F010894C3\n"
	":103F7000811C911C84911CD0BA94B9F70F5F1F4F1C\n"
	":103F8000AA940A0D111D11C0853741F42AD08EE183\n"
	":103F90000FD084E90DD086E00BD007C0813521F425\n"
	":103FA00088E019D01ED001C01CD080E101D058CFCC\n"
	":103FB0009091C00095FFFCCF8093C600089580913A\n"
	":103FC000C00087FFFCCF8091C00084FD01C0A89590\n"
	":103FD0008091C6000895E0E6F0E098E19083808348\n"
	":103FE0000895EDDF803219F088E0F5DFFFCF84E13E\n"
	":103FF000DFCFCF93C82FE3DFC150E9F7F2DFCF91D6\n"
	":0C400000089580E0E8DFEE27FF27099418\n"
	":023FFE000404B9\n"
	":0400000300003E00BB\n"
	":00000001FF\n"
    }
};


/*
 * Table of defined images
 */
image_t *images[] = {
  &image_328,
};

uint8_t NUMIMAGES = sizeof(images)/sizeof(images[0]);
