/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include <gccore.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fat.h>

#include "gba-joyport.h"
#include "packets.h"
#include "menus.h"
#include "utils.h"
#include "globals.h"

u8 *testdump;

s32 gbaChan=1;

int gbasize=0;
u32 savesize=0;

romHeaderT romHeader;

const char *biosname = "/dumps/gba_bios.bin";

int main(int argc, char *argv[]) 
{
	void *xfb = NULL;
	GXRModeObj *rmode = NULL;
	
	VIDEO_Init();
	rmode = VIDEO_GetPreferredMode(NULL);
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(xfb);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();
	
	int x = 24, y = 32, w, h;
	w = rmode->fbWidth - (32);
	h = rmode->xfbHeight - (48);
	
	CON_InitEx(rmode, x, y, w, h);
	VIDEO_ClearFrameBuffer(rmode, xfb, COLOR_BLACK);
	
	PAD_Init();
	initGbaJoyport();
	
	testdump = memalign(32,0x400000);
	if(!testdump) return 0;
	
	printBanner();
	
	if(!fatInitDefault()) {
		fatalError("ERROR: No usable device found to write dumped files to!");
	}
	mkdir("/dumps", S_IREAD | S_IWRITE);
	if(!dirExists("/dumps")) {
		fatalError("ERROR: Could not create dumps folder, make sure you have a supported device connected!");
	}
	while(1) {
		waitGbaConnect();
		
		preDumpMenu();
	}
	return 0;
}
