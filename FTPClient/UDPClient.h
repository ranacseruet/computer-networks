#include "Common.h"

#include <cstdio>
#include <windows.h>
#include <sys/stat.h>

class UDPClient
{
private:
	int port;
	int handle;

	sockaddr_in address, myaddress;
	char hostName[50];
public:
	UDPClient();
	~UDPClient();
	void SetHostName(char[]);
	void CreateConnection();
	bool SendRequest(Request);
	bool SendDatat(Data data);

	void run(void);

	Data RecieveData();
	Response RecieveResponse();
};