#include "utils.h"

#include <string.h>
#include <dirent.h>
#include <unistd.h>

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

bool hasSpecialHardware(const char *gameId) {
	switch(*gameId) {
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
			return strncmp(gameId, "PASS", 4) !=0;
		default:
			return true;
	}
}