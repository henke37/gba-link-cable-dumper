#include "transman.h"
#include "joybus.h"

#include <assert.h>

const u8 *sendBuff;
size_t sendLen, sendProgress;
u32 sendPrevVal;
u8 *recvBuff;
size_t recvSize, recvProgress;

void transManInit() {
	sendBuff=NULL;
	recvBuff=NULL;
}

void transManSetSend(const u8 *src, size_t len) {
	assert(src!=NULL);
	assert(sendBuff==NULL);
	assert(len>0);
	
	assert((((uintptr_t)src)%4)==0);
	assert((len % 4)==0);
	
	sendLen=len;
	sendProgress=0;
	sendBuff=src;
	sendPrevVal=0;
}
	
void transManSetRecv(u8 *dst, size_t len) {
	assert(dst!=NULL);
	assert(recvBuff==NULL);
	assert(len>0);
	
	assert((((uintptr_t)dst)%4)==0);
	assert((len % 4)==0);
	
	recvSize=len;
	recvProgress=0;
	recvBuff=dst;
}

bool transManSendCB() {
	if(sendBuff==NULL) return false;
	
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
		sendBuff=NULL;
	}
	
	return true;
}
bool transManRecvCB() {
	if(recvBuff==NULL) return false;
	
	*(vu32*)(recvBuff+recvProgress) = recvJoyBus();
	
	if(recvProgress==recvSize) {
		recvBuff=NULL;
	}
	
	return true;
}