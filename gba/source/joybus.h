#ifndef JOYBUS_H
#define JOYBUS_H

void configureJoyBus();

void clearJoyBus();

void ackJoyBusRead();
void ackJoyBusWrite();

int isJoyBusReadPending();
int isJoyBusWritePending();

void waitJoyBusReadAck();
void waitJoyBusWriteAck();

void sendJoyBus(u32 data);
u32 recvJoyBus();

#endif