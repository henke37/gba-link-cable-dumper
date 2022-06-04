#include "transman.h"

#include <assert.h>

u8 *sendBuff;
size_t sendLen, sendProgress;
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
}
	
void transManSetRecv(const u8 *dst, size_t len) {
	assert(dst!=NULL);
	assert(recvBuff==NULL);
	assert(len>0);
}

bool transManSendCB() {
	if(sendBuff==NULL) return false;
	
	return true;
}
bool transManRecvCB() {
	if(recvBuff==NULL) return false;
	
	return true;
}