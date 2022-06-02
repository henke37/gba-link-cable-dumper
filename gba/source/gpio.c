#include "gpio.h"

#include <gba.h>

#define GPIO_DATA *((vu16*)0x80000C4)
#define GPIO_DIR  *((vu16*)0x80000C6)
#define GPIO_CTRL *((vu16*)0x80000C8)
#define ROM_WS0_READ ((vu32*)0x8000000)

#define RUMBLE_FLAG 0x08

#define RTC_SCK 1
#define RTC_SIO 2
#define RTC_CS 4

#define SOL_CLK 1
#define SOL_RST 2
#define SOL_FLG 8

void initGPIO() {
	GPIO_CTRL = 1;
}

void initRTC() {
	initGPIO();
}

u8 toBCD(u8 x) {
	return (x%10) | ((x/10)<<4);
}
u8 fromBCD(u8 x) {
	return (x & 0x0F) | (((x&0x00F0) >> 4)*10);
}

bool rtcReadBit() {
	bool bit;
	GPIO_DATA = RTC_CS | RTC_SCK;
	GPIO_DATA = RTC_CS;
	bit = GPIO_DATA & RTC_SIO;
	
	return bit;
}
void rtcWriteBit(bool bit) {
	GPIO_DATA = RTC_CS | RTC_SCK | (bit?RTC_SIO:0);
	GPIO_DATA = RTC_CS | (bit?RTC_SIO:0);
}

void rtcWriteCMD(u8 data) {
	GPIO_DIR = RTC_CS | RTC_SCK | RTC_SIO;
	for(int i=7;i>=0;--i) {
		rtcWriteBit(data & 1<<i);
	}
}
void rtcWriteByte(u8 data) {
	GPIO_DIR = RTC_CS | RTC_SCK | RTC_SIO;
	for(int i=0;i<8;++i) {
		rtcWriteBit(data & 1<<i);
	}
}
u8 rtcReadByte() {
	u8 data=0;
	GPIO_DIR = RTC_CS | RTC_SCK;
	for(int i=0;i<8;++i) {
		data |= rtcReadBit() << i;
	}
	return data;
}

void rtcResetTime() {
	rtcWriteCMD(0x60);
}

u8 rtcReadStatus() {
	rtcWriteCMD(0x63);
	return rtcReadByte();
}
void rtcWriteStatus(u8 data) {
	rtcWriteCMD(0x62);
	rtcWriteByte(data);
}

struct rtcTime rtcReadTime() {
	struct rtcTime time;
	rtcWriteCMD(0x65);
	time.year=fromBCD(rtcReadByte());
	time.month=fromBCD(rtcReadByte());
	time.day=fromBCD(rtcReadByte());
	time.dayOfWeek=rtcReadByte();
	time.hour=rtcReadByte();
	time.pm=!(time.hour & 0x0070);
	time.hour=fromBCD(time.hour &~0x0070);
	time.min=fromBCD(rtcReadByte());
	time.sec=fromBCD(rtcReadByte());
	
	return time;
}

void rtcSetTime(struct rtcTime time) {
	rtcWriteCMD(0x64);
	rtcWriteByte(toBCD(time.year));
	rtcWriteByte(toBCD(time.month));
	rtcWriteByte(toBCD(time.day));
	rtcWriteByte(time.dayOfWeek);
	rtcWriteByte(toBCD(time.hour) | (time.pm?0:0x0070));
	rtcWriteByte(toBCD(time.min));
	rtcWriteByte(toBCD(time.sec));
}

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