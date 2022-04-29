/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
#include <gba.h>
#include "joybus.h"

#define JOY_RESET 1
#define JOY_SEND 2
#define JOY_RECV 4


void configureJoyBus(bool useInterrupts) {
	REG_HS_CTRL = (REG_HS_CTRL & ~0x40) | (useInterrupts?0x40:0);
}
 
void clearJoyBus() {
	REG_HS_CTRL |= 0x7;
}

void waitJoyBusSendCmd() {
	while((REG_HS_CTRL&JOY_SEND) == 0) ;
}

void waitJoyBusRecvCmd() {
	while((REG_HS_CTRL&JOY_RECV) == 0) ;
}

void sendJoyBus(u32 data) {
	REG_JOYTR=data;
	waitJoyBusSendCmd();
	REG_HS_CTRL |= JOY_SEND;
}

u32 recvJoyBus() {
	waitJoyBusRecvCmd();
	REG_HS_CTRL |= JOY_RECV;
	return REG_JOYRE;
}

void sendJoyBusBuff(const u8 *data, int len) {
	int i;
	for(i = 0; i < len; i+=4) {
		sendJoyBus( *(vu32*)(data+i) );
	}
}

void recvJoyBusBuff(u8 *data, int len) {
	int i;
	for(i = 0; i < len; i+=4) {
		*(vu32*)(data+i) = recvJoyBus();
	}
}

bool isJoyBusRecvPending() {
	return REG_HS_CTRL&JOY_RECV;
}

bool isJoyBusSendPending() {
	return REG_HS_CTRL&JOY_SEND;
}