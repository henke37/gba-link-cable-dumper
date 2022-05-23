#ifndef GBA_JOYBUS_H
#define GBA_JOYBUS_H
void initGbaJoyport();

class GbaConnection;

extern GbaConnection gbaCon[4];

class GbaConnection {
	u8 *resbuf,*cmdbuf;
	u8 gbaStatus;
	
	s32 getChan() const {
		return this-gbaCon;
	};
	
	void sendRaw(const u8 *buff);
	
	u32 recvRawNoWait();
	u32 recvRawUntilSet();

	void waitGbaReadSentData();
	void waitGbaSetDataToRecv();
	void pollStatus();
	
public:
	GbaConnection();
	
	void resetGba();
	u32 recv();
	u32 recvRaw();
	
	u32 recvBuff(u8 *buff, int len);
	void sendBuff(const u8 *buff, int len);
	
	void send(u32);
	
	bool isGbaConnected() const;
	
	friend void waitGbaBios(GbaConnection &con);
};



#endif