#ifndef GBA_JOYPORT_H
#define GBA_JOYPORT_H
void initGbaJoyport();

void doreset(s32 chan);
void getstatus(s32 chan);
u32 recv(s32 chan);
void send(s32 chan, u32 msg);

int isGbaConnected(s32 chan);

u32 recvToBuff(s32 chan, u8 *buff, int len);
void sendBuff(s32 chan, const u8 *buff, int len);

#endif