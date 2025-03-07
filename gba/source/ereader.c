#include "ereaderInternal.h"

#include <string.h>

void ereaderInitCam1();
void ereaderInitCam2();

void ereaderPowerOn() {
	EREAD_RESET |=  0x02;
	//wait 10 ms
	EREAD_CTRL0  =  0x0040;
	EREAD_RESET &= ~0x0002;
	EREAD_CTRL0  =  0x0020;
	//wait 40 ms
	EREAD_CTRL1 &= ~0x0010;
	EREAD_CTRL0  =  0x0067;
}

void ereaderPowerOff() {
	EREAD_CTRL0  =  0x04;
	EREAD_RESET &= ~0x02;
	EREAD_CTRL1 &= ~0x0020;
}

void ereaderInit() {
	if((EREAD_CALDATA[0x3C] & 0x03)==1) ereaderInitCam1();
	EREAD_CTRL0 |=  0x0010;
	if((EREAD_CALDATA[0x3C] & 0x03)==2) ereaderInitCam1();
	//memcpy( EREAD_INTENSITY, &EREAD_CALDATA[0x00], 0x2F);
	//memcpy(&EREAD_LED,       &EREAD_CALDATA[0x32], 0x02);
}

void ereaderInitCam1() {}
void ereaderInitCam2() {}