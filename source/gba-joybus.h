#ifndef GBA_JOYBUS_H
#define GBA_JOYBUS_H

class GbaConnection {
	u8 *resbuf,*cmdbuf;
	u8 gbaStatus;
	volatile u32 transval = 0;
	
	s32 getChan() const {
		return this-cons;
	};
	
	void sendRaw(const u8 *buff);
	
	u32 recvRawNoWait();
	u32 recvRawUntilSet();

	void waitGbaReadSentData();
	void waitGbaSetDataToRecv();
	void pollStatus();
	
	static void transcb(s32 chan, u32 ret);
	
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
	
	static GbaConnection cons[4];
};

#define gbaCon GbaConnection::cons

#endif