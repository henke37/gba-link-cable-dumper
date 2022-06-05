/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include <gba.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdnoreturn.h>

#include "main.h"
#include "libSave.h"
#include "joybus.h"
#include "transman.h"
#include "handler.h"
#include "gpio.h"
#include "tilt.h"

u8 save_data[0x20000] __attribute__ ((section (".sbss")));

void sioHandler();

void noreturn rebootSystem();

s32 gamesize;
u32 savesize;

//---------------------------------------------------------------------------------
// Program entry point
//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------

	// the vblank interrupt must be enabled for VBlankIntrWait() to work
	// since the default dispatcher handles the bios flags no vblank handler
	// is required
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	irqSet(IRQ_SERIAL, sioHandler);
	irqEnable(IRQ_SERIAL);
	
	transManInit();

	consoleDemoInit();
	
	// ansi escape sequence to set print co-ordinates
	// /x1b[line;columnH	
	//iprintf("\x1b[9;2HGBA Link Cable Dumper v1.6\n");
	//iprintf("\x1b[10;4HPlease look at the TV\n");
	
	// disable this, needs power
	SNDSTAT = 0;
	SNDBIAS = 0;
	
	// Set up waitstates for EEPROM access etc. 
	REG_WAITCNT = 0x0317;
	
	iprintf("Init:%#0x %#0x\n",REG_HS_CTRL, REG_JSTAT);
	
	enableJoyBusIRQ(true);
	
	while (1) {
		Halt();
	}
}

void sioHandler() {
	
	irqDisable(IRQ_SERIAL);
	
	do {
 
	//iprintf("IRQ: %#0x %#0x!\n",REG_HS_CTRL, REG_JSTAT);
	if(isJoyBusRecvPending()) {
		if(!transManRecvCB()) {
			u32 type=recvJoyBus();
			handlePacket(type);
		}
	}
	if(isJoyBusSendPending()) {
		if(!transManSendCB()) {
			iprintf("Spurious send?\n");
			REG_IME=0;
			while (1) {
				Halt();
			}
		}
	}
	if(isJoyBusResetPending()) {
		iprintf("Reset.\n");
		RegisterRamReset(RESET_SIO);
		rebootSystem();
	}
	
	} while(isJoyBusAnyPending());
	
	irqEnable(IRQ_SERIAL);
}

s32 getGameSize(void) {
	if(*(vu32*)(0x08000004) != 0x51AEFF24)
		return -1;
	s32 i;
	for(i = (1<<20); i < (1<<25); i<<=1)
	{
		vu16 *rompos = (vu16*)(0x08000000+i);
		int j;
		bool romend = true;
		for(j = 0; j < 0x1000; j++)
		{
			if(rompos[j] != j)
			{
				romend = false;
				break;
			}
		}
		if(romend) break;
	}
	return i;
}

void readSave(u8 *data, u32 savesize) {
	//disable interrupts
	u32 prevIrqMask = REG_IME;
	REG_IME = 0;
	//backup save
	switch (savesize){
	case 0x200:
		GetSave_EEPROM_512B(data);
		break;
	case 0x2000:
		GetSave_EEPROM_8KB(data);
		break;
	case 0x8000:
		GetSave_SRAM_32KB(data);
		break;
	case 0x10000:
		GetSave_FLASH_64KB(data);
		break;
	case 0x20000:
		GetSave_FLASH_128KB(data);
		break;
	default:
		break;
	}
	//restore interrupts
	REG_IME = prevIrqMask;
}

void writeSave(u8 *data, u32 savesize) {
	//disable interrupts
	u32 prevIrqMask = REG_IME;
	REG_IME = 0;
	//write it
	switch (savesize){
	case 0x200:
		PutSave_EEPROM_512B(data);
		break;
	case 0x2000:
		PutSave_EEPROM_8KB(data);
		break;
	case 0x8000:
		PutSave_SRAM_32KB(data);
		break;
	case 0x10000:
		PutSave_FLASH_64KB(data);
		break;
	case 0x20000:
		PutSave_FLASH_128KB(data);
		break;
	default:
		break;
	}
	//restore interrupts
	REG_IME = prevIrqMask;
}

u8 purloinBiosData(int offset) {
	WaveData *fakeAddr=(WaveData *)( offset-(((offset & 3)+1)| 3) );
	u8 b = (MidiKey2Freq(fakeAddr, 168, 0) * 2) >> 24;
	return b;
}

void noreturn rebootSystem() {
	SystemCall(0x26); 
	__builtin_unreachable();
}

#define rtcTitleCount 9

const char* rtcTitles[rtcTitleCount]= {
	"AXPJ",
	"AXVJ",
	"BPEJ",
	"AXPE",
	"AXVE",
	"BPEE",
	"AXPP",
	"AXVP",
	"BPEP"
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overread"
bool isRtcGame() {
	const char *gameCode=(const char*)&ROM_DATA[0x00AC];
	for(int i=0;i<rtcTitleCount;++i) {
		if(strncmp(gameCode, rtcTitles[i], 4)==0) {
			return true;
		}
	}
	return false;
}

void initHW() {
	switch(ROM_DATA[0x00AC]) {
		case 'K': {
			REG_WAITCNT = 0x0B17;
			startTiltScan();
		} break;
		
		case 'R': {
			initGPIO();
			REG_WAITCNT = 0x045B7;
		} break;
		
		case 'P': {
			REG_WAITCNT = 0x05803;
		} break;
		
		case 'U': {
			initRTC();
		} break;
		
		default: {
			REG_WAITCNT = 0x0317;
			bool isRtc=isRtcGame();
			if(isRtc) {
				initRTC();
			}
		} break;
	}
}
#pragma GCC diagnostic pop