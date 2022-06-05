#ifndef TRANSMAN_H
#define TRANSMAN_H

#include <stdbool.h>
#include <stddef.h>
#include <gba.h>

typedef void (*transCompleteCb)(void);

void transManInit();
void transManSetSend(const u8 *src, size_t len, transCompleteCb cb);
void transManSetRecv(u8 *dst, size_t len, transCompleteCb cb);
bool transManSendCB();
bool transManRecvCB();

void transManSendCompleteDefaultCb();
void transManRecvCompleteDefaultCb();

#endif