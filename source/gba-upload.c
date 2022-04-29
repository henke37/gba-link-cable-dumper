/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include <gccore.h>

#include "gba-upload.h"
#include "gba-joyport.h"

extern u8 *resbuf;

unsigned int docrc(u32 crc, u32 val);
unsigned int calckey(unsigned int size);

void waitGbaBios(s32 chan) {
	resbuf[2]=0;
	while(!(resbuf[2]&0x10))
	{
		resetGba(chan);
		getGbaStatus(chan);
	}
}

void gbaUploadMultiboot(s32 chan, const uint8_t *executable, size_t executableSize) {
	unsigned int sendsize = ((executableSize+7)&~7);
	unsigned int ourkey = calckey(sendsize);
	int i;
	
	//printf("Our Key: %08x\n", ourkey);
	//get current sessionkey
	u32 sessionkeyraw = recvFromGbaRaw(chan);
	u32 sessionkey = __builtin_bswap32(sessionkeyraw^0x7365646F);
	//send over our own key
	sendToGba(chan, __builtin_bswap32(ourkey));
	
	unsigned int fcrc = 0x15a0;	
	
	//send over gba header
	sendBuffToGba(chan, executable, 0xC0);
	
	//printf("Header done! Sending ROM...\n");
	for(i = 0xC0; i < sendsize; i+=4)
	{
		u32 enc = ((executable[i+3]<<24)|(executable[i+2]<<16)|(executable[i+1]<<8)|(executable[i]));
		fcrc=docrc(fcrc,enc);
		sessionkey = (sessionkey*0x6177614B)+1;
		enc^=sessionkey;
		enc^=((~(i+(0x20<<20)))+1);
		enc^=0x20796220;
		sendToGba(chan, enc);
	}
	fcrc |= (sendsize<<16);
	//printf("ROM done! CRC: %08x\n", fcrc);
	
	//send over CRC
	sessionkey = (sessionkey*0x6177614B)+1;
	fcrc^=sessionkey;
	fcrc^=((~(i+(0x20<<20)))+1);
	fcrc^=0x20796220;
	sendToGba(chan, fcrc);
	//get crc back (unused)
	recvFromGbaRaw(chan);
}

unsigned int calckey(unsigned int size)
{
	unsigned int ret = 0;
	size=(size-0x200) >> 3;
	int res1 = (size&0x3F80) << 1;
	res1 |= (size&0x4000) << 2;
	res1 |= (size&0x7F);
	res1 |= 0x380000;
	int res2 = res1;
	res1 = res2 >> 0x10;
	int res3 = res2 >> 8;
	res3 += res1;
	res3 += res2;
	res3 <<= 24;
	res3 |= res2;
	res3 |= 0x80808080;

	if((res3&0x200) == 0)
	{
		ret |= (((res3)&0xFF)^0x4B)<<24;
		ret |= (((res3>>8)&0xFF)^0x61)<<16;
		ret |= (((res3>>16)&0xFF)^0x77)<<8;
		ret |= (((res3>>24)&0xFF)^0x61);
	}
	else
	{
		ret |= (((res3)&0xFF)^0x73)<<24;
		ret |= (((res3>>8)&0xFF)^0x65)<<16;
		ret |= (((res3>>16)&0xFF)^0x64)<<8;
		ret |= (((res3>>24)&0xFF)^0x6F);
	}
	return ret;
}

unsigned int docrc(u32 crc, u32 val)
{
	int i;
	for(i = 0; i < 0x20; i++)
	{
		if((crc^val)&1)
		{
			crc>>=1;
			crc^=0xa1c1;
		}
		else
			crc>>=1;
		val>>=1;
	}
	return crc;
}
