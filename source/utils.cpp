#include "utils.h"

#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"

bool dirExists(const char *path) {
	DIR *dir;
	dir = opendir(path);
	if(dir)
	{
		closedir(dir);
		return true;
	}
	return false;
}

bool fileExists(const char *fileName) {
	return access(fileName, F_OK)==0;
}

bool hasSpecialHardware() {
	switch(romHeader.gameId[0]) {
		case 'A':
		case 'B':
		case 'C':
		case 'F':
			return false;
		case 'K':
		case 'R':
		case 'U':
		case 'V':
			return true;
		case 'P':
			return strncmp(romHeader.gameId, "PASS", 4) !=0;
		default:
			return true;
	}
}

void warnError(const char *msg) {
	puts(msg);
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	sleep(2);
}
void fatalError(const char *msg) {
	puts(msg);
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	sleep(5);
	exit(0);
}
void endproc() {
	printf("Start pressed, exit\n");
	VIDEO_WaitVSync();
	VIDEO_WaitVSync();
	exit(0);
}