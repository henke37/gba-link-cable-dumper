/*
 * Copyright (C) 2016 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
#include <gba.h>
#include "joybus.h"


#define JOY_WRITE 2
#define JOY_READ 4
#define JOY_RW 6


void configureJoyBus() {

}
 
void clearJoyBus();

void waitJoyBusReadAck() {
	while((REG_HS_CTRL&JOY_WRITE) == 0) ;
}

void waitJoyBusWriteAck() {
	while((REG_HS_CTRL&JOY_READ) == 0) ;
}