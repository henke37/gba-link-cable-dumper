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
#include "gba-joyport.h"
#include "utils.h"

//from my tests 50us seems to be the lowest
//safe si transfer delay in between calls
#define SI_TRANS_DELAY 50

#define JOYSTAT_SEND 8
#define JOYSTAT_RECV 2

u8 *resbuf,*cmdbuf;
vu8 gbaStatus[4];

u32 recvFromGbaRawNoWait(s32 chan);

void waitGbaReadSentData(s32 chan);
void waitGbaSetDataToRecv(s32 chan);

volatile u32 transval = 0;
void transcb(s32 chan, u32 ret)
{
	transval = 1;
}

volatile u32 resval = 0;
void acb(s32 res, u32 val)
{
	resval = val;
}

void initGbaJoyport() {
	cmdbuf = memalign(32,32);
	resbuf = memalign(32,32);
}

int isGbaConnected(s32 chan) {
	u32 type=SI_Probe(chan);
	return type==(SI_GBA);
}
void resetGba(s32 chan)
{
	cmdbuf[0] = 0xFF; //reset
	transval = 0;
	SI_Transfer(chan,cmdbuf,1,resbuf,3,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	
	gbaStatus[chan]=resbuf[2];
}
void getGbaStatus(s32 chan)
{
	cmdbuf[0] = 0; //status
	transval = 0;
	SI_Transfer(chan,cmdbuf,1,resbuf,3,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	
	gbaStatus[chan]=resbuf[2];
}
u32 recvFromGbaRaw(s32 chan) {
	waitGbaSetDataToRecv(chan);
	return recvFromGbaRawNoWait(chan);
}

u32 recvFromGbaRawNoWait(s32 chan) {
	u32 recvData;
	memset(resbuf,0,32);
	cmdbuf[0]=0x14; //read
	transval = 0;
	SI_Transfer(chan,cmdbuf,1,resbuf,5,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	
	recvData=*(vu32*)resbuf;
	gbaStatus[chan]=resbuf[4];
	
	return recvData;
}
u32 recvFromGba(s32 chan) {
	return __builtin_bswap32(recvFromGbaRaw(chan));
}

void sendToGba(s32 chan, u32 msg) {
	cmdbuf[0]=0x15;
	cmdbuf[1]=(msg>>0)&0xFF;
	cmdbuf[2]=(msg>>8)&0xFF;
	cmdbuf[3]=(msg>>16)&0xFF;
	cmdbuf[4]=(msg>>24)&0xFF;
	transval = 0;
	resbuf[0] = 0;
	SI_Transfer(chan,cmdbuf,5,resbuf,1,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	gbaStatus[chan]=resbuf[0];
	
	waitGbaReadSentData(chan);
}

void sendToGbaRaw(s32 chan, const u8 *buff) {
	cmdbuf[0]=0x15;
	cmdbuf[1]=buff[0];
	cmdbuf[2]=buff[1];
	cmdbuf[3]=buff[2];
	cmdbuf[4]=buff[3];
	transval = 0;
	resbuf[0] = 0;
	SI_Transfer(chan,cmdbuf,5,resbuf,1,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	gbaStatus[chan]=resbuf[0];
	
	waitGbaReadSentData(chan);
}

u32 recvBuffFromGba(s32 chan, u8 *buff, int len) {
	int j;
	u32 bytes_read = 0;
	
	waitGbaSetDataToRecv(chan);
	
	u32 prevVal=0;
	
	for(j = 0; j < len; ) {
		u32 val;
		int wc=0;
		while(1) {
			val=recvFromGbaRawNoWait(chan);
			if((gbaStatus[chan] & JOYSTAT_SEND)==JOYSTAT_SEND) break;
			wc++;
			printf("\x1b[s\x1b[1;60HWR: %x\x1b[u",gbaStatus[chan]);
		}
		printf("\x1b[s\x1b[1;60HWR: OK\x1b[2;60H%d\x1b[u",wc);
		
		if(gbaStatus[chan] & 0x0010) {
			val=__builtin_bswap32(val);
			for(int k=0;k<val;++k) {
				if(j>=len) {
					fatalError("Decompression overrun!");
				}
				
				bytes_read+=4;
				*(vu32*)(buff+j) = prevVal;
				j+=4;
			}
		} else {
			*(vu32*)(buff+j) = val;
			bytes_read+=4;
			prevVal=val;
			j+=4;
		}
	}
	
	return bytes_read;
}

void sendBuffToGba(s32 chan, const u8 *buff, int len) {
	int i;
	for(i = 0; i < len; i+=4)
		sendToGba(chan,__builtin_bswap32(*(vu32*)(buff+i)));
}


void waitGbaReadSentData(s32 chan) {
	int wc=0;
	do {
		printf("\x1b[s\x1b[1;60HWS: %x\x1b[u",gbaStatus[chan]);
		getGbaStatus(chan);
		wc++;
	} while((gbaStatus[chan] & JOYSTAT_RECV)==JOYSTAT_RECV);
	printf("\x1b[s\x1b[1;60HWS: OK\x1b[2;60H%d\x1b[u",wc);
}
void waitGbaSetDataToRecv(s32 chan) {
	int wc=0;
	do {
		getGbaStatus(chan);
		wc++;
	} while((gbaStatus[chan] & JOYSTAT_SEND)==0);
	printf("\x1b[s\x1b[1;60HWR: OK\x1b[2;60H%d\x1b[u",wc);
}

u8 getExtaGbaStatus(s32 chan) {
	return gbaStatus[chan] & 0x30;
}