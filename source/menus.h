#ifndef MENUS_H
#define MENUS_H

#include <stdnoreturn.h>

void printBanner();
void waitGbaConnect();
void preDumpMenu();
void handleGbaCart();

void warnError(const char *msg);
void noreturn fatalError(const char *msg);
void noreturn endproc();

#endif