#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stdnoreturn.h>

bool hasSpecialHardware();

bool fileExists(const char *fileName);
bool dirExists(const char *path);

void warnError(const char *msg);
void noreturn fatalError(const char *msg);
void noreturn endproc();

#endif