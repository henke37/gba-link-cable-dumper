#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

bool hasSpecialHardware();

bool fileExists(const char *fileName);
bool dirExists(const char *path);

#endif