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
	
public:
	UDPClient();
	~UDPClient();
	void CreateConnection();
	bool SendRequest(Request);
	bool SendDatat(Data data);

	void run(void);

	Data RecieveData();
	Response RecieveResponse();
};