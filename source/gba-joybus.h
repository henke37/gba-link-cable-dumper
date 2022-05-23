#ifndef GBA_JOYBUS_H
#define GBA_JOYBUS_H
void initGbaJoyport();

void resetGba(s32 chan);
void getGbaStatus(s32 chan);
u32 recvFromGba(s32 chan);
u32 recvFromGbaRaw(s32 chan);
void sendToGba(s32 chan, u32 msg);
void sendToGbaRaw(s32 chan, const u8 *buff);

int isGbaConnected(s32 chan);

u32 recvBuffFromGba(s32 chan, u8 *buff, int len);
void sendBuffToGba(s32 chan, const u8 *buff, int len);

u8 getExtaGbaStatus(s32 chan);

#endif