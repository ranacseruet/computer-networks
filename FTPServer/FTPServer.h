#include "UDPServer.h"

class FTPServer
{
private:
	UDPServer serverInstance;
public:
	FTPServer(void);
	~FTPServer(void);

	void run(void);

	void get(void);
	void put(void);
	void del(void);
	void list(void);
	void handshake(void);
};