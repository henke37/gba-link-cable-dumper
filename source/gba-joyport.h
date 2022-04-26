#ifndef GBA_JOYPORT_H
#define GBA_JOYPORT_H
void initGbaJoyport();

void doreset();
void getstatus();
u32 recv();
void send(u32 msg);

int isGbaConnected();

#endif