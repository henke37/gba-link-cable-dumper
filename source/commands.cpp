#include "commands.h"

#include <stdio.h>
#include <gccore.h>
#include <unistd.h>

#include "globals.h"
#include "gba-joybus.h"
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
	gbaCon[gbaChan].send(READ_BIOS);
	//lets go!
	printf("Dumping...\n");
	
	gbaCon[gbaChan].recvBuff(testdump, 0x4000);
		
	fwrite(testdump,0x4000,1,f);
	printf("Closing file\n");
	fclose(f);
	
	size_t readCnt=gbaCon[gbaChan].recv();
	if(readCnt!=savesize) {
		fatalError("Read size desync!\n");
	}
	
	printf("BIOS dumped.\n");
	sleep(5);
}


void readRom(u8 *buff,u32 offset, u32 len) {
	gbaCon[gbaChan].send(READ_ROM);
	gbaCon[gbaChan].send(offset);
	gbaCon[gbaChan].send(len);
	gbaCon[gbaChan].recvBuff(buff, len);
	u32 readBytes=gbaCon[gbaChan].recv();
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
	
	gbaCon[gbaChan].send(READ_SAVE); 
	printf("Receiving...\n");
	
	gbaCon[gbaChan].recvBuff(testdump, savesize);
	
	size_t readCnt=gbaCon[gbaChan].recv();
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
	gbaCon[gbaChan].send(WRITE_SAVE);
	gbaCon[gbaChan].sendBuff(testdump, savesize);
	
	fclose(f);
	printf("Waiting for GBA\n");
	size_t written=gbaCon[gbaChan].recv();
	if(written!=savesize) {
		fatalError("Write size desync!\n");
	}
	printf("Save restored.\n");
	sleep(5);
}

void clearSave() {
	printf("Clearing save\n");
	VIDEO_WaitVSync();
	gbaCon[gbaChan].send(CLEAR_SAVE);
	
	size_t written=gbaCon[gbaChan].recv();
	if(written!=savesize) {
		fatalError("Write size desync!\n");
	}
	printf("Done.");
	sleep(5);
}


void testComs() {
	gbaCon[gbaChan].send(PING);
	gbaCon[gbaChan].send(1234);
	if(gbaCon[gbaChan].recv()!=1234) {
		fatalError("Ping failure!");
	}
	
	gbaCon[gbaChan].send(PONG);
	gbaCon[gbaChan].send(gbaCon[gbaChan].recv());
	
	printf("Send TST_READBUF packet. ");
	gbaCon[gbaChan].send(TST_READBUF);
	printf("Recv buff ");
	gbaCon[gbaChan].recvBuff(testdump, TESTBUF_LEN);
	for(int i=0;i<TESTBUF_LEN;++i) {
		printf("%02d",testdump[i]);
	}
	for(int i=0;i<TESTBUF_LEN;++i) {
		if(testdump[i]!=(i/10)) fatalError("Failed!");
	}
	printf("Pass.\n");
	
	printf("Send TST_SENDBUF packet. ");
	gbaCon[gbaChan].send(TST_SENDBUF);
	for(int i=0;i<TESTBUF_LEN;++i) {
		testdump[i]=i;
	}
	printf("Send buff.\n");
	gbaCon[gbaChan].sendBuff(testdump, TESTBUF_LEN);
	
	printf("Send TST_READZEROS packet. ");
	gbaCon[gbaChan].send(TST_READZEROS);
	printf("Recv buff ");
	gbaCon[gbaChan].recvBuff(testdump, TESTBUF_LEN);
	for(int i=0;i<TESTBUF_LEN;++i) {
		printf("%02d",testdump[i]);
	}
	for(int i=0;i<TESTBUF_LEN;++i) {
		if(testdump[i]!=0) fatalError("Failed!");
	}
	printf("Pass.\n");
}

void sendDumper() {
	printf("GBA Found! Waiting on BIOS\n");
	
	waitGbaBios(gbaCon[gbaChan]);
	
	printf("Ready, sending dumper\n");
	
	gbaUploadMultiboot(gbaCon[gbaChan], gba_mb_gba, gba_mb_gba_size);
}

void setRumble(bool active) {
	gbaCon[gbaChan].send(RUMBLE);
	gbaCon[gbaChan].send(active);
}

u16 readGyro() {
	gbaCon[gbaChan].send(GYRO_READ);
	return gbaCon[gbaChan].recv();
}

struct tiltData readTilt() {
	struct tiltData tilt;
	gbaCon[gbaChan].send(TILT_READ);
	tilt.x=gbaCon[gbaChan].recv();
	tilt.y=gbaCon[gbaChan].recv();
	return tilt;
}

struct rtcData readRtc() {
	struct rtcData time;
	
	gbaCon[gbaChan].send(RTC_READ);
	
	u32 data=gbaCon[gbaChan].recvRaw();
	time.status=(data >> 24) & 0x00FF;
	time.day=(data >> 16) & 0x00FF;
	time.month=(data >> 8) & 0x00FF;
	time.year=data & 0x00FF;
	
	data=gbaCon[gbaChan].recvRaw();
	time.sec=(data >> 16) & 0x00FF;
	time.min=(data >> 8) & 0x00FF;
	time.hour=data & 0x00FF;
	
	return time;
}