#ifndef GLOBALS_H
#define GLOBALS_H

#include <gccore.h>

extern char romFile[64];
extern char saveFile[64];

#define readBuffSize 0x4000

extern u8 *testdump;

extern s32 gbaChan;

extern int gbasize;
extern u32 savesize;

extern const char *biosname;

typedef struct {
	char gameName[12];
	char gameId[4];
	char makerId[2];
	char fixed;
	char unitCode;
	char devType;
	char reserved[7];
	char version;
	char complement;
	char reserved2[2];
} romHeaderT;
extern romHeaderT romHeader;

#endif