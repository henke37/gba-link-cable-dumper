#include "menus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gccore.h>
#include <unistd.h>

#include "gba-joybus.h"
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
void gyroAndRumbleMenu();
void tiltMenu();

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
	
	gbaCon[gbaChan].send(CHECK_GAME);
	gbasize = gbaCon[gbaChan].recv();
	savesize = gbaCon[gbaChan].recv();
	
	if(gbasize == -1) {
		warnError("ERROR: No (Valid) GBA Card inserted!\n");
		return;
	}
	
	readRom(((u8*)&romHeader),0x00A0,sizeof(romHeader));
	
	//generate file paths
	sprintf(romFile,"/dumps/%.12s [%.4s%.2s].gba",
		romHeader.gameName, romHeader.gameId, romHeader.makerId);
	fixFName(romFile+7); //fix name behind "/dumps/"
	
	sprintf(saveFile,"/dumps/%.12s [%.4s%.2s].sav",
		romHeader.gameName, romHeader.gameId, romHeader.makerId);
	fixFName(saveFile+7); //fix name behind "/dumps/"
	
	/*for(int i=0;i<sizeof(romHeader);++i) {
		printf("%.2X",((u8*)&romHeader)[i]);
	}*/
	
	while(1) {
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
			
		
		bool romExists = fileExists(romFile);
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
		
		while(1) {
			PAD_ScanPads();
			//sendToGba( gbaChan, READ_PAD);
			//u32 gbaBtns = gbaCon[gbaChan].recv();
			
			VIDEO_WaitVSync();
			u32 btns = PAD_ButtonsDown(0);
			if(btns&PAD_BUTTON_START)
				endproc();
			
			if(btns&PAD_BUTTON_A && ( !romExists || (PAD_ButtonsHeld(0) & PAD_TRIGGER_L) ) ) {
				dumpRom();
				break;
			} else if(btns & PAD_BUTTON_B) {
				return;
			} else if(btns & PAD_BUTTON_Y) {
				backupSave();
				break;
			} else if(btns & PAD_BUTTON_X) {
				restoreSave();
				break;
			} else if((PAD_ButtonsHeld(0) & (PAD_TRIGGER_L | PAD_TRIGGER_R))==(PAD_TRIGGER_L | PAD_TRIGGER_R)) {
				clearSave();
				break;
			} else if(btns & PAD_TRIGGER_Z) {
				specialHWMenu();
				break;
			}
		}
	
	}
}

bool detectGba() {
	for(gbaChan=0;gbaChan<3;++gbaChan) {
		if(gbaCon[gbaChan].isGbaConnected()) return true;
	}
	gbaChan=-1;
	return false;
}

void waitGbaConnect() {
	printf("Waiting for a GBA...\n");

	while(1) {
		if(detectGba()) break;
		PAD_ScanPads();
		VIDEO_WaitVSync();
		if(PAD_ButtonsHeld(0))
			endproc();
	}
	
	printf("Found one in port %d.\n", gbaChan+1);
	
	sendDumper();
	
	printf("Done!\n");
	sleep(2);
	
	testComs();
}

void preDumpMenu() {
	while(1) {
		clearScreen();
		
		bool biosDumped=fileExists(biosname);
		
		printf("Press A once you have a GBA Game inserted.\n");
		if(biosDumped) {
			printf("GBA BIOS dumped.\n");
		} else {
			printf("Press Y to backup the GBA BIOS.\n \n");
		}
		
		while(1) {
			PAD_ScanPads();
			VIDEO_WaitVSync();
			
			u32 btns = PAD_ButtonsDown(0);
			if(btns&PAD_BUTTON_START) {
				endproc();
			} else if(btns&PAD_BUTTON_A) {
				handleGbaCart();
				break;
			} else if(btns&PAD_BUTTON_Y && (!biosDumped || (PAD_ButtonsHeld(0) & PAD_TRIGGER_L))) {
				dumpGbaBios();
				break;
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
		case 'R'://gyro & rumble
			gyroAndRumbleMenu();
			break;
		case 'K'://tilt
			tiltMenu();
		case 'V'://Rumble
		default:
			printf("Not implemented.\n");
	}
}

void ereaderMenu() {
}
void rtcAndUvMenu() {
	struct rtcData time=readRtc();
	
	if(time.status & 0x0070) {
		printf("RTC: Bad power!\n");
	} else {
		printf(
			"RTC: %d/%d/%d %d:%d:%d\n",
			time.year+2000,
			time.month,
			time.day,
			time.hour,
			time.min,
			time.sec
		);
	}
	
	while(1) {
		PAD_ScanPads();
		VIDEO_WaitVSync();
		
		u32 btns = PAD_ButtonsDown(0);
		if(btns&PAD_BUTTON_START) {
			endproc();
		} else if(btns&PAD_BUTTON_B) {
			break;
		}
	}
}

void tiltMenu() {
	clearScreen();
	printf("Press B to return to dumping.\n");
	
	while(1) {
		PAD_ScanPads();
		VIDEO_WaitVSync();
		
		struct tiltData tilt=readTilt();
		printf("\rTilt: %.4hx %.4hx", tilt.x, tilt.y);
		
		u32 btns = PAD_ButtonsDown(0);
		if(btns&PAD_BUTTON_START) {
			endproc();
		} else if(btns&PAD_BUTTON_B) {
			break;
		}
	}
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

void gyroAndRumbleMenu() {
	clearScreen();
	printf("Hold A to shake shake.\n");
	printf("Press B to return to dumping.\n");
	
	while(1) {
		PAD_ScanPads();
		VIDEO_WaitVSync();
		
		u16 gyro=readGyro();
		printf("\rGyro: %.4hx", gyro);
		
		u32 btns = PAD_ButtonsDown(0);
		if(btns&PAD_BUTTON_START) {
			endproc();
		} else if(btns&PAD_BUTTON_B) {
			break;
		}
		
		if(btns&PAD_BUTTON_A) {
			setRumble(true);
		} else if(PAD_ButtonsUp(0)&PAD_BUTTON_A) {
			setRumble(false);
		}
	}
}