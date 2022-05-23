#ifndef GBA_UPLOAD_H
#define GBA_UPLOAD_H

class GbaConnection;

void waitGbaBios(GbaConnection &con);
void gbaUploadMultiboot(GbaConnection &con, const uint8_t *executable, size_t executableSize);

#endif