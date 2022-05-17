#include "gpio.h"

#include <gba.h>

#define GPIO_DATA *((vu16*)0x80000C4)
#define GPIO_DIR  *((vu16*)0x80000C6)
#define GPIO_CTRL *((vu16*)0x80000C8)
#define ROM_WS0_READ ((vu32*)0x8000000)

#define RUMBLE_FLAG 0x08

void initGPIO() {
	GPIO_CTRL = 1;
}

void rtcRead();
void rtcWrite();

void solarRead();
void setRumble(bool active) {
	GPIO_DIR=0x0B;
	GPIO_DATA=(GPIO_DATA & ~RUMBLE_FLAG) | (active?RUMBLE_FLAG:0);
}

void readTilt();

u16 readGyro() {
	GPIO_DIR=0x0B;
	
	u16 rumbleBit = (GPIO_DATA & RUMBLE_FLAG);
	
	//Start conversion
	GPIO_DATA = rumbleBit | 0x01;
	GPIO_DATA = rumbleBit;
	
	u16 val=0;
	
	for(int i=0;i<16;++i) {
		//CLK to low
		GPIO_DATA = rumbleBit;
		
		ROM_WS0_READ[0x100];
		ROM_WS0_READ[0x101];
		ROM_WS0_READ[0x102];
		
		//read bit
		bool bit=GPIO_DATA & 0x04;		
		val |= bit << i;
		
		//CLK to high
		GPIO_DATA = rumbleBit | 0x02;
	}
	
	val &= 0x0FFF;
	
	return val;
}