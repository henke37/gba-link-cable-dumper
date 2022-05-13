#ifndef COMMANDS_H
#define COMMANDS_H

#include <gccore.h>

//TODO: factor out command access in these
void dumpGbaBios();
void handleGbaCart();
void dumpRom();
void backupSave();
void restoreSave();
void clearSave();
void testComs();
void sendDumper();

//actual protocol commands
void readRom(u8 *buff,u32 offset, u32 len);
void setRumble(bool active);
u16 readGyro();

struct tiltData {
	u16 x;
	u16 y;
};
struct tiltData readTilt();

#endif