#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include <gba.h>

void initGPIO();

void initRTC();

void solarRead();
void setRumble(bool active);

u16 readGyro();

struct rtcTime {
	u8 year;
	u8 month;
	u8 day;
	u8 dayOfWeek;
	bool pm;
	u8 hour;
	u8 min;
	u8 sec;
};

void rtcResetTime();
u8   rtcReadStatus();
void rtcWriteStatus(u8 data);
struct rtcTime rtcReadTime();
void rtcSetTime(struct rtcTime);

#endif