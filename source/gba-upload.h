#ifndef GBA_UPLOAD_H
#define GBA_UPLOAD_H

void waitGbaBios(s32 chan);
void gbaUploadMultiboot(s32 chan, const uint8_t *executable, size_t executableSize);

#endif