#ifndef JOYBUS_H
#define JOYBUS_H

void configureJoyBus();

void clearJoyBus();

void ackJoyBusRead();
void ackJoyBusWrite();

int isJoyBusRecvPending();
int isJoyBusSendPending();

void waitJoyBusReadAck();
void waitJoyBusWriteAck();

void sendJoyBus(u32 data);
u32 recvJoyBus();

#endif