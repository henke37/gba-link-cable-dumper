#ifndef JOYBUS_H
#define JOYBUS_H

void configureJoyBus(bool useInterrupts);

void clearJoyBus();

void ackJoyBusRead();
void ackJoyBusWrite();

bool isJoyBusRecvPending(u8);
bool isJoyBusSendPending(u8);
bool isJoyBusResetPending(u8);

void sendJoyBus(u32 data);
u32 recvJoyBus();
u32 recvJoyBusNoWait();

void sendJoyBusBuff(const u8 *data, int len);
void recvJoyBusBuff(u8 *data, int len);

#endif