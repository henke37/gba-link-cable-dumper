/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
#include <gba.h>
#include "joybus.h"


#define JOY_WRITE 2
#define JOY_READ 4
#define JOY_RW 6


void configureJoyBus() {

}
 
void clearJoyBus();

void waitJoyBusReadAck() {
	while((REG_HS_CTRL&JOY_WRITE) == 0) ;
}

void waitJoyBusWriteAck() {
	while((REG_HS_CTRL&JOY_READ) == 0) ;
}

void sendJoyBus(u32 data) {
	REG_JOYTR=data;
}

u32 recvJoyBus() {
	return REG_JOYRE;
}

void sendJoyBusBuff(const u8 *data, int len) {
	int i;
	for(i = 0; i < len; i+=4) {
		sendJoyBus( *(vu32*)(data+i) );
		waitJoyBusWriteAck();
		REG_HS_CTRL |= JOY_RW;
	}
}

void recvJoyBusBuff(u8 *data, int len) {
	int i;
	for(i = 0; i < len; i+=4) {
		waitJoyBusReadAck();
		REG_HS_CTRL |= JOY_RW;
		*(vu32*)(data+i) = recvJoyBus();
	}
}

int isJoyBusRecvPending() {
	return REG_HS_CTRL&JOY_READ;
}

int isJoyBusSendPending() {
	return REG_HS_CTRL&JOY_WRITE;
}