#include "../FTPClient/Common.h"

public class UDPServer
{
private:
	char serverName[HOSTNAME_LENGTH];
	struct sockaddr_in server, client;
	SOCKET s;
	WSADATA wsa;
	Logger *logger;
public:
	UDPServer(Logger*);
	~UDPServer(void);

	void run(void);

	bool SendDatat(Data data);
	bool SendResponse(Response data);

	bool RecieveRequest(Request*);
	Data RecieveData();
};