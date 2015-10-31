#include "UDP.h"

#include <cstdio>
#include <windows.h>
#include <sys/stat.h>

class UDPClient:public UDP
{
public:
	UDPClient();
	~UDPClient();
	void CreateConnection();
	void CloseConnection();

	void SetHostName(char[]);
	
	bool SendRequest(Request);
	Response RecieveResponse();
};