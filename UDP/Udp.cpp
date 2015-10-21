#include "Udp.h"
#include <windows.h>

UDP::UDP(int sock)
{
	socket = sock;
}

UDP::~UDP()
{
	//WSACleanup();
}

bool UDP::SendRequest(Request req)
{
	//TODO
	return true;
}

Request UDP::RecieveRequest()
{
	//TODO
	Request req;
	return req;
}

int main(void)
{

	return 0;
}