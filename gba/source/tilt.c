#include "tilt.h"

#define TILT_SCAN1 *((vu8*)(0xE008000))
#define TILT_SCAN2 *((vu8*)(0xE008100))
#define TILT_DATA1 *((vu8*)(0xE008200))
#define TILT_DATA2 *((vu8*)(0xE008300))
#define TILT_DATA3 *((vu8*)(0xE008400))
#define TILT_DATA4 *((vu8*)(0xE008500))

void startTiltScan() {
	TILT_SCAN1=0x55;
	TILT_SCAN2=0xAA;
}

struct tiltData readTilt() {
	struct tiltData out;
	
	while((TILT_DATA2 & 0x80)==0);
	
	out.x=TILT_DATA1 | ((TILT_DATA2&0x0F)<<8);
	out.y=TILT_DATA3 | ((TILT_DATA4&0x0F)<<8);

	return out;
}