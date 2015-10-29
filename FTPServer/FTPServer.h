#include "UDPServer.h"

public class FTPServer
{
private:
	UDPServer* udpServer;
	FileHelper* fileHelper;
public:
	FTPServer(UDPServer*, FileHelper*);
	~FTPServer(void);

	void run(void);

	void get(void);
	void put(void);
	void del(void);
	void list(Request);
	void handshake(void);
};