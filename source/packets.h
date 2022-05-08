#ifndef PACKETS_H
#define PACKETS_H

typedef enum {
	PING=1,
	PONG=2,
	TST_READBUF=3,
	TST_SENDBUF=4,
	READ_PAD=5,
	CHECK_GAME=10,
	READ_ROM=11,
	READ_SAVE=20,
	WRITE_SAVE=21,
	CLEAR_SAVE=22,
	READ_BIOS=30,
	RTC_READ=40,
	RTC_WRITE=41,
	SOLAR_READ=42,
	TILT_READ=43,
	GYRO_READ=44,
	RUMBLE=45,
	EREADER_INIT=50,
	EREADER_SCAN=51,
	EREADER_READ=52,
	RUMBLE_DS=100,
	SLIDER_DS=101
} packetType;

#endif