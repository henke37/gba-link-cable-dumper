#include "transman.h"
#include "joybus.h"

#include <assert.h>
#include <stdio.h>

const u8 *sendBuff;
size_t sendLen, sendProgress;
u32 sendPrevVal;
transCompleteCb sendCompleteCb;

u8 *recvBuff;
size_t recvSize, recvProgress;
transCompleteCb recvCompleteCb;

void transManInit() {
	sendBuff=NULL;
	recvBuff=NULL;
}

void transManSetSend(const u8 *src, size_t len, transCompleteCb cb) {
	assert(src!=NULL);
	assert(sendBuff==NULL);
	assert(len>0);
	
	assert((((uintptr_t)src)%4)==0);
	assert((len % 4)==0);
	
	sendLen=len;
	sendProgress=0;
	sendBuff=src;
	sendPrevVal=0;
	sendCompleteCb=cb;
	
	transManSendCB();
}
	
void transManSetRecv(u8 *dst, size_t len, transCompleteCb cb) {
	assert(dst!=NULL);
	assert(recvBuff==NULL);
	assert(len>0);
	
	assert((((uintptr_t)dst)%4)==0);
	assert((len % 4)==0);
	
	recvSize=len;
	recvProgress=0;
	recvBuff=dst;
	recvCompleteCb=cb;
	
	transManRecvCB();
}

bool transManSendCB() {
	if(sendBuff==NULL) return false;
	
	iprintf("TMSCB: %d\n",sendProgress);
	
	u32 val=*(vu32*)(sendBuff+sendProgress);
	if(val==sendPrevVal) {
		int k=1;
		int kMax=(sendLen-sendProgress)/4;
		for(;k<kMax;++k) {
			val=((vu32*)(sendBuff+sendProgress))[k];
			if(val!=sendPrevVal) break;
		}
		//k+=1;
		REG_JSTAT |= 0x0010;
		sendJoyBusNoJStat(k);
		sendProgress+=4*k;
	} else {
		sendJoyBus(val);
		sendPrevVal=val;
		sendProgress+=4;
	}
	
	assert(sendProgress<=sendLen);
	if(sendProgress==sendLen) {
		sendCompleteCb();
		sendBuff=NULL;
	}
	
	return true;
}
bool transManRecvCB() {
	if(recvBuff==NULL) return false;
	
	*(vu32*)(recvBuff+recvProgress) = recvJoyBus();
	
	if(recvProgress==recvSize) {
		recvCompleteCb();
		recvBuff=NULL;
	}
	
	return true;
}

void transManSendCompleteDefaultCb() {
	sendJoyBus(sendLen);
}
void transManRecvCompleteDefaultCb() {
	sendJoyBus(recvSize);
}