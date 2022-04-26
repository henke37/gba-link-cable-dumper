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

#include "gba_mb_gba.h"

u8 *testdump;

s32 gbaChan=1;

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
void createFile(const char *path, size_t size)
{
	int fd = open(path, O_WRONLY|O_CREAT);
	if(fd >= 0)
	{
		ftruncate(fd, size);
		close(fd);
	}
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
		//hm
		while(1)
		{
			printmain();
			printf("Press A once you have a GBA Game inserted.\n");
			printf("Press Y to backup the GBA BIOS.\n \n");
			PAD_ScanPads();
			VIDEO_WaitVSync();
			u32 btns = PAD_ButtonsDown(0);
			if(btns&PAD_BUTTON_START)
				endproc();
			else if(btns&PAD_BUTTON_A)
			{
				if(recvFromGba(gbaChan) == 0) {//ready
					handleGbaCart();
				}
				
			}
			else if(btns&PAD_BUTTON_Y)
			{
				dumpGbaBios();
			}
		
		}
	}
	return 0;
}

void dumpGbaBios() {
	const char *biosname = "/dumps/gba_bios.bin";
	FILE *f = fopen(biosname,"rb");
	if(f)
	{
		fclose(f);
		warnError("ERROR: BIOS already backed up!\n");
		return;
	}

	//create base file with size
	printf("Preparing file...\n");
	createFile(biosname,0x4000);
	f = fopen(biosname,"wb");
	if(!f)
		fatalError("ERROR: Could not create file! Exit...");
	//send over bios dump command
	sendToGba(gbaChan, 5);
	//the gba might still be in a loop itself
	sleep(1);
	//lets go!
	printf("Dumping...\n");
	
	recvBuffFromGba(gbaChan, testdump, 0x4000);
		
	fwrite(testdump,0x4000,1,f);
	printf("Closing file\n");
	fclose(f);
	printf("BIOS dumped!\n");
	sleep(5);
}

void handleGbaCart() {
	printf("Waiting for GBA\n");
	VIDEO_WaitVSync();
	
	int gbasize = 0;
	while(gbasize == 0)
		gbasize = __builtin_bswap32(recvFromGba(gbaChan));
	sendToGba(gbaChan, 0); //got gbasize
	u32 savesize = __builtin_bswap32(recvFromGba(gbaChan));
	sendToGba(gbaChan, 0); //got savesize
	
	if(gbasize == -1) 
	{
		warnError("ERROR: No (Valid) GBA Card inserted!\n");
		return;
	}
	
	//get rom header
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
	char gamename[64];
	sprintf(gamename,"/dumps/%.12s [%.4s%.2s].gba",
		(char*)(testdump+0xA0),(char*)(testdump+0xAC),(char*)(testdump+0xB0));
	fixFName(gamename+7); //fix name behind "/dumps/"
	char savename[64];
	sprintf(savename,"/dumps/%.12s [%.4s%.2s].sav",
		(char*)(testdump+0xA0),(char*)(testdump+0xAC),(char*)(testdump+0xB0));
	fixFName(savename+7); //fix name behind "/dumps/"
	
	//let the user choose the option
	printf("Press A to dump this game, it will take about %i minutes.\n",gbasize/1024/1024*3/2);
	printf("Press B if you want to cancel dumping this game.\n");
	if(savesize > 0)
	{
		printf("Press Y to backup this save file.\n");
		printf("Press X to restore this save file.\n");
		printf("Press Z to clear the save file on the GBA Cartridge.\n");
	}
	printf("\n");
	
	int command = 0;
	while(1)
	{
		PAD_ScanPads();
		VIDEO_WaitVSync();
		u32 btns = PAD_ButtonsDown(0);
		if(btns&PAD_BUTTON_START)
			endproc();
		else if(btns&PAD_BUTTON_A)
		{
			command = 1;
			break;
		}
		else if(btns&PAD_BUTTON_B)
			break;
		else if(savesize > 0)
		{
			if(btns&PAD_BUTTON_Y)
			{
				command = 2;
				break;
			}
			else if(btns&PAD_BUTTON_X)
			{
				command = 3;
				break;
			}
			else if(btns&PAD_TRIGGER_Z)
			{
				command = 4;
				break;
			}
		}
	}
	if(command == 1)
	{
		FILE *f = fopen(gamename,"rb");
		if(f)
		{
			fclose(f);
			command = 0;
			warnError("ERROR: Game already dumped!\n");
		}
	}
	else if(command == 2)
	{
		FILE *f = fopen(savename,"rb");
		if(f)
		{
			fclose(f);
			command = 0;
			warnError("ERROR: Save already backed up!\n");
		}
	}
	else if(command == 3)
	{
		size_t readsize = 0;
		FILE *f = fopen(savename,"rb");
		if(f)
		{
			fseek(f,0,SEEK_END);
			readsize = ftell(f);
			if(readsize != savesize)
			{
				command = 0;
				warnError("ERROR: Save has the wrong size, aborting restore!\n");
			}
			else
			{
				rewind(f);
				fread(testdump,readsize,1,f);
			}
			fclose(f);
		}
		else
		{
			command = 0;
			warnError("ERROR: No Save to restore!\n");
		}
	}
	sendToGba(gbaChan, command);
	//let gba prepare
	sleep(1);
	if(command == 0)
		return;
	else if(command == 1)
	{
		//create base file with size
		printf("Preparing file...\n");
		createFile(gamename,gbasize);
		FILE *f = fopen(gamename,"wb");
		if(!f)
			fatalError("ERROR: Could not create file! Exit...");
		printf("Dumping...\n");
		while(gbasize > 0)
		{
			int toread = (gbasize > 0x400000 ? 0x400000 : gbasize);
			
			recvBuffFromGba(gbaChan, testdump, toread);
			
			fwrite(testdump,toread,1,f);
			gbasize -= toread;
		}
		printf("\nClosing file\n");
		fclose(f);
		printf("Game dumped!\n");
		sleep(5);
	}
	else if(command == 2)
	{
		//create base file with size
		printf("Preparing file...\n");
		createFile(savename,savesize);
		FILE *f = fopen(savename,"wb");
		if(!f)
			fatalError("ERROR: Could not create file! Exit...");
		printf("Waiting for GBA\n");
		VIDEO_WaitVSync();
		u32 readval = 0;
		while(readval != savesize)
			readval = __builtin_bswap32(recvFromGba(gbaChan));
		sendToGba(gbaChan,0); //got savesize
		printf("Receiving...\n");
		
		recvBuffFromGba(gbaChan, testdump, savesize);
		
		printf("Writing save...\n");
		fwrite(testdump,savesize,1,f);
		fclose(f);
		printf("Save backed up!\n");
		sleep(5);
	}
	else if(command == 3 || command == 4)
	{
		u32 readval = 0;
		while(readval != savesize)
			readval = __builtin_bswap32(recvFromGba(gbaChan));
		if(command == 3)
		{
			printf("Sending save\n");
			VIDEO_WaitVSync();
			sendBuffToGba(gbaChan, testdump, savesize);
		}
		printf("Waiting for GBA\n");
		while(recvFromGba(gbaChan) != 0)
			VIDEO_WaitVSync();
		printf(command == 3 ? "Save restored!\n" : "Save cleared!\n");
		sendToGba(gbaChan, 0);
		sleep(5);
	}
}
				