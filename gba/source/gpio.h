#ifndef GPIO_H
#define GPIO_H

#include <stdbool.h>
#include <gba.h>

void initGPIO();

void rtcRead();
void rtcWrite();

void solarRead();
void setRumble(bool active);

u16 readGyro();

#endif