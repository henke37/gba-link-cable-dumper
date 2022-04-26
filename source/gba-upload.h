#ifndef GBA_UPLOAD_H
#define GBA_UPLOAD_H

void waitGbaBios();
void gbaUploadMultiboot(const uint8_t *executable, size_t executableSize);

#endif