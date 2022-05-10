#ifndef MENUS_H
#define MENUS_H

void printBanner();
void waitGbaConnect();
void preDumpMenu();
void handleGbaCart();

void warnError(char *msg);
void fatalError(char *msg);
void endproc();

#endif