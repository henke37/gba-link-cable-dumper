#ifndef MAIN_H
#define MAIN_H


#define	REG_WAITCNT *(vu16 *)(REG_BASE + 0x204)

#define ROM_DATA ((const u8 *)0x08000000)
#define ROM_HEADER_LEN 0xC0

#define biosSize 0x4000

s32 getGameSize(void);

void initHW();

u8 purloinBiosData(int offset);

extern u8 save_data[0x20000];

void readSave(u8 *data, u32 savesize);
void writeSave(u8 *data, u32 savesize);

#endif