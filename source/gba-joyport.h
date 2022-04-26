#ifndef GBA_JOYPORT_H
#define GBA_JOYPORT_H
void initGbaJoyport();

void doreset();
void getstatus();
u32 recv();
void send(u32 msg);

int isGbaConnected();

u32 recvToBuff(u8 *buff, int len);

#endif