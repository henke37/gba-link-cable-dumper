#ifndef JOYBUS_H
#define JOYBUS_H

void enableJoyBusIRQ(bool enabled);

bool isJoyBusAnyPending();
bool isJoyBusRecvPending();
bool isJoyBusSendPending();
bool isJoyBusResetPending();

void sendJoyBusNoJStat(u32 data);
void sendJoyBus(u32 data);
u32 recvJoyBus();

void ackJoySend();
void ackJoyRecv();

void sendJoyBusBuff(const u8 *data, int len);
void recvJoyBusBuff(u8 *data, int len);

#endif