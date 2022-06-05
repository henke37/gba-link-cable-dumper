#include "handler.h"

#include "gpio.h"
#include "tilt.h"
#include "joybus.h"
#include "main.h"
#include "libSave.h"
#include "transman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../source/packets.h"

void sendBiosDump();
void finishWriteSave();

void handlePacket(u32 type) {
	switch(type) {
		case PING: {
			iprintf("Got a ping!\n");
			sendJoyBus(recvJoyBus());
		} break;
		case PONG: {
			iprintf("PONG ");
			sendJoyBus(4321);
			if(recvJoyBus()!=4321) {
				iprintf("FAIL!");
			} else {
				iprintf("OK!");
			}
		} break;
		
		case TST_READBUF: {
			iprintf("TST_RB");
			for(int i=0;i<TESTBUF_LEN;++i) {
				save_data[i]=i/10;
			}
			sendJoyBusBuff(save_data, TESTBUF_LEN);
		} break;
		
		case TST_READZEROS: {
			iprintf("TST_RZ");
			for(int i=0;i<TESTBUF_LEN;++i) {
				save_data[i]=0;
			}
			sendJoyBusBuff(save_data, TESTBUF_LEN);
		} break;
		
		case TST_SENDBUF: {
			iprintf("TST_SB ");
			memset(save_data,0,TESTBUF_LEN);
			recvJoyBusBuff(save_data, TESTBUF_LEN);
			for(int i=0;i<TESTBUF_LEN;++i) {
				if(save_data[i]!=i) {
					iprintf("FAIL!");
					break;
				}
			}
			iprintf("PASS");
		} break;
		
		case READ_PAD: {
			sendJoyBus(REG_KEYINPUT);
		} break;
		
		case CHECK_GAME: {
			iprintf("Checking game...\n");
			gamesize = getGameSize();
			savesize = SaveSize(save_data,gamesize);
			sendJoyBus(gamesize);
			sendJoyBus(savesize);
			
			initHW();
		} break;
			
		case READ_ROM:	{
			u32 offset = recvJoyBus();
			u32 length = recvJoyBus();
			const u8* addr=ROM_DATA+offset;
			iprintf("ROMREAD: %p %lx ",addr,length);
			
			transManSetSend(addr, length, transManSendCompleteDefaultCb);
		} break;
		
		case READ_SAVE: {
			iprintf("Reading save.\n");
			
			readSave(save_data,savesize);
			transManSetSend(save_data, savesize, transManSendCompleteDefaultCb);
		} break;
		
		case WRITE_SAVE: {
			iprintf("Writing save.\n");
			
			transManSetRecv(save_data, savesize, finishWriteSave);
		} break;
		
		case CLEAR_SAVE: {
			iprintf("Erasing save. Hope you made a copy first!\n");
			
			//clear the save
			memset(save_data, 0, savesize);
				
			writeSave(save_data, savesize);
			
			sendJoyBus(savesize);
		} break;
		
		case READ_BIOS:{
			iprintf("Purloining bios\n");
			sendBiosDump();
		} break;
		
		case RUMBLE:{
			setRumble(recvJoyBus()!=0);
		} break;
		
		case TILT_READ: {
			struct tiltData tilt=readTilt();
			sendJoyBus(tilt.x);
			sendJoyBus(tilt.y);
			startTiltScan();
		} break;
		
		case GYRO_READ:{
			sendJoyBus(readGyro());
		}break;
		
		case RTC_READ: {
			u8 status = rtcReadStatus();
			struct rtcTime time=rtcReadTime();
			sendJoyBus(time.year | time.month << 8 | time.day << 16 | status << 24);
			sendJoyBus((time.hour +(time.pm?12:0)) | time.min << 8 | time.sec << 16);
		} break;
		
		case SOLAR_READ: {
			u8 status=solarRead();
			sendJoyBus(status);
		} break;
		
		default:
			iprintf("Got unknown packet %#010lx!\n",type);
		break;
	}
}


void sendBiosDump() {
	int i;
	//disable interrupts
	u32 prevIrqMask = REG_IME;
	REG_IME = 0;
	//dump BIOS
	for (i = 0; i < biosSize; i+=4)
	{
		u8 a = purloinBiosData(i);
		u8 b = purloinBiosData(i+1);
		u8 c = purloinBiosData(i+2);
		u8 d = purloinBiosData(i+3);
		u32 abcd = a | (b<<8) | (c<<16) | (d<<24);
		sendJoyBus(abcd);
	}
	//restore interrupts
	REG_IME = prevIrqMask;
	
	sendJoyBus(biosSize);
}

void finishWriteSave() {
	writeSave(save_data, savesize);
	sendJoyBus(savesize);
}