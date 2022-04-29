#ifndef JOYBUS_H
#define JOYBUS_H

void configureJoyBus(bool useInterrupts);

void clearJoyBus();

void ackJoyBusRead();
void ackJoyBusWrite();

bool isJoyBusRecvPending();
bool isJoyBusSendPending();

void sendJoyBus(u32 data);
u32 recvJoyBus();

void sendJoyBusBuff(const u8 *data, int len);
void recvJoyBusBuff(u8 *data, int len);

#endif