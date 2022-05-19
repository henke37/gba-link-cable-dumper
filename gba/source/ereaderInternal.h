#ifndef EREADER_INTERNAL_H
#define EREADER_INTERNAL_H

#include "ereader.h"

#include <gba.h>

#define EREAD_RESET *((vu16*)0xDFA0000)
#define EREAD_CTRL0 *((vu8* )0xE00FFB0)
#define EREAD_CTRL1 *((vu8* )0xE00FFB1)
#define EREAD_LED   *((vu16*)0xE00FFB2)

#define EREAD_CALDATA   (( u8*)0xE00D000)
#define EREAD_SCANDATA  ((vu8*)0xDFC0000)
#define EREAD_INTENSITY ((vu8*)0xE00FF80)

void ereaderSIOBegin();
void ereaderSIOEnd();
bool ereaderSIOReadBit();
void ereaderSIOWriteBit(bool);
u8   ereaderSIOReadByte(bool endFlag);
bool ereaderSIOWriteByte(u8);

#endif