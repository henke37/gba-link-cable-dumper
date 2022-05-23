/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include <malloc.h>
#include <gccore.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>

#include "gba-joybus.h"
#include "utils.h"

//from my tests 50us seems to be the lowest
//safe si transfer delay in between calls
#define SI_TRANS_DELAY 50

#define JOYSTAT_SEND 8
#define JOYSTAT_RECV 2

GbaConnection gbaCon[4];


volatile u32 transval = 0;
void transcb(s32 chan, u32 ret) {
	transval = 1;
}

volatile u32 resval = 0;

GbaConnection::GbaConnection() {
	cmdbuf = (u8*) memalign(32,32);
	resbuf = (u8*) memalign(32,32);
}

bool GbaConnection::isGbaConnected() const {
	u32 type=SI_Probe(getChan());
	return type==(SI_GBA);
}

void GbaConnection::resetGba() {
	cmdbuf[0] = 0xFF; //reset
	transval = 0;
	SI_Transfer(getChan(),cmdbuf,1,resbuf,3,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	
	gbaStatus=resbuf[2];
}
void GbaConnection::pollStatus() {
	cmdbuf[0] = 0; //status
	transval = 0;
	SI_Transfer(getChan(),cmdbuf,1,resbuf,3,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	
	gbaStatus=resbuf[2];
}
u32 GbaConnection::recvRaw() {
	waitGbaSetDataToRecv();
	return recvRawNoWait();
}

u32 GbaConnection::recvRawNoWait() {
	u32 recvData;
	memset(resbuf,0,32);
	cmdbuf[0]=0x14; //read
	transval = 0;
	SI_Transfer(getChan(),cmdbuf,1,resbuf,5,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	
	recvData=*(vu32*)resbuf;
	gbaStatus=resbuf[4];
	
	return recvData;
}
u32 GbaConnection::recv() {
	return __builtin_bswap32(recvRaw());
}

void GbaConnection::send(u32 msg) {
	cmdbuf[0]=0x15;
	cmdbuf[1]=(msg>>0)&0xFF;
	cmdbuf[2]=(msg>>8)&0xFF;
	cmdbuf[3]=(msg>>16)&0xFF;
	cmdbuf[4]=(msg>>24)&0xFF;
	transval = 0;
	resbuf[0] = 0;
	SI_Transfer(getChan(),cmdbuf,5,resbuf,1,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	gbaStatus=resbuf[0];
	
	waitGbaReadSentData();
}

void GbaConnection::sendRaw(const u8 *buff) {
	cmdbuf[0]=0x15;
	cmdbuf[1]=buff[0];
	cmdbuf[2]=buff[1];
	cmdbuf[3]=buff[2];
	cmdbuf[4]=buff[3];
	transval = 0;
	resbuf[0] = 0;
	SI_Transfer(getChan(),cmdbuf,5,resbuf,1,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	gbaStatus=resbuf[0];
	
	waitGbaReadSentData();
}

u32 GbaConnection::recvRawUntilSet() {
	u32 val;
	int wc=0;
	while(1) {
		val=recvRawNoWait();
		if((gbaStatus & JOYSTAT_SEND)==JOYSTAT_SEND) break;
		wc++;
		printf("\x1b[s\x1b[1;60HWR: %x\x1b[u",gbaStatus);
	}
	printf("\x1b[s\x1b[1;60HWR: OK\x1b[2;60H%d\x1b[u",wc);
	
	return val;
}

u32 GbaConnection::recvBuff(u8 *buff, int len) {
	int j;
	u32 bytes_read = 0;
	
	assert((((uintptr_t)buff)%4)==0);
	assert((len % 4)==0);
	
	waitGbaSetDataToRecv();
	
	u32 prevVal=0;
	
	for(j = 0; j < len; ) {
		u32 val=recvRawUntilSet();
		
		if(gbaStatus & 0x0010) {
			val=__builtin_bswap32(val);
			for(u32 k=0;k<val;++k) {
				if(j>=len) {
					fatalError("Decompression overrun!");
				}
				
				bytes_read+=4;
				*(vu32*)(buff+j) = prevVal;
				j+=4;
			}
		} else {
			if(j>=len) {
				fatalError("Decompression overrun!");
			}
			*(vu32*)(buff+j) = val;
			bytes_read+=4;
			prevVal=val;
			j+=4;
		}
	}
	
	return bytes_read;
}

void GbaConnection::sendBuff(const u8 *buff, int len) {
	int i;
	
	assert((((uintptr_t)buff)%4)==0);
	assert((len % 4)==0);
	
	for(i = 0; i < len; i+=4)
		send(__builtin_bswap32(*(vu32*)(buff+i)));
}


void GbaConnection::waitGbaReadSentData() {
	int wc=0;
	do {
		printf("\x1b[s\x1b[1;60HWS: %x\x1b[u",gbaStatus);
		pollStatus();
		wc++;
	} while((gbaStatus & JOYSTAT_RECV)==JOYSTAT_RECV);
	printf("\x1b[s\x1b[1;60HWS: OK\x1b[2;60H%d\x1b[u",wc);
}
void GbaConnection::waitGbaSetDataToRecv() {
	int wc=0;
	do {
		pollStatus();
		wc++;
	} while((gbaStatus & JOYSTAT_SEND)==0);
	printf("\x1b[s\x1b[1;60HWR: OK\x1b[2;60H%d\x1b[u",wc);
}