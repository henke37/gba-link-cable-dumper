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
#include <dirent.h>
#include <fat.h>

#include "gba-joyport.h"
#include "gba-upload.h"
#include "packets.h"

#include "gba_mb_gba.h"

u8 *testdump;

s32 gbaChan=1;

int gbasize=0;
u32 savesize=0;

char romFile[64];
char saveFile[64];

const char *biosname = "/dumps/gba_bios.bin";

void printmain()
{
	printf("\x1b[2J");
	printf("\x1b[37m");
	printf("GBA Link Cable Dumper v1.6 by FIX94\n");
	printf("Save Support based on SendSave by Chishm\n");
	printf("GBA BIOS Dumper by Dark Fader\n \n");
}

void dumpGbaBios();
void handleGbaCart();
void dumpRom();
void backupSave();
void restoreSave();
void clearSave();

int fileExists(const char *fileName);

void endproc()
{
	printf("Start pressed, exit\n");
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	exit(0);
}
void fixFName(char *str)
{
	u8 i = 0;
	for(i = 0; i < strlen(str); ++i)
	{
		if(str[i] < 0x20 || str[i] > 0x7F)
			str[i] = '_';
		else switch(str[i])
		{
			case '\\':
			case '/':
			case ':':
			case '*':
			case '?':
			case '\"':
			case '<':
			case '>':
			case '|':
				str[i] = '_';
				break;
			default:
				break;
		}
	}
}

bool dirExists(const char *path)
{
	DIR *dir;
	dir = opendir(path);
	if(dir)
	{
		closedir(dir);
		return true;
	}
	return false;
}
void warnError(char *msg)
{
	puts(msg);
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	sleep(2);
}
void fatalError(char *msg)
{
	puts(msg);
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	sleep(5);
	exit(0);
}
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
	
	if(!fatInitDefault())
	{
		printmain();
		fatalError("ERROR: No usable device found to write dumped files to!");
	}
	mkdir("/dumps", S_IREAD | S_IWRITE);
	if(!dirExists("/dumps"))
	{
		printmain();
		fatalError("ERROR: Could not create dumps folder, make sure you have a supported device connected!");
	}
	while(1)
	{
		printmain();
		printf("Waiting for a GBA in port %d...\n", gbaChan+1);

		while(1)
		{
			if(isGbaConnected(gbaChan)) break;
			PAD_ScanPads();
			VIDEO_WaitVSync();
			if(PAD_ButtonsHeld(0))
				endproc();
		}
		printf("GBA Found! Waiting on BIOS\n");
		
		waitGbaBios(gbaChan);
		
		printf("Ready, sending dumper\n");
		
		gbaUploadMultiboot(gbaChan, gba_mb_gba, gba_mb_gba_size);
		
		printf("Done!\n");
		sleep(2);
		
		sendToGba(gbaChan, PING);
		sendToGba(gbaChan, 1234);
		if(recvFromGba(gbaChan)!=1234) {
			fatalError("Ping failure!");
		}
		
		while(1)
		{
			printmain();
			printf("Press A once you have a GBA Game inserted.\n");
			printf("Press Y to backup the GBA BIOS.\n \n");
			PAD_ScanPads();
			VIDEO_WaitVSync();
			
			u32 btns = PAD_ButtonsDown(0);
			if(btns&PAD_BUTTON_START) {
				endproc();
			} else if(btns&PAD_BUTTON_A) {
				handleGbaCart();
			} else if(btns&PAD_BUTTON_Y) {
				dumpGbaBios();
			}
		
		}
	}
	return 0;
}

void dumpGbaBios() {
	FILE *f = fopen(biosname,"rb");
	if(f)
	{
		fclose(f);
		warnError("ERROR: BIOS already backed up!\n");
		return;
	}

	//create base file with size
	printf("Preparing file...\n");
	f = fopen(biosname,"wb");
	if(!f)
		fatalError("ERROR: Could not create file! Exit...");
	//send over bios dump command
	sendToGba(gbaChan, READ_BIOS);
	//lets go!
	printf("Dumping...\n");
	
	recvBuffFromGba(gbaChan, testdump, 0x4000);
		
	fwrite(testdump,0x4000,1,f);
	printf("Closing file\n");
	fclose(f);
	
	size_t readCnt=recvFromGba(gbaChan);
	if(readCnt!=savesize) {
		fatalError("Read size desync!\n");
	}
	
	printf("BIOS dumped.\n");
	sleep(5);
}

void handleGbaCart() {
	printf("Waiting for GBA\n");
	VIDEO_WaitVSync();
	
	sendToGba(gbaChan, CHECK_GAME);
	gbasize = recvFromGba(gbaChan);
	savesize = recvFromGba(gbaChan);
	
	if(gbasize == -1) 
	{
		warnError("ERROR: No (Valid) GBA Card inserted!\n");
		return;
	}
	
	sendToGba(gbaChan, READ_HEADER);
	recvBuffFromGba(gbaChan, testdump, 0xC0);
		
	//print out all the info from the  game
	printf("Game Name: %.12s\n",(char*)(testdump+0xA0));
	printf("Game ID: %.4s\n",(char*)(testdump+0xAC));
	printf("Company ID: %.2s\n",(char*)(testdump+0xB0));
	printf("ROM Size: %02.02f MB\n",((float)(gbasize/1024))/1024.f);
	if(savesize > 0)
		printf("Save Size: %02.02f KB\n \n",((float)(savesize))/1024.f);
	else
		printf("No Save File\n \n");
		
	//generate file paths
	sprintf(romFile,"/dumps/%.12s [%.4s%.2s].gba",
		(char*)(testdump+0xA0),(char*)(testdump+0xAC),(char*)(testdump+0xB0));
	fixFName(romFile+7); //fix name behind "/dumps/"
	
	sprintf(saveFile,"/dumps/%.12s [%.4s%.2s].sav",
		(char*)(testdump+0xA0),(char*)(testdump+0xAC),(char*)(testdump+0xB0));
	fixFName(saveFile+7); //fix name behind "/dumps/"
	
	int romExists = fileExists(romFile);
	int saveExists = fileExists(saveFile);
	
	//let the user choose the option
	if(!romExists){
		printf("Press A to dump this game, it will take about %i minutes.\n",gbasize/1024/1024*3/2);
	} else {
		printf("Rom dumped.\n");
	}
	printf("Press B if you want to cancel dumping this game.\n");
	if(savesize > 0)
	{
		if(!saveExists) {
			printf("Press Y to backup this save file.\n");
		} else {
			printf("Press X to restore this save file.\n");
		}
		printf("Press L+R to clear the save file on the GBA Cartridge.\n");
	}
	printf("\n");
	
	while(1)
	{
		PAD_ScanPads();
		//sendToGba( gbaChan, READ_PAD);
		//u32 gbaBtns = recvFromGba(gbaChan);
		
		VIDEO_WaitVSync();
		u32 btns = PAD_ButtonsDown(0);
		if(btns&PAD_BUTTON_START)
			endproc();
		
		if(btns&PAD_BUTTON_A) {
			dumpRom();
		} else if(btns & PAD_BUTTON_B) {
			return;
		} else if(btns & PAD_BUTTON_Y) {
			backupSave();
		} else if(btns & PAD_BUTTON_X) {
			restoreSave();
		} else if((btns & (PAD_TRIGGER_L | PAD_TRIGGER_R))==(PAD_TRIGGER_L | PAD_TRIGGER_R)) {
			clearSave();
		}
	}
}

void dumpRom() {
//create base file with size
	printf("Preparing file...\n");
	FILE *f = fopen(romFile,"wb");
	if(!f)
		fatalError("ERROR: Could not create file! Exit...");
	printf("Dumping...\n");
	sendToGba(gbaChan, READ_ROM);
	while(gbasize > 0)
	{
		int toread = (gbasize > 0x400000 ? 0x400000 : gbasize);
		
		recvBuffFromGba(gbaChan, testdump, toread);
		
		fwrite(testdump,1,toread,f);
		gbasize -= toread;
	}
	printf("\nClosing file\n");
	fclose(f);
	
	size_t readCnt=recvFromGba(gbaChan);
	if(readCnt!=savesize) {
		fatalError("Read size desync!\n");
	}
	printf("Game dumped.\n");
	sleep(5);
}

void backupSave() {
	printf("Preparing file...\n");
	FILE *f = fopen(saveFile,"wb");
	if(!f)
		fatalError("ERROR: Could not create file! Exit...");
	printf("Waiting for GBA\n");
	VIDEO_WaitVSync();
	
	sendToGba(gbaChan,READ_SAVE); 
	printf("Receiving...\n");
	
	recvBuffFromGba(gbaChan, testdump, savesize);
	
	size_t readCnt=recvFromGba(gbaChan);
	if(readCnt!=savesize) {
		fatalError("Read size desync!\n");
	}
	
	printf("Writing save...\n");
	fwrite(testdump,1,savesize,f);
	fclose(f);
	printf("Save backed up.\n");
	sleep(5);
}

void restoreSave() {
	printf("Preparing file...\n");
	FILE *f = fopen(saveFile,"rb");
	if(!f) {
		warnError("ERROR: Could not open file!");
		return;
	}
	
	printf("Reading save\n");
	VIDEO_WaitVSync();
	size_t readC=fread(testdump,1,savesize,f);
	if(readC!=savesize) {
		fclose(f);
		if(feof(f)) {
			warnError("Error: File too short!");
		} else {
			warnError("Error: Reading file failed!");
		}
		return;
	}
	
	printf("Sending save\n");
	VIDEO_WaitVSync();
	sendToGba(gbaChan, WRITE_SAVE);
	sendBuffToGba(gbaChan, testdump, savesize);
	
	fclose(f);
	printf("Waiting for GBA\n");
	size_t written=recvFromGba(gbaChan);
	if(written!=savesize) {
		fatalError("Write size desync!\n");
	}
	printf("Save restored.\n");
	sleep(5);
}

void clearSave() {
	printf("Clearing save\n");
	VIDEO_WaitVSync();
	sendToGba(gbaChan, CLEAR_SAVE);
	
	size_t written=recvFromGba(gbaChan);
	if(written!=savesize) {
		fatalError("Write size desync!\n");
	}
	printf("Done.");
	sleep(5);
}

int fileExists(const char *fileName) {
	return access(fileName, F_OK);
}