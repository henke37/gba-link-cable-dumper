#ifndef JOYBUS_H
#define JOYBUS_H

void configureJoyBus();

void clearJoyBus();

void ackJoyBusRead();
void ackJoyBusWrite();

int isJoyBusRecvPending();
int isJoyBusSendPending();

void waitJoyBusSendAck();
void waitJoyBusRecvReady();

void sendJoyBus(u32 data);
u32 recvJoyBus();

void sendJoyBusBuff(const u8 *data, int len);
void recvJoyBusBuff(u8 *data, int len);

#endif