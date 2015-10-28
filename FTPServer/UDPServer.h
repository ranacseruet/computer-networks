#include "../FTPClient/Common.h"

class UDPServer
{
private:
	int port, serverSocket;
	char serverName[HOSTNAME_LENGTH];
	struct sockaddr_in server, client;
	SOCKET s;
	WSADATA wsa;
public:
	UDPServer(int);
	~UDPServer(void);

	void run(void);

	bool SendDatat(Data data);
	bool SendResponse(Response data);

	void RecieveRequest(Request*);
	Data RecieveData();
};