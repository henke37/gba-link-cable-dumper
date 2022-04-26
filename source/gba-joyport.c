/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include <malloc.h>
#include <gccore.h>
#include <string.h>
#include "gba-joyport.h"

//from my tests 50us seems to be the lowest
//safe si transfer delay in between calls
#define SI_TRANS_DELAY 50

u8 *resbuf,*cmdbuf;

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

int isGbaConnected() {
	SI_GetTypeAsync(1,acb);
	if(resval) {
		if(resval == 0x80 || resval & 8) {
			return 0;
		}
		else if(resval)
			return 1;
	}
	return 0;
}
void doreset()
{
	cmdbuf[0] = 0xFF; //reset
	transval = 0;
	SI_Transfer(1,cmdbuf,1,resbuf,3,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
}
void getstatus()
{
	cmdbuf[0] = 0; //status
	transval = 0;
	SI_Transfer(1,cmdbuf,1,resbuf,3,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
}
u32 recv()
{
	memset(resbuf,0,32);
	cmdbuf[0]=0x14; //read
	transval = 0;
	SI_Transfer(1,cmdbuf,1,resbuf,5,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
	return *(vu32*)resbuf;
}
void send(u32 msg)
{
	cmdbuf[0]=0x15;cmdbuf[1]=(msg>>0)&0xFF;cmdbuf[2]=(msg>>8)&0xFF;
	cmdbuf[3]=(msg>>16)&0xFF;cmdbuf[4]=(msg>>24)&0xFF;
	transval = 0;
	resbuf[0] = 0;
	SI_Transfer(1,cmdbuf,5,resbuf,1,transcb,SI_TRANS_DELAY);
	while(transval == 0) ;
}