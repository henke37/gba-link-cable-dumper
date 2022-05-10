#include "menus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <unistd.h>

#include "gba-joyport.h"
#include "gba-upload.h"
#include "packets.h"
#include "utils.h"
#include "globals.h"
#include "commands.h"

char romFile[64];
char saveFile[64];

void fixFName(char *str);

void clearScreen();

void specialHWMenu();
void ereaderMenu();
void rtcAndUvMenu();

void printBanner() {
	printf("\x1b[2J");
	printf("\x1b[37m");
	printf("GBA Link Cable Dumper v2.0 by Henke37 and original code by FIX94\n");
	printf("Save Support based on SendSave by Chishm\n");
	printf("GBA BIOS Dumper by Dark Fader\n \n");
}

void handleGbaCart() {
	printf("Waiting for GBA\n");
	VIDEO_WaitVSync();
	
	sendToGba(gbaChan, CHECK_GAME);
	gbasize = recvFromGba(gbaChan);
	savesize = recvFromGba(gbaChan);
	
	if(gbasize == -1) {
		warnError("ERROR: No (Valid) GBA Card inserted!\n");
		return;
	}
	
	readRom(((void*)&romHeader),0x00A0,sizeof(romHeader));
	
	clearScreen();
		
	//print out all the info from the  game
	printf("Game Name: %.12s\n",romHeader.gameName);
	printf("Game ID: %.4s\n",romHeader.gameId);
	printf("Company ID: %.2s\n",romHeader.makerId);
	printf("ROM Size: %02.02f MB\n",((float)(gbasize/1024))/1024.f);
	if(savesize > 0)
		printf("Save Size: %02.02f KB\n \n",((float)(savesize))/1024.f);
	else
		printf("No Save File\n \n");
		
	//generate file paths
	sprintf(romFile,"/dumps/%.12s [%.4s%.2s].gba",
		romHeader.gameName, romHeader.gameId, romHeader.makerId);
	fixFName(romFile+7); //fix name behind "/dumps/"
	
	sprintf(saveFile,"/dumps/%.12s [%.4s%.2s].sav",
		romHeader.gameName, romHeader.gameId, romHeader.makerId);
	fixFName(saveFile+7); //fix name behind "/dumps/"
	
	bool romExists = false;//fileExists(romFile);
	bool saveExists = fileExists(saveFile);
	
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
	if(hasSpecialHardware()) {
		printf("Press Z for additional options.\n");
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
		} else if(btns & PAD_TRIGGER_Z) {
			specialHWMenu();
		}
	}
}

void waitGbaConnect() {
	printf("Waiting for a GBA in port %d...\n", gbaChan+1);

	while(1) {
		if(isGbaConnected(gbaChan)) break;
		PAD_ScanPads();
		VIDEO_WaitVSync();
		if(PAD_ButtonsHeld(0))
			endproc();
	}
	
	sendDumper();
	
	printf("Done!\n");
	sleep(2);
	
	testComs();
}

void preDumpMenu() {
	while(1) {
		clearScreen();
		printf("Press A once you have a GBA Game inserted.\n");
		printf("Press Y to backup the GBA BIOS.\n \n");
		
		while(1) {
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
}

void specialHWMenu() {
	switch(romHeader.gameId[0]) {
		case 'P'://ereader
			ereaderMenu();
			break;
		case 'U'://RTC & UV
			rtcAndUvMenu();
			break;
		case 'K'://accl
		case 'R'://gyro & rumble
		case 'V'://Rumble
		default:
			printf("Not implemented.\n");
	}
}

void ereaderMenu() {
}
void rtcAndUvMenu() {
}

void warnError(char *msg) {
	puts(msg);
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	sleep(2);
}
void fatalError(char *msg) {
	puts(msg);
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	sleep(5);
	exit(0);
}
void endproc() {
	printf("Start pressed, exit\n");
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	exit(0);
}

void fixFName(char *str) {
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

void clearScreen() {
	printf("\x1b[1J\x1b[1;0H");
}