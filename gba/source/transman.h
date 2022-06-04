#ifndef TRANSMAN_H
#define TRANSMAN_H

#include <stdbool.h>
#include <stddef.h>
#include <gba.h>

void transManInit();
void transManSetSend(const u8 *src, size_t len);
void transManSetRecv(const u8 *dst, size_t len);
bool transManSendCB();
bool transManRecvCB();

#endif