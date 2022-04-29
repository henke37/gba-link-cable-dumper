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
#include "libSave.h"
#include "joybus.h"

#include "../../source/packets.h"

#define	REG_WAITCNT *(vu16 *)(REG_BASE + 0x204)

u8 save_data[0x20000] __attribute__ ((section (".sbss")));

#define ROM_DATA ((const u8 *)0x08000000)
#define ROM_HEADER_LEN 0xC0

#define biosSize 0x4000

s32 getGameSize(void);

void readSave(u8 *data, u32 savesize);
void writeSave(u8 *data, u32 savesize);

void sendBiosDump();

void handlePacket(u32 type);

u8 purloinBiosData(int offset);

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

	consoleDemoInit();
	
	// ansi escape sequence to set print co-ordinates
	// /x1b[line;columnH	
	iprintf("\x1b[9;2HGBA Link Cable Dumper v1.6\n");
	iprintf("\x1b[10;4HPlease look at the TV\n");
	
	// disable this, needs power
	SNDSTAT = 0;
	SNDBIAS = 0;
	
	// Set up waitstates for EEPROM access etc. 
	REG_WAITCNT = 0x0317;
	
	while (1) {
		Halt();
		u32 type=recvJoyBus();
		handlePacket(type);
	}
}

void handlePacket(u32 type) {
	switch(type) {
		case PING: {
			iprintf("Got a ping!\n");
			sendJoyBus(recvJoyBus());
		} break;
		case READ_PAD: {
			sendJoyBus(REG_KEYINPUT);
		} break;
		case CHECK_GAME: {
			iprintf("Checking game...\n");
			s32 gamesize = getGameSize();
			u32 savesize = SaveSize(save_data,gamesize);
			sendJoyBus(gamesize);
			sendJoyBus(savesize);
		} break;
		
		case READ_HEADER: {
			//game in, send header
			iprintf("Reading rom header.\n");
			sendJoyBusBuff(ROM_DATA, ROM_HEADER_LEN);
		} break;
			
		case READ_ROM:	{
			iprintf("Dumping rom (take a walk).\n");
			s32 gamesize = getGameSize();
			//disable interrupts
			u32 prevIrqMask = REG_IME;
			REG_IME = 0;
			//dump the game
			sendJoyBusBuff(ROM_DATA, gamesize);
			
			sendJoyBus(gamesize);
			//restore interrupts
			REG_IME = prevIrqMask;
		} break;
		
		case READ_SAVE: {
			iprintf("Reading save.\n");
			s32 gamesize = getGameSize();
			u32 savesize = SaveSize(save_data,gamesize);
			
			readSave(save_data,savesize);
			sendJoyBusBuff(save_data, savesize);
			sendJoyBus(savesize);
		} break;
		
		case WRITE_SAVE: {
			iprintf("Writing save.\n");
			s32 gamesize = getGameSize();
			u32 savesize = SaveSize(save_data,gamesize);
			
			recvJoyBusBuff(save_data, savesize);
			writeSave(save_data, savesize);
			sendJoyBus(savesize);
		} break;
		
		case CLEAR_SAVE: {
			iprintf("Erasing save. Hope you made a save first!\n");
			s32 gamesize = getGameSize();
			u32 savesize = SaveSize(save_data,gamesize);
			
			//clear the save
			memset(save_data, 0, savesize);
				
			writeSave(save_data, savesize);
			
			sendJoyBus(savesize);
		} break;
		
		case READ_BIOS:{
			iprintf("Purloining bios\n");
			sendBiosDump();
		} break;
		
		default:
			iprintf("Got unknown packet %#010lx!",type);
		break;
	}
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

u8 purloinBiosData(int offset) {
	WaveData *fakeAddr=(WaveData *)( offset-(((offset & 3)+1)| 3) );
	u8 b = (MidiKey2Freq(fakeAddr, 168, 0) * 2) >> 24;
	return b;
}
