#ifndef TILT_H
#define TILT_H

#include <gba.h>

struct tiltData {
	u16 x;
	u16 y;
};

struct tiltData readTilt();
void startTiltScan();

#endif