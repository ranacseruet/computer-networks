#include "Udp.h"
#include <windows.h>
#include <winsock.h>

UDPServer::UDPServer(int sock)
{
	socket = sock;
}

UDPServer::~UDPServer()
{
	//WSACleanup();
}

Request* UDPServer::RecieveRequest()
{
	Request* req;

	return req;
}

int main(void)
{
	return 0;
}