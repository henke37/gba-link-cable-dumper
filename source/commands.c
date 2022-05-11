#include "commands.h"

#include <stdio.h>
#include <gccore.h>
#include <unistd.h>

#include "globals.h"
#include "gba-joyport.h"
#include "gba-upload.h"
#include "utils.h"
#include "globals.h"
#include "packets.h"
#include "menus.h"

#include "gba_mb_gba.h"

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


void readRom(u8 *buff,u32 offset, u32 len) {
	sendToGba(gbaChan, READ_ROM);
	sendToGba(gbaChan, offset);
	sendToGba(gbaChan, len);
	recvBuffFromGba(gbaChan, buff, len);
	u32 readBytes=recvFromGba(gbaChan);
	if(readBytes!=len) {
		fatalError("Read rom size missmatch!");
	}
}

void dumpRom() {
//create base file with size
	printf("Preparing file...\n");
	FILE *f = fopen(romFile,"wb");
	if(!f)
		fatalError("ERROR: Could not create file! Exit...");
	printf("Dumping...\n");
	
	int offset=0;
	while(offset < gbasize) {
		int toRead = gbasize-offset;
		if(toRead > readBuffSize) toRead= readBuffSize;
		
		readRom(testdump, offset, toRead);
		
		fwrite(testdump,1,toRead,f);
		offset += toRead;
		
		printf("\r%02.02f MB done",(float)(offset/1024)/1024.f);
	}
	printf("\nClosing file\n");
	fclose(f);
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


void testComs() {
	sendToGba(gbaChan, PING);
	sendToGba(gbaChan, 1234);
	if(recvFromGba(gbaChan)!=1234) {
		fatalError("Ping failure!");
	}
	
	sendToGba(gbaChan, PONG);
	sendToGba(gbaChan, recvFromGba(gbaChan));
	
	printf("Send TST_READBUF packet. ");
	sendToGba(gbaChan, TST_READBUF);
	printf("Recv buff ");
	recvBuffFromGba(gbaChan, testdump, 40);
	for(int i=0;i<40;++i) {
		printf("%02d",testdump[i]);
	}
	for(int i=0;i<40;++i) {
		if(testdump[i]!=i) fatalError("Failed!");
	}
	printf("Pass.\n");
	
	printf("Send TST_SENDBUF packet. ");
	sendToGba(gbaChan, TST_SENDBUF);
	for(int i=0;i<40;++i) {
		testdump[i]=i;
	}
	printf("Send buff.\n");
	sendBuffToGba(gbaChan, testdump, 40);
}

void sendDumper() {
	printf("GBA Found! Waiting on BIOS\n");
	
	waitGbaBios(gbaChan);
	
	printf("Ready, sending dumper\n");
	
	gbaUploadMultiboot(gbaChan, gba_mb_gba, gba_mb_gba_size);
}

void setRumble(bool active) {
	sendToGba(gbaChan, RUMBLE);
	sendToGba(gbaChan, active);
}