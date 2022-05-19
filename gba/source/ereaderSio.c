#include "ereaderInternal.h"

#define SIO_DATA 0x01
#define SIO_CLOCK 0x02
#define SIO_DIR 0x04

void ereaderSIOBegin() {
	EREAD_CTRL0 |= SIO_DATA | SIO_DIR;
	EREAD_CTRL0 |= SIO_CLOCK;
	//delay
	EREAD_CTRL0 &= ~SIO_DATA;
	//delay
	EREAD_CTRL0 &= ~SIO_CLOCK;
}

void ereaderSIOEnd() {
	EREAD_CTRL0 |= SIO_DIR;
	EREAD_CTRL0 &= ~SIO_DATA;
	//delay
	EREAD_CTRL0 |= SIO_CLOCK;
	//delay
	EREAD_CTRL0 |= SIO_DATA;
}

bool ereaderSIOReadBit() {
	bool bit;
	EREAD_CTRL0 &= ~SIO_DIR;
	//delay
	EREAD_CTRL0 |= SIO_CLOCK;
	//delay
	bit=EREAD_CTRL0 & SIO_DATA;
	EREAD_CTRL0 &= ~SIO_CLOCK;
	
	return bit;
}

void ereaderSIOWriteBit(bool bit) {
	EREAD_CTRL0 = (EREAD_CTRL0 & ~SIO_DATA) | (bit?SIO_DATA:0);
	//delay
	EREAD_CTRL0 |= SIO_CLOCK;
	//delay
	EREAD_CTRL0 &= ~SIO_CLOCK;
}

u8 ereaderSIOReadByte(bool endFlag) {
	u8 data=0;
	
	for(int i=7;i>=0;--i) {
		data |= ereaderSIOReadBit() << i;
	}
	ereaderSIOWriteBit(endFlag);
	
	return data;
}

bool ereaderSIOWriteByte(u8 data) {
	bool err;
	for(int i=7;i>=0;--i) {
		ereaderSIOWriteBit(data & (1<<i));
	}
	err=ereaderSIOReadBit();
	
	EREAD_CTRL0 |= SIO_DIR;
	
	return err;
}
