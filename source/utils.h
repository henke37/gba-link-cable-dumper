#ifndef UTILS_H
#define UTILS_H

bool hasSpecialHardware();

bool fileExists(const char *fileName);
bool dirExists(const char *path);

void warnError(const char *msg);
[[noreturn]] void fatalError(const char *msg);
[[noreturn]] void endproc();

#endif