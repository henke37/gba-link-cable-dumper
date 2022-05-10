#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

bool hasSpecialHardware(const char *gameId);

bool fileExists(const char *fileName);
bool dirExists(const char *path);

#endif