#ifndef JOYBUS_H
#define JOYBUS_H

void enableJoyBusIRQ(bool enabled);

bool isJoyBusRecvPending();
bool isJoyBusSendPending();
bool isJoyBusResetPending();

void sendJoyBus(u32 data);
u32 recvJoyBus();

void sendJoyBusBuff(const u8 *data, int len);
void recvJoyBusBuff(u8 *data, int len);

#endif