/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
#include <gba.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "joybus.h"

#define JOYCNT_RESET 1
#define JOYCNT_SEND 4
#define JOYCNT_RECV 2

#define JOYSTAT_RECV 2
#define JOYSTAT_SEND 8

void enableJoyBusIRQ(bool enabled) {
	REG_HS_CTRL = enabled?0x40:0;
}

void waitJoyBusSendCmd() {
	while((REG_HS_CTRL&JOYCNT_SEND) == 0) ;
}

void waitJoyBusRecvCmd() {
	while((REG_HS_CTRL&JOYCNT_RECV) == 0) ;
}

void sendJoyBusNoJStat(u32 data) {
	if(REG_HS_CTRL&JOYCNT_SEND) {
		iprintf("Send missed deadline!\n");
	}
	//iprintf("S:%lx", data);
	REG_JOYTR=data;
	waitJoyBusSendCmd();
	REG_HS_CTRL |= JOYCNT_SEND;
}

void sendJoyBus(u32 data) {
	REG_JSTAT &= ~0x0010;
	sendJoyBusNoJStat(data);
}

u32 recvJoyBus() {
	waitJoyBusRecvCmd();
	
	u32 val = REG_JOYRE;
	REG_HS_CTRL |= JOYCNT_RECV;
	//iprintf("R:%lx", val);
	return val;
}

void sendJoyBusBuff(const u8 *data, int len) {
	int i;
	u32 prevVal=0;
	
	assert((((uintptr_t)data)%4)==0);
	assert((len % 4)==0);
	
	for(i = 0; i < len;)  {
		u32 val=*(vu32*)(data+i);
		if(val==prevVal) {
			int k=1;
			int kMax=(len-i)/4;
			for(;k<kMax;++k) {
				val=((vu32*)(data+i))[k];
				if(val!=prevVal) break;
			}
			//k+=1;
			REG_JSTAT |= 0x0010;
			sendJoyBusNoJStat(k);
			i+=4*k;
		} else {
			sendJoyBus(val);
			prevVal=val;
			i+=4;
		}
	}
}

void recvJoyBusBuff(u8 *data, int len) {
	int i;
	
	assert((((uintptr_t)data)%4)==0);
	assert((len % 4)==0);
	
	for(i = 0; i < len; i+=4) {
		*(vu32*)(data+i) = recvJoyBus();
	}
}

bool isJoyBusRecvPending() {
	return REG_HS_CTRL&JOYCNT_RECV;
}

bool isJoyBusSendPending() {
	return REG_HS_CTRL&JOYCNT_SEND;
}

bool isJoyBusResetPending() {
	return REG_HS_CTRL&JOYCNT_RESET;
}

bool isJoyBusAnyPending() {
	return REG_HS_CTRL&(JOYCNT_SEND|JOYCNT_RECV|JOYCNT_RESET);
}